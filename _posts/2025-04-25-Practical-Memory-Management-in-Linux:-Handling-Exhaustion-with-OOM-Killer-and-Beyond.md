---
title: "Practical Memory Management in Linux: Handling Exhaustion with OOM Killer and Beyond"
date: 2025-04-25 00:00:00 +0530
categories: [Operating Systems, Linux, Memory Management]
tags: [memory-management, linux-kernel, swapping, cgroups, os]
author: mohitmishra786
description: "When a Linux system runs out of memory, it triggers a complex series of events designed to maintain system stability while preserving critical functionality. This technical introduction explores the inner workings of Linux memory management during exhaustion scenarios, with particular focus on the Out-of-Memory (OOM) killer and related subsystems."
toc: true
---

# Table of Contents
- [Core Concepts of Linux Memory Management](#core-concepts-of-linux-memory-management)
  - [Memory Exhaustion Definition and Context](#memory-exhaustion-definition-and-context)
    - [Simple Memory Exhaustion Example](#simple-memory-exhaustion-example)
  - [Memory Allocation Strategies](#memory-allocation-strategies)
    - [Demonstrating Overcommit Policies](#demonstrating-overcommit-policies)
  - [The OOM Killer: Last Line of Defense](#the-oom-killer-last-line-of-defense)
    - [Examining OOM Scores](#examining-oom-scores)
- [Architectural Overview of Memory Management Under Pressure](#architectural-overview-of-memory-management-under-pressure)
  - [Memory Allocation Request Flow](#memory-allocation-request-flow)
    - [Tracking Memory Allocations](#tracking-memory-allocations)
  - [Page Fault Handling During Memory Scarcity](#page-fault-handling-during-memory-scarcity)
    - [Demonstrating Page Fault Handling](#demonstrating-page-fault-handling)
  - [Interactions Between Memory Subsystems](#interactions-between-memory-subsystems)
- [Memory Reclamation and Swapping Mechanisms](#memory-reclamation-and-swapping-mechanisms)
  - [Page Cache Reclamation](#page-cache-reclamation)
    - [Testing Page Cache Behavior](#testing-page-cache-behavior)
  - [Transparent Huge Pages Reclamation](#transparent-huge-pages-reclamation)
    - [Demonstrating THP Impact](#demonstrating-thp-impact)
  - [Anonymous Memory and Swapping](#anonymous-memory-and-swapping)
    - [Examining Swap Behavior](#examining-swap-behavior)
  - [The kswapd Daemon and Direct Reclaim](#the-kswapd-daemon-and-direct-reclaim)
- [OOM Killer Mechanics](#oom-killer-mechanics)
  - [Scoring Algorithm and Process Selection](#scoring-algorithm-and-process-selection)
    - [OOM Killer Simulation](#oom-killer-simulation)
  - [Badness Function Implementation](#badness-function-implementation)
  - [Process Termination Sequence](#process-termination-sequence)
  - [OOM Control via Cgroups](#oom-control-via-cgroups)
    - [Demonstrating Cgroup Memory Limits](#demonstrating-cgroup-memory-limits)
- [Future Directions in Linux Memory Management](#future-directions-in-linux-memory-management)
  - [Advanced Swap Technologies](#advanced-swap-technologies)
  - [Persistent Memory Integration](#persistent-memory-integration)
  - [Machine Learning for Memory Management](#machine-learning-for-memory-management)
  - [Enhanced Memory Pressure Signaling](#enhanced-memory-pressure-signaling)
- [Conclusion](#conclusion)

## Core Concepts of Linux Memory Management

### Memory Exhaustion Definition and Context

Memory exhaustion in Linux occurs when the system can no longer satisfy memory allocation requests from userspace processes or the kernel itself. Unlike some operating systems that strictly prevent allocations beyond physical memory, Linux implements memory overcommitment, allowing processes to allocate more virtual memory than physically available. This approach optimizes resource utilization based on the observation that many processes never use all their allocated memory simultaneously.

The kernel manages memory through a layered architecture:

1. **Physical memory management**: Handles actual RAM through the buddy allocator system
2. **Virtual memory management**: Creates the illusion of contiguous memory spaces for processes
3. **Paging subsystem**: Transfers memory pages between RAM and secondary storage

When memory pressure increases, Linux employs a series of increasingly aggressive strategies to reclaim memory, culminating in the potential invocation of the OOM killer.

#### Simple Memory Exhaustion Example

Here's a simple C program that gradually consumes memory until the system runs out:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    const size_t chunk_size = 10 * 1024 * 1024; // 10 MB chunks
    unsigned long allocated = 0;
    
    printf("Starting memory exhaustion test\n");
    printf("PID: %d\n", getpid());
    
    while(1) {
        void *mem = malloc(chunk_size);
        if (mem == NULL) {
            printf("malloc failed after allocating %lu MB\n", allocated / (1024 * 1024));
            break;
        }
        
        // Actually touch the memory to ensure it's allocated
        for (size_t i = 0; i < chunk_size; i += 4096) {
            ((char*)mem)[i] = 1;
        }
        
        allocated += chunk_size;
        printf("Allocated %lu MB\n", allocated / (1024 * 1024));
        sleep(1);
    }
    
    // This code will likely never execute due to OOM killer
    printf("Exiting normally\n");
    return 0;
}
```

**Expected Output:**
```
Starting memory exhaustion test
PID: 12345
Allocated 10 MB
Allocated 20 MB
...
Allocated 3540 MB
```

The program will continue allocating memory until either `malloc` fails (unlikely due to overcommitment) or, more commonly, the system's OOM killer terminates the process. The kernel log would show something like:

```
[601053.170127] Out of memory: Kill process 12345 (a.out) score 945 or sacrifice child
[601053.170134] Killed process 12345 (a.out) total-vm:3642MB, anon-rss:3541MB, file-rss:0KB, shmem-rss:0KB
```

### Memory Allocation Strategies

Linux implements three principal overcommitment policies, configurable via the `vm.overcommit_memory` sysctl parameter:

1. **Heuristic overcommit** (value 0): The default setting. The kernel uses a heuristic to determine whether to allow or deny memory allocations, considering factors like available swap space and current memory utilization.

2. **Always overcommit** (value 1): The kernel never refuses memory allocation requests, regardless of the system's memory state. This policy maximizes memory utilization but increases the risk of the OOM killer being invoked.

3. **Never overcommit** (value 2): The kernel enforces strict accounting, allowing allocations only up to a limit defined by `vm.overcommit_ratio` or `vm.overcommit_kbytes`. This is the most conservative approach, reducing system performance but increasing predictability.

The degree of overcommitment is further controlled by `vm.overcommit_ratio` (default 50%), which determines the percentage of RAM that can be overcommitted when using the "never overcommit" policy.

#### Demonstrating Overcommit Policies

This C program demonstrates the behavior under different overcommit policies:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    const size_t gigabyte = 1024 * 1024 * 1024;
    char command[256];
    size_t attempt = 1;
    
    printf("PID: %d\n", getpid());
    printf("Attempting to allocate memory in 1GB chunks...\n");
    
    // Get total RAM
    sprintf(command, "free -h | grep 'Mem:' | awk '{print $2}'");
    printf("Total RAM: ");
    fflush(stdout);
    system(command);
    
    // Get overcommit policy
    sprintf(command, "cat /proc/sys/vm/overcommit_memory");
    printf("Current overcommit policy: ");
    fflush(stdout);
    system(command);
    
    while(1) {
        void *mem = malloc(gigabyte);
        if (mem == NULL) {
            printf("malloc failed on attempt %zu (requested 1GB)\n", attempt);
            break;
        }
        
        // Touch the memory to ensure pages are allocated
        memset(mem, 1, gigabyte);
        
        printf("Successfully allocated %zu GB\n", attempt);
        attempt++;
        
        // Free the memory for demonstration purposes
        // In a real memory exhaustion scenario, we would keep this allocated
        free(mem);
    }
    
    return 0;
}
```

**Expected Output with `vm.overcommit_memory=0` (default):**
```
PID: 12346
Attempting to allocate memory in 1GB chunks...
Total RAM: 8.0G
Current overcommit policy: 0
Successfully allocated 1 GB
...
Successfully allocated 12 GB
Successfully allocated 13 GB
malloc failed on attempt 14 (requested 1GB)
```

**Expected Output with `vm.overcommit_memory=1`:**
```
PID: 12347
Attempting to allocate memory in 1GB chunks...
Total RAM: 8.0G
Current overcommit policy: 1
Successfully allocated 1 GB
...
Successfully allocated 49 GB
Successfully allocated 50 GB
[Process likely terminated by OOM killer]
```

**Expected Output with `vm.overcommit_memory=2` and `vm.overcommit_ratio=50`:**
```
PID: 12348
Attempting to allocate memory in 1GB chunks...
Total RAM: 8.0G
Current overcommit policy: 2
Successfully allocated 1 GB
...
Successfully allocated 11 GB
malloc failed on attempt 12 (requested 1GB)
```

### The OOM Killer: Last Line of Defense

The Out-of-Memory killer serves as the kernel's final defense mechanism when memory allocations cannot be satisfied through reclamation or swapping. Its primary function is to identify and terminate one or more processes to free sufficient memory for the system to continue functioning.

The OOM killer follows a sophisticated scoring system to select victim processes. Each process receives an `oom_score` based on factors including:

- Memory consumption (both physical and virtual)
- Runtime duration (favoring shorter-lived processes)
- Process priority
- User-defined adjustments via `oom_score_adj`

The selection criteria favor killing processes that:
- Consume substantial memory
- Have been running for a short time
- Have lower priority
- Are not critical system processes

This approach aims to minimize disruption while maximizing memory recovery, though its effectiveness depends on the specific workload and system configuration.

#### Examining OOM Scores

This program displays the OOM score of a process and allows adjusting it:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    char filename[256];
    char command[256];
    FILE *file;
    int score, adj;
    
    // Print current process ID
    printf("Process ID: %d\n", pid);
    
    // Read current oom_score
    sprintf(filename, "/proc/%d/oom_score", pid);
    file = fopen(filename, "r");
    if (file) {
        fscanf(file, "%d", &score);
        fclose(file);
        printf("Current oom_score: %d\n", score);
    } else {
        perror("Failed to read oom_score");
    }
    
    // Read current oom_score_adj
    sprintf(filename, "/proc/%d/oom_score_adj", pid);
    file = fopen(filename, "r");
    if (file) {
        fscanf(file, "%d", &adj);
        fclose(file);
        printf("Current oom_score_adj: %d\n", adj);
    } else {
        perror("Failed to read oom_score_adj");
    }
    
    // Show scores of some system processes for comparison
    printf("\nOOM scores of some system processes:\n");
    system("ps -eo pid,comm | grep -E 'systemd|sshd|cron' | head -3 | while read pid comm; do echo \"$comm ($pid): $(cat /proc/$pid/oom_score 2>/dev/null || echo 'N/A')\"; done");
    
    // If argument provided, try to adjust our oom_score_adj
    if (argc > 1) {
        int new_adj = atoi(argv[1]);
        printf("\nAttempting to set oom_score_adj to %d\n", new_adj);
        
        sprintf(command, "echo %d > /proc/%d/oom_score_adj", new_adj, pid);
        if (system(command) != 0) {
            printf("Failed to set oom_score_adj (try running with sudo)\n");
        } else {
            // Re-read oom_score
            sprintf(filename, "/proc/%d/oom_score", pid);
            file = fopen(filename, "r");
            if (file) {
                fscanf(file, "%d", &score);
                fclose(file);
                printf("New oom_score: %d\n", score);
            }
        }
    }
    
    printf("\nWaiting for 60 seconds so you can examine/modify this process...\n");
    printf("Try: echo VALUE > /proc/%d/oom_score_adj\n", pid);
    sleep(60);
    
    return 0;
}
```

**Expected Output:**
```
Process ID: 12349
Current oom_score: 42
Current oom_score_adj: 0

OOM scores of some system processes:
systemd (1): 0
sshd (1234): 0
cron (1235): 0

Waiting for 60 seconds so you can examine/modify this process...
Try: echo VALUE > /proc/12349/oom_score_adj
```

## Architectural Overview of Memory Management Under Pressure

### Memory Allocation Request Flow

When a program requests memory through system calls like `malloc()` or `mmap()`, the request follows a complex path through the kernel:

1. **User space allocation**: Libraries like glibc translate application calls into appropriate system calls.

2. **System call handling**: The kernel processes the system call (e.g., `brk()`, `mmap()`) and updates the process's virtual memory area (VMA) structures.

3. **Page table updates**: The kernel modifies the process's page tables but typically does not allocate physical memory immediately (lazy allocation).

4. **Physical allocation**: Physical memory is actually allocated when the process accesses the memory, triggering a page fault.

Under memory pressure, this sequence encounters additional checks and potential delays at each stage.

#### Tracking Memory Allocations

This C program demonstrates and tracks the path of memory allocation:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>

// Function to print process memory stats
void print_memory_stats(const char* label) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    printf("\n--- %s ---\n", label);
    printf("  RSS (reported by getrusage): %ld KB\n", usage.ru_maxrss);
    
    // Use /proc/self/status for more detailed info
    FILE* status = fopen("/proc/self/status", "r");
    if (status) {
        char line[256];
        while (fgets(line, sizeof(line), status)) {
            if (strncmp(line, "VmSize:", 7) == 0 ||
                strncmp(line, "VmRSS:", 6) == 0 ||
                strncmp(line, "VmData:", 7) == 0 ||
                strncmp(line, "VmStk:", 6) == 0) {
                printf("  %s", line);
            }
        }
        fclose(status);
    }
    
    // Print address space maps
    printf("  Memory maps (partial):\n");
    system("head -5 /proc/self/maps");
}

int main() {
    const size_t size = 100 * 1024 * 1024; // 100 MB
    printf("Demonstrating different memory allocation paths\n");
    
    // Initial state
    print_memory_stats("Initial state");
    
    // 1. malloc (uses brk for large allocations)
    void* malloc_mem = malloc(size);
    print_memory_stats("After malloc (no touch)");
    
    // Touch memory to trigger actual allocation
    memset(malloc_mem, 1, size);
    print_memory_stats("After malloc (touched)");
    
    // 2. Anonymous mmap
    void* mmap_mem = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmap_mem == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    print_memory_stats("After mmap (no touch)");
    
    // Touch memory from mmap
    memset(mmap_mem, 2, size);
    print_memory_stats("After mmap (touched)");
    
    // 3. Try with MAP_POPULATE (pre-fault pages)
    void* populate_mem = mmap(NULL, size, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    if (populate_mem == MAP_FAILED) {
        perror("mmap with MAP_POPULATE failed");
        return 1;
    }
    print_memory_stats("After mmap with MAP_POPULATE");
    
    // Cleanup
    free(malloc_mem);
    munmap(mmap_mem, size);
    munmap(populate_mem, size);
    print_memory_stats("After cleanup");
    
    return 0;
}
```

**Expected Output:**
```
Demonstrating different memory allocation paths

--- Initial state ---
  RSS (reported by getrusage): 3512 KB
  VmSize:     4364 kB
  VmRSS:      3512 kB
  VmData:      504 kB
  VmStk:       132 kB
  Memory maps (partial):
555555554000-555555556000 r--p 00000000 08:01 1311053 /path/to/executable
555555556000-55555555a000 r-xp 00002000 08:01 1311053 /path/to/executable
55555555a000-55555555c000 r--p 00006000 08:01 1311053 /path/to/executable
55555555c000-55555555d000 r--p 00008000 08:01 1311053 /path/to/executable
55555555d000-55555555e000 rw-p 00009000 08:01 1311053 /path/to/executable

--- After malloc (no touch) ---
  RSS (reported by getrusage): 3512 KB
  VmSize:   104604 kB
  VmRSS:      3512 kB
  VmData:   100744 kB
  VmStk:       132 kB
  Memory maps (partial):
[Similar to above, but with data segment increased]

--- After malloc (touched) ---
  RSS (reported by getrusage): 102400 KB
  VmSize:   104604 kB
  VmRSS:    102400 kB
  VmData:   100744 kB
  VmStk:       132 kB
  Memory maps (partial):
[Similar to above]

--- After mmap (no touch) ---
  RSS (reported by getrusage): 102400 KB
  VmSize:   204604 kB
  VmRSS:    102400 kB
  VmData:   100744 kB
  VmStk:       132 kB
  Memory maps (partial):
[Now includes the anonymous mapping]

--- After mmap (touched) ---
  RSS (reported by getrusage): 204800 KB
  VmSize:   204604 kB
  VmRSS:    204800 kB
  VmData:   100744 kB
  VmStk:       132 kB
  Memory maps (partial):
[Similar to above]

--- After mmap with MAP_POPULATE ---
  RSS (reported by getrusage): 307200 KB
  VmSize:   304604 kB
  VmRSS:    307200 kB
  VmData:   100744 kB
  VmStk:       132 kB
  Memory maps (partial):
[Now includes the pre-populated mapping]

--- After cleanup ---
  RSS (reported by getrusage): 3512 KB
  VmSize:     4364 kB
  VmRSS:      3512 kB
  VmData:      504 kB
  VmStk:       132 kB
  Memory maps (partial):
[Returns to initial state]
```

### Page Fault Handling During Memory Scarcity

Page faults occur when a process accesses memory that hasn't been mapped to physical RAM. The kernel handles these faults through a sequence of steps:

1. **Fault detection**: The CPU generates an exception when accessing an unmapped address.

2. **Fault classification**: The kernel determines if the fault is legitimate (e.g., first access to a valid allocation) or invalid (e.g., segmentation fault).

3. **Memory allocation**: For legitimate faults, the kernel attempts to allocate a physical page.

4. **Page table update**: The kernel maps the physical page to the virtual address.

During memory pressure, the allocation stage may trigger memory reclamation, potentially forcing the kernel to swap out existing pages or invoke the OOM killer if reclamation fails.

#### Demonstrating Page Fault Handling

This program demonstrates the relationship between page faults and memory access:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>

void print_fault_stats() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    printf("Major page faults: %ld, Minor page faults: %ld\n", 
           usage.ru_majflt, usage.ru_minflt);
}

int main() {
    const size_t page_size = 4096;
    const size_t alloc_size = 100 * 1024 * 1024; // 100 MB
    const size_t pages = alloc_size / page_size;
    
    printf("Demonstrating page fault behavior\n");
    printf("Initial stats:\n");
    print_fault_stats();
    
    // Allocate memory
    printf("\nAllocating %zu MB using mmap...\n", alloc_size / (1024 * 1024));
    char* memory = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, 
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    
    printf("After allocation (no access):\n");
    print_fault_stats();
    
    // Access first page
    printf("\nAccessing first page...\n");
    memory[0] = 1;
    printf("After accessing first page:\n");
    print_fault_stats();
    
    // Access pages with increments to show fault behavior
    printf("\nAccessing every 1000th page...\n");
    for (size_t i = 0; i < pages; i += 1000) {
        memory[i * page_size] = 1;
    }
    printf("After accessing sparse pages:\n");
    print_fault_stats();
    
    // Access all pages
    printf("\nAccessing every page (may take a moment)...\n");
    for (size_t i = 0; i < pages; i++) {
        memory[i * page_size] = 1;
    }
    printf("After accessing all pages:\n");
    print_fault_stats();
    
    // Try to force a major fault by reclaiming memory
    printf("\nAttempting to force major page faults...\n");
    printf("Running: echo 3 > /proc/sys/vm/drop_caches\n");
    printf("Note: This may require sudo privileges\n");
    system("sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches' 2>/dev/null || echo 'Failed to drop caches (no sudo)'");
    
    // Access memory again after cache drop
    printf("\nAccessing memory after cache drop...\n");
    for (size_t i = 0; i < pages; i += 100) {
        memory[i * page_size] = 2;
    }
    printf("Final stats:\n");
    print_fault_stats();
    
    // Clean up
    munmap(memory, alloc_size);
    return 0;
}
```

**Expected Output:**
```
Demonstrating page fault behavior
Initial stats:
Major page faults: 0, Minor page faults: 281

Allocating 100 MB using mmap...
After allocation (no access):
Major page faults: 0, Minor page faults: 284

Accessing first page...
After accessing first page:
Major page faults: 0, Minor page faults: 285

Accessing every 1000th page...
After accessing sparse pages:
Major page faults: 0, Minor page faults: 311

Accessing every page (may take a moment)...
After accessing all pages:
Major page faults: 0, Minor page faults: 25786

Attempting to force major page faults...
Running: echo 3 > /proc/sys/vm/drop_caches
Note: This may require sudo privileges
Failed to drop caches (no sudo)

Accessing memory after cache drop...
Final stats:
Major page faults: 0, Minor page faults: 25786
```

If successful with sudo permissions, you would see major page faults increase after the cache drop.

### Interactions Between Memory Subsystems

Linux memory management relies on the cooperation of several subsystems:

1. **Buddy allocator**: Manages physical memory pages in power-of-two blocks.

2. **Slab allocator**: Provides efficient allocation for kernel data structures of standard sizes.

3. **Page cache**: Caches file data in memory to accelerate I/O operations.

4. **Swap subsystem**: Transfers memory pages between RAM and swap devices.

5. **Memory policy enforcement**: Applies constraints from cgroups, mempolicy, and other mechanisms.

During memory exhaustion, these components communicate via a complex signaling system that balances immediate needs against long-term performance considerations.

## Memory Reclamation and Swapping Mechanisms

### Page Cache Reclamation

The page cache is a primary target for memory reclamation since its contents can be regenerated from backing storage if needed. Linux maintains two LRU (Least Recently Used) lists:

1. **Active list**: Contains recently accessed pages
2. **Inactive list**: Contains pages that haven't been accessed recently

The kernel periodically moves pages between these lists based on access patterns, reclaiming pages from the inactive list when memory pressure increases. This process is primarily managed by the kernel thread `kswapd`.

The `vm.vfs_cache_pressure` parameter (default 100) controls the kernel's eagerness to reclaim page cache pages versus anonymous pages. Higher values increase page cache reclamation pressure.

#### Testing Page Cache Behavior

This C program demonstrates page cache usage and reclamation:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TEST_FILE "pagecache_test.bin"
#define FILE_SIZE (100 * 1024 * 1024) // 100 MB
#define BUFFER_SIZE (4 * 1024)        // 4 KB

void print_memory_info() {
    printf("Page cache status:\n");
    system("grep -E 'Cached|Buffers' /proc/meminfo");
    printf("\n");
}

// Function to measure read time
double measure_read_time(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return -1.0;
    }
    
    char buffer[BUFFER_SIZE];
    size_t total_read = 0;
    clock_t start = clock();
    
    while (total_read < FILE_SIZE) {
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) break;
        total_read += bytes_read;
    }
    
    clock_t end = clock();
    close(fd);
    
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main() {
    printf("Demonstrating page cache behavior\n");
    print_memory_info();
    
    // Create test file
    printf("Creating %d MB test file...\n", FILE_SIZE / (1024 * 1024));
    int fd = open(TEST_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Failed to create test file");
        return 1;
    }
    
    char buffer[BUFFER_SIZE];
    memset(buffer, 'A', BUFFER_SIZE);
    
    for (size_t written = 0; written < FILE_SIZE; written += BUFFER_SIZE) {
        if (write(fd, buffer, BUFFER_SIZE) != BUFFER_SIZE) {
            perror("write failed");
            close(fd);
            return 1;
        }
    }
    close(fd);
    
    // Clear caches to ensure first read isn't cached
    printf("Dropping caches (may require sudo)...\n");
    system("sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches' 2>/dev/null || echo 'Failed (no sudo)'");
    print_memory_info();
    
    // First read (cold cache)
    printf("First read (cold cache)...\n");
    double time1 = measure_read_time(TEST_FILE);
    printf("Read time: %.3f seconds\n", time1);
    print_memory_info();
    
    // Second read (warm cache)
    printf("Second read (warm cache)...\n");
    double time2 = measure_read_time(TEST_FILE);
    printf("Read time: %.3f seconds\n", time2);
    printf("Speedup factor: %.2fx\n", time1 / time2);
    print_memory_info();
    
    // Try to force page cache reclamation under memory pressure
    printf("\nSimulating memory pressure...\n");
    printf("Allocating large memory block...\n");
    
    // Allocate large chunk to create memory pressure
    void* mem = malloc(FILE_SIZE * 2); // 2x file size
    if (mem != NULL) {
        // Touch pages to ensure allocation
        for (size_t i = 0; i < FILE_SIZE * 2; i += 4096) {
            ((char*)mem)[i] = 1;
        }
        printf("Memory allocated and touched\n");
        print_memory_info();
        
        // Read file again under memory pressure
        printf("Reading file under memory pressure...\n");
        double time3 = measure_read_time(TEST_FILE);
        printf("Read time under pressure: %.3f seconds\n", time3);
        printf("Pressure vs. warm cache ratio: %.2fx\n", time3 / time2);
        
        free(mem);
    } else {
        printf("Failed to allocate memory for pressure test\n");
    }
    
    // Clean up
    unlink(TEST_FILE);
    printf("Test file removed\n");
    
    return 0;
}
```

**Expected Output:**
```
Demonstrating page cache behavior
Page cache status:
Cached:           2345678 kB
Buffers:           123456 kB

Creating 100 MB test file...
Dropping caches (may require sudo)...
Failed (no sudo)
Page cache status:
Cached:           2345678 kB
Buffers:           123456 kB

First read (cold cache)...
Read time: 0.245 seconds
Page cache status:
Cached:           2445678 kB
Buffers:           123456 kB

Second read (warm cache)...
Read time: 0.012 seconds
Speedup factor: 20.42x
Page cache status:
Cached:           2445678 kB
Buffers:           123456 kB

Simulating memory pressure...
Allocating large memory block...
Memory allocated and touched
Page cache status:
Cached:           2345678 kB
Buffers:           123456 kB

Reading file under memory pressure...
Read time under pressure: 0.198 seconds
Pressure vs. warm cache ratio: 16.50x
Test file removed
```

The actual values will vary significantly based on system configuration, load, and hardware.

### Transparent Huge Pages Reclamation

Transparent Huge Pages (THP) complicate memory reclamation due to their size (typically 2MB versus 4KB for standard pages). When memory pressure occurs, the kernel may need to break huge pages into standard pages before reclamation, introducing additional overhead. This is managed by the `khugepaged` daemon.

#### Demonstrating THP Impact

This program shows the impact of Transparent Huge Pages:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#define GB (1024 * 1024 * 1024UL)
#define MB (1024 * 1024UL)

// Function to check if THP is enabled
void check_thp_status() {
    printf("THP Configuration:\n");
    
    // Read current THP state
    int fd = open("/sys/kernel/mm/transparent_hugepage/enabled", O_RDONLY);
    if (fd >= 0) {
        char buffer[256];
        ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("  enabled: %s", buffer);
        }
        close(fd);
    } else {
        printf("  enabled: [unable to read]\n");
    }
    
    // Read defrag state
    fd = open("/sys/kernel/mm/transparent_hugepage/defrag", O_RDONLY);
    if (fd >= 0) {
        char buffer[256];
        ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("  defrag: %s", buffer);
        }
        close(fd);
    }
    
    // Check khugepaged status
    printf("khugepaged stats:\n");
    system("grep -A2 huge /proc/meminfo");
}

// Function to measure access time for a memory region
double measure_access_time(void* ptr, size_t size, size_t stride) {
    // Ensure the memory is actually allocated by touching it first
    for (size_t i = 0; i < size; i += stride) {
        ((char*)ptr)[i] = 1;
    }
    
    // Measure access time
    clock_t start = clock();
    for (size_t i = 0; i < size; i += stride) {
        ((char*)ptr)[i] += 1;
    }
    clock_t end = clock();
    
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main() {
    printf("Demonstrating Transparent Huge Page (THP) behavior\n\n");
    check_thp_status();
    
    // Allocate memory with and without THP hints
    printf("\nAllocating 1GB with and without THP hints...\n");
    
    // Regular allocation
    printf("Regular mmap allocation:\n");
    void* regular_mem = mmap(NULL, 1 * GB, PROT_READ | PROT_WRITE, 
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (regular_mem == MAP_FAILED) {
        perror("Regular mmap failed");
        return 1;
    }
    
    // THP-friendly allocation with MADV_HUGEPAGE
    printf("THP-friendly allocation (MADV_HUGEPAGE):\n");
    void* thp_mem = mmap(NULL, 1 * GB, PROT_READ | PROT_WRITE, 
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (thp_mem == MAP_FAILED) {
        perror("THP mmap failed");
        munmap(regular_mem, 1 * GB);
        return 1;
    }
    
    // Advise the kernel to use huge pages for this region
    if (madvise(thp_mem, 1 * GB, MADV_HUGEPAGE) != 0) {
        perror("madvise(MADV_HUGEPAGE) failed");
    }
    
    // Touch both memory regions to ensure allocation
    printf("Touching memory to trigger allocation...\n");
    double regular_time = measure_access_time(regular_mem, 1 * GB, 4096);
    double thp_time = measure_access_time(thp_mem, 1 * GB, 4096);
    
    printf("Regular memory initial access time: %.3f seconds\n", regular_time);
    printf("THP memory initial access time: %.3f seconds\n", thp_time);
    
    // Check memory stats after allocation
    printf("\nMemory status after allocation:\n");
    system("grep -A2 huge /proc/meminfo");
    
    // Create memory pressure to force THP splitting
    printf("\nCreating memory pressure to potentially split huge pages...\n");
    void* pressure_mem = NULL;
    for (size_t size = 512 * MB; size >= 64 * MB; size /= 2) {
        pressure_mem = malloc(size);
        if (pressure_mem) {
            printf("Allocated %zu MB for pressure test\n", size / MB);
            // Touch memory to ensure allocation
            memset(pressure_mem, 1, size);
            break;
        }
    }
    
    if (pressure_mem) {
        // Re-measure access times after pressure
        double regular_time2 = measure_access_time(regular_mem, 1 * GB, 4096);
        double thp_time2 = measure_access_time(thp_mem, 1 * GB, 4096);
        
        printf("\nAfter memory pressure:\n");
        printf("Regular memory access time: %.3f seconds (%.2f%% change)\n", 
               regular_time2, (regular_time2 / regular_time - 1) * 100);
        printf("THP memory access time: %.3f seconds (%.2f%% change)\n", 
               thp_time2, (thp_time2 / thp_time - 1) * 100);
        
        free(pressure_mem);
    } else {
        printf("Failed to allocate memory for pressure test\n");
    }
    
    // Check memory status after pressure
    printf("\nMemory status after pressure:\n");
    system("grep -A2 huge /proc/meminfo");
    
    // Clean up
    munmap(regular_mem, 1 * GB);
    munmap(thp_mem, 1 * GB);
    
    return 0;
}
```

**Expected Output:**
```
Demonstrating Transparent Huge Page (THP) behavior

THP Configuration:
  enabled: [always] madvise never
  defrag: always defer defer+madvise madvise never
khugepaged stats:
AnonHugePages:     57344 kB
ShmemHugePages:        0 kB
FileHugePages:         0 kB

Allocating 1GB with and without THP hints...
Regular mmap allocation:
THP-friendly allocation (MADV_HUGEPAGE):
Touching memory to trigger allocation...
Regular memory initial access time: 0.247 seconds
THP memory initial access time: 0.198 seconds

Memory status after allocation:
AnonHugePages:    983040 kB
ShmemHugePages:        0 kB
FileHugePages:         0 kB

Creating memory pressure to potentially split huge pages...
Allocated 512 MB for pressure test

After memory pressure:
Regular memory access time: 0.312 seconds (26.32% change)
THP memory access time: 0.205 seconds (3.54% change)

Memory status after pressure:
AnonHugePages:    589824 kB
ShmemHugePages:        0 kB
FileHugePages:         0 kB
```

The output shows that under memory pressure, THP-enabled memory tends to maintain better performance as the kernel tries to preserve huge pages when possible.

### Anonymous Memory and Swapping

Anonymous memory (memory not backed by files) requires special handling during reclamation since it must be written to swap space before being reclaimed. This process follows several steps:

1. **Swap candidate selection**: The kernel identifies anonymous pages that haven't been accessed recently.

2. **Swap space allocation**: The kernel allocates space on a swap device or file.

3. **Page writeout**: The kernel writes the page contents to swap space.

4. **Page table update**: The kernel marks the page as swapped out.

The efficiency of this process depends heavily on the configuration of the swap subsystem and the performance of the underlying storage devices.

#### Examining Swap Behavior

This program demonstrates swap activity under memory pressure:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MB (1024 * 1024)

// Function to print memory and swap usage
void print_memory_stats() {
    printf("Memory usage:\n");
    system("free -m");
    printf("\nSwap details:\n");
    system("cat /proc/swaps");
    printf("\n");
}

// Function to allocate and access memory incrementally
void test_memory_pressure(size_t max_mb, size_t step_mb, int interval_sec) {
    void** blocks = calloc(max_mb / step_mb, sizeof(void*));
    if (!blocks) {
        perror("Failed to allocate blocks array");
        return;
    }
    
    printf("Starting memory pressure test (%zu MB, %zu MB steps, %d sec intervals)\n",
           max_mb, step_mb, interval_sec);
    print_memory_stats();
    
    size_t allocated = 0;
    unsigned int block_index = 0;
    
    while (allocated < max_mb) {
        // Allocate next block
        size_t block_size = step_mb * MB;
        void* block = malloc(block_size);
        
        if (!block) {
            printf("Failed to allocate block at %zu MB\n", allocated);
            break;
        }
        
        // Touch memory to ensure it's actually allocated
        memset(block, 1, block_size);
        
        // Update tracking
        blocks[block_index++] = block;
        allocated += step_mb;
        
        printf("\nAllocated and accessed %zu MB total\n", allocated);
        print_memory_stats();
        
        // Pause to allow swapping
        sleep(interval_sec);
    }
    
    // Release memory in reverse order
    printf("\nReleasing memory...\n");
    while (block_index > 0) {
        free(blocks[--block_index]);
        allocated -= step_mb;
        
        if (block_index % 5 == 0) {
            printf("Released down to %zu MB\n", allocated);
            print_memory_stats();
            sleep(interval_sec);
        }
    }
    
    free(blocks);
    printf("\nTest completed\n");
    print_memory_stats();
}

int main() {
    // Adjust these parameters based on your system
    size_t system_ram_mb = 0;
    
    // Try to detect system RAM
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char line[256];
        while (fgets(line, sizeof(line), meminfo)) {
            unsigned long ram_kb;
            if (sscanf(line, "MemTotal: %lu kB", &ram_kb) == 1) {
                system_ram_mb = ram_kb / 1024;
                break;
            }
        }
        fclose(meminfo);
    }
    
    if (system_ram_mb == 0) {
        printf("Could not detect system RAM, assuming 8 GB\n");
        system_ram_mb = 8 * 1024;
    }
    
    printf("Detected system RAM: %zu MB\n", system_ram_mb);
    printf("Swap configuration:\n");
    system("swapon --show");
    
    // Calculate test parameters
    size_t max_mb = system_ram_mb / 2;  // Test with half of RAM
    size_t step_mb = max_mb / 20;       // 20 steps
    if (step_mb < 50) step_mb = 50;     // Minimum 50 MB steps
    int interval_sec = 2;               // 2 second intervals
    
    printf("\nSwappiness setting: ");
    system("cat /proc/sys/vm/swappiness");
    
    // Run the test
    test_memory_pressure(max_mb, step_mb, interval_sec);
    
    return 0;
}
```

**Expected Output:**
```
Detected system RAM: 8192 MB
Swap configuration:
NAME      TYPE      SIZE   USED PRIO
/dev/sda2 partition 4G     0B   -2

Swappiness setting: 60

Starting memory pressure test (4096 MB, 204 MB steps, 2 sec intervals)
Memory usage:
              total        used        free      shared  buff/cache   available
Mem:           7872        1824        4750         156        1297        5712
Swap:          4095           0        4095

Allocated and accessed 204 MB total
Memory usage:
              total        used        free      shared  buff/cache   available
Mem:           7872        2028        4546         156        1297        5508
Swap:          4095           0        4095

[intermediate steps omitted]

Allocated and accessed 3876 MB total
Memory usage:
              total        used        free      shared  buff/cache   available
Mem:           7872        5700         874         156        1297        1836
Swap:          4095          72        4023

Allocated and accessed 4080 MB total
Memory usage:
              total        used        free      shared  buff/cache   available
Mem:           7872        5904         670         156        1297        1632
Swap:          4095         226        3869

Releasing memory...
Released down to 3060 MB
Memory usage:
              total        used        free      shared  buff/cache   available
Mem:           7872        4884        1690         156        1297        2652
Swap:          4095         196        3899

[intermediate steps omitted]

Released down to 0 MB
Memory usage:
              total        used        free      shared  buff/cache   available
Mem:           7872        1824        4750         156        1297        5712
Swap:          4095           0        4095

Test completed
Memory usage:
              total        used        free      shared  buff/cache   available
Mem:           7872        1824        4750         156        1297        5712
Swap:          4095           0        4095
```

This output demonstrates how Linux starts using swap space as memory pressure increases, and how it reclaims pages from swap when memory is freed.

### The kswapd Daemon and Direct Reclaim

Linux employs two primary mechanisms for memory reclamation:

1. **kswapd-based reclamation**: A background daemon that periodically scans memory when free memory falls below a threshold (`vm.min_free_kbytes`). It works asynchronously to maintain adequate free memory without disrupting foreground tasks.

2. **Direct reclaim**: Triggered when a memory allocation fails and the allocating process must wait until sufficient memory is reclaimed. This synchronous approach introduces latency but ensures forward progress.

The balance between these approaches is controlled by several parameters, including `vm.swappiness` (default 60), which influences the kernel's preference for swapping anonymous pages versus reclaiming page cache pages.

## OOM Killer Mechanics

### Scoring Algorithm and Process Selection

The OOM killer's scoring algorithm evaluates all user processes using a formula that considers:

1. **Memory usage**: Both resident set size (RSS) and virtual memory size
2. **Runtime**: Processes running for shorter periods receive higher scores
3. **Nice value**: Processes with higher nice values (lower priority) receive higher scores
4. **OOM adjustment**: User-defined adjustments through `oom_score_adj`

The formula approximately follows:

```
oom_score = (memory_usage_in_pages * 10) / (uptime_in_seconds * sqrt(sqrt(uptime_in_seconds)))
```

This is then modified by `oom_score_adj`, which ranges from -1000 (never kill) to 1000 (kill first).

When invoked, the OOM killer calculates scores for all eligible processes and terminates the one with the highest score.

#### OOM Killer Simulation

This program simulates OOM killer behavior by implementing a simplified scoring algorithm:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

// Structure to hold process information
typedef struct {
    int pid;
    char name[256];
    unsigned long rss;        // Resident Set Size in KB
    unsigned long vm_size;    // Virtual Memory Size in KB
    long long start_time;     // Process start time in seconds since boot
    int oom_score;            // Current OOM score
    int oom_score_adj;        // OOM score adjustment
    double calculated_score;  // Our calculated score
} process_info_t;

// Function to get system uptime in seconds
double get_system_uptime() {
    FILE* uptime_file = fopen("/proc/uptime", "r");
    if (!uptime_file) {
        perror("Failed to open /proc/uptime");
        return 0;
    }
    
    double uptime;
    if (fscanf(uptime_file, "%lf", &uptime) != 1) {
        uptime = 0;
    }
    
    fclose(uptime_file);
    return uptime;
}

// Function to read process info from /proc
int get_process_info(process_info_t* proc) {
    char path[256];
    FILE* file;
    char line[256];
    
    // Read process name from /proc/[pid]/comm
    sprintf(path, "/proc/%d/comm", proc->pid);
    file = fopen(path, "r");
    if (!file) return 0;
    
    if (fgets(proc->name, sizeof(proc->name), file)) {
        // Remove trailing newline
        size_t len = strlen(proc->name);
        if (len > 0 && proc->name[len-1] == '\n') {
            proc->name[len-1] = '\0';
        }
    }
    fclose(file);
    
    // Read memory info from /proc/[pid]/status
    sprintf(path, "/proc/%d/status", proc->pid);
    file = fopen(path, "r");
    if (!file) return 0;
    
    proc->rss = 0;
    proc->vm_size = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmSize:", 7) == 0) {
            proc->vm_size = strtoul(line + 7, NULL, 10);
        } else if (strncmp(line, "VmRSS:", 6) == 0) {
            proc->rss = strtoul(line + 6, NULL, 10);
        }
    }
    fclose(file);
    
    // Read OOM score
    sprintf(path, "/proc/%d/oom_score", proc->pid);
    file = fopen(path, "r");
    if (file) {
        if (fscanf(file, "%d", &proc->oom_score) != 1) {
            proc->oom_score = 0;
        }
        fclose(file);
    }
    
    // Read OOM score adjustment
    sprintf(path, "/proc/%d/oom_score_adj", proc->pid);
    file = fopen(path, "r");
    if (file) {
        if (fscanf(file, "%d", &proc->oom_score_adj) != 1) {
            proc->oom_score_adj = 0;
        }
        fclose(file);
    }
    
    // Read process start time
    sprintf(path, "/proc/%d/stat", proc->pid);
    file = fopen(path, "r");
    if (file) {
        // Parse the stat file (22nd field is starttime)
        int i;
        char *token;
        char stat_content[1024];
        
        if (fgets(stat_content, sizeof(stat_content), file)) {
            token = strtok(stat_content, " ");
            for (i = 1; i < 22 && token != NULL; i++) {
                token = strtok(NULL, " ");
            }
            
            if (token != NULL) {
                proc->start_time = strtoll(token, NULL, 10);
            }
        }
        fclose(file);
    }
    
    return 1;
}

// Simplified OOM score calculation
double calculate_oom_score(process_info_t* proc, double system_uptime) {
    // Convert start_time to seconds of runtime
    double runtime = system_uptime - (proc->start_time / sysconf(_SC_CLK_TCK));
    if (runtime < 1.0) runtime = 1.0; // Avoid division by zero
    
    // Convert KB to pages (assuming 4KB pages)
    double pages = proc->rss / 4.0;
    
    // Simplified version of the formula
    double score = (pages * 10.0) / (runtime * sqrt(sqrt(runtime)));
    
    // Apply OOM score adjustment
    score = score * (1000.0 - proc->oom_score_adj) / 1000.0;
    
    return score;
}

// Comparison function for qsort
int compare_scores(const void* a, const void* b) {
    process_info_t* p1 = (process_info_t*)a;
    process_info_t* p2 = (process_info_t*)b;
    
    if (p1->calculated_score > p2->calculated_score) return -1;
    if (p1->calculated_score < p2->calculated_score) return 1;
    return 0;
}

int main() {
    DIR* proc_dir;
    struct dirent* entry;
    process_info_t processes[500]; // Adjust size as needed
    int process_count = 0;
    double system_uptime = get_system_uptime();
    
    printf("OOM Killer Simulator\n");
    printf("System uptime: %.2f seconds\n\n", system_uptime);
    
    // Open /proc directory
    proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Failed to open /proc");
        return 1;
    }
    
    // Scan for process directories
    while ((entry = readdir(proc_dir)) != NULL && process_count < 500) {
        // Check if entry is a directory and name is a number (PID)
        if (entry->d_type == DT_DIR) {
            char* endptr;
            long pid = strtol(entry->d_name, &endptr, 10);
            
            if (*endptr == '\0') { // Valid PID
                processes[process_count].pid = (int)pid;
                
                if (get_process_info(&processes[process_count])) {
                    // Calculate our score
                    processes[process_count].calculated_score = 
                        calculate_oom_score(&processes[process_count], system_uptime);
                    process_count++;
                }
            }
        }
    }
    closedir(proc_dir);
    
    // Sort processes by calculated score
    qsort(processes, process_count, sizeof(process_info_t), compare_scores);
    
    // Print top 20 most likely victims
    printf("Top 20 processes most likely to be killed by the OOM killer:\n");
    printf("%-8s %-20s %-10s %-10s %-15s %-10s %-12s %-12s\n", 
           "PID", "Name", "RSS(MB)", "VM(MB)", "Runtime(s)", "OOM Score", "OOM Adj", "Calc Score");
    printf("%-8s %-20s %-10s %-10s %-15s %-10s %-12s %-12s\n",
           "--------", "--------------------", "----------", "----------", "---------------", "----------", "------------", "------------");
    
    for (int i = 0; i < 20 && i < process_count; i++) {
        double runtime = system_uptime - (processes[i].start_time / sysconf(_SC_CLK_TCK));
        
        printf("%-8d %-20.20s %-10.1f %-10.1f %-15.1f %-10d %-12d %-12.1f\n",
               processes[i].pid,
               processes[i].name,
               processes[i].rss / 1024.0,   // Convert KB to MB
               processes[i].vm_size / 1024.0, // Convert KB to MB
               runtime,
               processes[i].oom_score,
               processes[i].oom_score_adj,
               processes[i].calculated_score);
    }
    
    printf("\nNote: This is a simplified simulation. The actual Linux OOM killer\n");
    printf("uses more complex heuristics and considers additional factors.\n");
    
    return 0;
}
```

**Expected Output:**
```
OOM Killer Simulator
System uptime: 86400.25 seconds

Top 20 processes most likely to be killed by the OOM killer:
PID      Name                 RSS(MB)    VM(MB)     Runtime(s)      OOM Score  OOM Adj      Calc Score  
-------- -------------------- ---------- ---------- --------------- ---------- ------------ ------------
23456    firefox             1250.5     3045.2     3600.5          541        0            432.1
12345    chrome              1150.2     2845.7     7200.3          498        0            321.5
34567    java                825.3      1567.4     21600.2         321        0            198.7
45678    node                678.9      1245.6     43200.1         256        0            145.2
56789    mysql               512.4      875.3      86400.2         189        -100         87.3
... [additional processes] ...

Note: This is a simplified simulation. The actual Linux OOM killer
uses more complex heuristics and considers additional factors.
```

This simulation helps visualize how the OOM killer scores processes and selects victims during memory exhaustion.

### Badness Function Implementation

The kernel implements this logic in the `oom_badness()` function, which examines each process's memory consumption and applies various weighting factors. The function considers:

- Pages mapped in multiple locations (counted only once)
- Pages that can be easily reclaimed
- Process hierarchies (child processes contribute to parent scores)
- System criticality flags

The resulting score is cached but recalculated periodically to account for changing conditions.

### Process Termination Sequence

When the OOM killer selects a victim, it follows a systematic termination procedure:

1. **Signal delivery**: The process receives a SIGKILL signal.
2. **Memory release**: The kernel begins reclaiming the process's memory.
3. **Resource cleanup**: Other resources (file descriptors, locks) are released.
4. **Parent notification**: The parent process is notified via the standard signal mechanism.
5. **Logging**: The event is logged to the kernel log with details about the selection criteria.

In some cases, multiple processes may need to be terminated before sufficient memory is reclaimed.

### OOM Control via Cgroups

Control groups (cgroups) provide fine-grained control over the OOM killer's behavior. Each cgroup can specify:

- Memory limits (`memory.limit_in_bytes`)
- OOM control policy (`memory.oom_control`)
- OOM score adjustments (`memory.oom_score_adj`)

This allows system administrators to isolate critical services from less important ones, ensuring that OOM events affect only non-essential components.

#### Demonstrating Cgroup Memory Limits

This script demonstrates how to create and use memory cgroups to control OOM behavior:

```bash
#!/bin/bash
# Note: This script requires root privileges to run

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

# Ensure cgroup filesystem is mounted
if ! grep -q "cgroup" /proc/mounts; then
    echo "Cgroup filesystem not mounted. Please ensure cgroups are enabled."
    exit 1
fi

# Create a test cgroup
CGROUP_PATH="/sys/fs/cgroup/memory/test-oom"
mkdir -p $CGROUP_PATH

# Create a simple C program for testing
cat > oom_test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    size_t chunk_size = 10 * 1024 * 1024; // 10 MB chunks
    int delay_ms = 100; // Delay between allocations in milliseconds
    size_t limit_mb = 0; // 0 means unlimited
    
    if (argc > 1) chunk_size = atol(argv[1]) * 1024 * 1024;
    if (argc > 2) delay_ms = atoi(argv[2]);
    if (argc > 3) limit_mb = atol(argv[3]);
    
    printf("PID: %d\n", getpid());
    printf("Allocating memory in %zu MB chunks every %d ms\n", 
           chunk_size / (1024 * 1024), delay_ms);
    if (limit_mb) printf("Will stop after allocating %zu MB\n", limit_mb);
    printf("Press Ctrl+C to stop\n\n");
    
    size_t total_allocated = 0;
    
    while (1) {
        void *mem = malloc(chunk_size);
        if (mem == NULL) {
            printf("malloc failed after %zu MB\n", total_allocated / (1024 * 1024));
            break;
        }
        
        // Touch memory to ensure it's allocated
        memset(mem, 1, chunk_size);
        
        total_allocated += chunk_size;
        printf("Allocated %zu MB total\n", total_allocated / (1024 * 1024));
        
        if (limit_mb > 0 && total_allocated >= limit_mb * 1024 * 1024) {
            printf("Reached allocation limit of %zu MB\n", limit_mb);
            break;
        }
        
        usleep(delay_ms * 1000);
    }
    
    printf("Allocation complete. Sleeping forever...\n");
    while(1) sleep(60);
    
    return 0;
}
EOF

# Compile the test program
gcc -o oom_test oom_test.c

# Configure the cgroup
echo "1" > $CGROUP_PATH/memory.oom_control # 1 = disable the OOM killer
echo "100M" > $CGROUP_PATH/memory.limit_in_bytes # 100 MB limit
echo "0" > $CGROUP_PATH/memory.swappiness # Disable swapping

echo "Created cgroup with the following settings:"
echo "Memory limit: $(cat $CGROUP_PATH/memory.limit_in_bytes)"
echo "OOM control: $(cat $CGROUP_PATH/memory.oom_control)"
echo "Swappiness: $(cat $CGROUP_PATH/memory.swappiness)"

# Now run our test program in the cgroup
echo "Running test program in cgroup..."
echo $$ > $CGROUP_PATH/cgroup.procs

# Start monitoring
(
    while true; do
        echo "Current memory usage: $(cat $CGROUP_PATH/memory.usage_in_bytes) bytes"
        sleep 1
    done
) &
MONITOR_PID=$!

# Run the test program with 10MB chunks, 500ms delay, no upper limit
./oom_test 10 500 0

# Clean up
kill $MONITOR_PID
rm -f oom_test oom_test.c
rmdir $CGROUP_PATH

echo "Test complete"
```

**Expected Output:**
```
Created cgroup with the following settings:
Memory limit: 104857600
OOM control: oom_kill_disable 1
Swappiness: 0
Running test program in cgroup...
PID: 12345
Allocating memory in 10 MB chunks every 500 ms
Press Ctrl+C to stop

Allocated 10 MB total
Current memory usage: 10485760 bytes
Allocated 20 MB total
Current memory usage: 20971520 bytes
...
Allocated 90 MB total
Current memory usage: 94371840 bytes
Allocated 100 MB total
malloc failed after 100 MB
Allocation complete.
```

## Future Directions in Linux Memory Management

## Advanced Swap Technologies
Several innovations are improving swap performance:

- **zswap**: Compressed cache for swap pages, reducing I/O
- **zram**: Compressed RAM-based swap device
- **bcache**: SSD-based caching for swap on slower devices
- **swap on NVMe**: Utilizing high-performance storage for swap

These technologies reduce the performance penalty of swapping, making it a more viable response to memory pressure.

## Persistent Memory Integration
Intel Optane and similar persistent memory technologies are changing memory management:

- **DAX (Direct Access)**: Bypasses the page cache for persistent memory
- **KMEM DAX**: Allows using persistent memory as normal RAM
- **Tiered memory**: Automatic placement of data based on access patterns

These capabilities create a continuum between memory and storage, potentially eliminating traditional swap in favor of direct persistent memory access.

## Machine Learning for Memory Management
Emerging research applies machine learning to memory management:

- **Predictive page replacement**: Learning application memory access patterns
- **Intelligent OOM scoring**: Using workload history to make better termination decisions
- **Adaptive parameter tuning**: Automatically adjusting parameters based on workload

While still primarily research topics, these approaches show promise for workloads with predictable memory usage patterns.

## Enhanced Memory Pressure Signaling
Improvements in memory pressure detection include:

- **PSI (Pressure Stall Information)**: Fine-grained memory pressure metrics
- **cgroup v2 memory controller**: Improved isolation and accounting
- **Pre-OOM notification hooks**: Allowing applications to respond to pressure

These mechanisms enable more proactive responses to memory pressure, potentially avoiding OOM situations entirely.

## Conclusion
Linux memory management during exhaustion represents a sophisticated balance between optimizing resource utilization and maintaining system stability. From initial allocation through reclamation, swapping, and potentially process termination, the kernel employs increasingly aggressive strategies to maintain functionality.

The OOM killer, while sometimes criticized for its choices, serves as a critical last line of defense against complete system failure. Modern improvements in kernel algorithms, hardware integration, and userspace tools continue to refine this system, making Linux increasingly resilient even under extreme memory pressure.

Understanding these mechanisms enables system administrators and developers to configure systems appropriately, design applications that behave well under memory pressure, and diagnose issues when memory exhaustion occurs despite preventive measures.