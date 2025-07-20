---
layout: post
title: "Can We Swap Processes Without a Hitch? Implementing Context Switching to Optimize OS Performance"
date: 2024-11-04 12:00:00 -0400
categories: [System Programming & OS]
tags: [Context Switching, Process Scheduling, Operating Systems, Performance Optimization, CPU Management, Kernel Development]
author: mohitmishra786
description: "An exploration of context switching optimization techniques, examining how efficient process swapping can enhance operating system performance and minimize overhead."
toc: true
---
## Table of Contents

* [1. Introduction](#1-introduction)
* [2. What is Context Switching?](#2-what-is-context-switching)
    * [Flow of Context Switch](#flow-of-context-switch)
* [3. Anatomy of a Context Switch](#3-anatomy-of-a-context-switch)
* [4. Process Control Block (PCB)](#4-process-control-block-pcb)
* [5. Hardware Context vs. Software Context](#5-hardware-context-vs-software-context)
    * [Types of Contexts](#types-of-contexts)
* [6. Hardware Support for Context Switching](#6-hardware-support-for-context-switching)
* [7. Context Switch Triggers](#7-context-switch-triggers)
* [8. The Context Switch Mechanism](#8-the-context-switch-mechanism)
* [9. Context Switching Costs](#9-context-switching-costs)
* [10. Implementation of Context Switch Handler](#10-implementation-of-context-switch-handler)
* [11. Real-world Examples](#11-real-world-examples)
* [12. Performance Considerations](#12-performance-considerations)
* [13. Conclusion](#13-conclusion)


## About Me
- [My Github](https://github.com/mohitmishra786)
- [Personal Tech Blogs](https://mohitmishra786.github.io/chessman)
- [Medium](https://medium.com/@mohitmishra786687)
- [Book Repo](https://github.com/mohitmishra786/myJourneyOfBuildingOS)

## 1. Introduction

Context switching is one of the most fundamental operations in modern operating systems, enabling multitasking and concurrent execution of processes. While it appears seamless to end-users, the underlying mechanism is intricate and involves complex interactions between hardware and software. This article explains the internals of context switching, examining its theoretical foundations and practical implementation details.

## 2. What is Context Switching?

At its core, context switching is the mechanism by which the CPU switches from executing one process to another. This operation involves saving the state of the currently running process and loading the saved state of another scheduled process.

### Flow of Context Switch

At its core, a process context consists of:

```c
struct process_context {
    // CPU Registers
    uint64_t rax, rbx, rcx, rdx;    // General purpose registers
    uint64_t rsi, rdi;              // Source and destination index registers
    uint64_t rbp, rsp;              // Stack base and stack pointer
    uint64_t r8, r9, r10, r11;      // Extended registers
    uint64_t r12, r13, r14, r15;    // Extended registers
    uint64_t rip;                   // Instruction pointer
    uint64_t rflags;                // CPU flags

    // Segment Registers
    uint16_t cs, ds, es, fs, gs, ss;

    // Control Registers
    uint64_t cr3;                   // Page table base register

    // FPU/SSE State
    uint8_t fpu_state[512];         // FPU and SSE registers state
};
```

This structure represents the minimal context that must be saved and restored during a context switch. Let’s examine each component:
1. **General Purpose Registers:** Store intermediate computational results
2. **Stack Pointers:** Track the current execution stack
3. **Instruction Pointer:** Points to the next instruction to execute
4. **CPU Flags:** Store status information about the last arithmetic operation
5. **Segment Registers:** Used for memory segmentation
6. **CR3:** Contains the physical address of the page directory
7. **FPU/SSE State:** Floating-point and SIMD execution state


## 3. Anatomy of a Context Switch

A context switch can occur for several reasons:
1. Timer interrupt (preemption)
2. System call
3. I/O request
4. Inter-process communication

Here’s a simplified view of the context switch handler:

```c
void context_switch(struct process_context* old_context,
                   struct process_context* new_context) {
    // 1. Save CPU registers of current process
    save_cpu_state(old_context);

    // 2. Save FPU/SSE state if used
    if (fpu_used()) {
        save_fpu_state(old_context->fpu_state);
    }

    // 3. Switch to new page tables
    load_cr3(new_context->cr3);

    // 4. Restore FPU/SSE state if necessary
    if (fpu_used()) {
        restore_fpu_state(new_context->fpu_state);
    }

    // 5. Restore CPU registers of new process
    restore_cpu_state(new_context);
}

void save_cpu_state(struct process_context* ctx) {
    __asm__ volatile (
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        : "=m" (ctx->rax), "=m" (ctx->rbx),
          "=m" (ctx->rcx), "=m" (ctx->rdx),
          "=m" (ctx->rsi), "=m" (ctx->rdi),
          "=m" (ctx->rbp), "=m" (ctx->rsp)
        :
        : "memory"
    );
}
```

## 4. Process Control Block (PCB)

The Process Control Block (PCB), also known as the process descriptor, is a data structure that contains all the information needed to save and restore a process's context. Here's a typical PCB structure in C:

```c
struct process_control_block {
    // Process identification
    pid_t pid;                    // Process ID
    pid_t parent_pid;            // Parent process ID

    // CPU registers
    struct cpu_context {
        unsigned long rax, rbx, rcx, rdx;    // General purpose registers
        unsigned long rsi, rdi, rbp;         // Source, destination, base pointer
        unsigned long rsp;                   // Stack pointer
        unsigned long rip;                   // Instruction pointer
        unsigned long rflags;                // CPU flags
        unsigned long cs, ss, ds, es, fs, gs; // Segment registers
    } cpu_regs;

    // Memory management
    struct mm_struct {
        unsigned long pgd;        // Page Global Directory
        unsigned long start_code, end_code;  // Text segment
        unsigned long start_data, end_data;  // Data segment
        unsigned long start_stack;           // Stack segment
        unsigned long start_brk, brk;        // Heap segment
    } mm;

    // Process state
    enum process_state {
        TASK_RUNNING,
        TASK_INTERRUPTIBLE,
        TASK_UNINTERRUPTIBLE,
        TASK_STOPPED,
        TASK_ZOMBIE
    } state;

    // Scheduling information
    int priority;
    int static_priority;
    unsigned long time_slice;
    unsigned long total_time;    // Total CPU time used

    // File management
    struct files_struct *files;  // Open file descriptors

    // Signal handling
    sigset_t signal_mask;       // Blocked signals
    struct sighand_struct *sighand;  // Signal handlers
};
```

## 5. Hardware Context vs. Software Context

Context switching involves two main types of contexts:
1. Hardware Context: Involves CPU registers, program counter, stack pointer, status registers, and memory management registers.
2. Software Context: Includes process ID, parent process ID, privileges, priorities, memory management data, I/O status, and accounting information.

### Types of Contexts

## 6. Hardware Support for Context Switching

Modern CPUs provide several features to optimize context switching:
### Task State Segment (TSS)

```c
struct tss_struct {
    uint32_t reserved1;
    uint64_t rsp0;        // Stack pointer for ring 0
    uint64_t rsp1;        // Stack pointer for ring 1
    uint64_t rsp2;        // Stack pointer for ring 2
    uint64_t reserved2;
    uint64_t ist[7];      // Interrupt stack table
    uint32_t reserved3;
    uint32_t reserved4;
    uint16_t reserved5;
    uint16_t iopb_offset; // I/O permission bitmap offset
} __attribute__((packed));

void setup_tss(struct tss_struct* tss) {
    memset(tss, 0, sizeof(struct tss_struct));
    tss->rsp0 = KERNEL_STACK_TOP;
    tss->iopb_offset = sizeof(struct tss_struct);
}
```

### Memory Management Unit (MMU)

```c
struct page_table_entry {
    uint64_t present:1;
    uint64_t writable:1;
    uint64_t user_accessible:1;
    uint64_t write_through:1;
    uint64_t cache_disabled:1;
    uint64_t accessed:1;
    uint64_t dirty:1;
    uint64_t page_size:1;
    uint64_t global:1;
    uint64_t available:3;
    uint64_t page_frame:40;
    uint64_t reserved:11;
    uint64_t nx:1;
} __attribute__((packed));
```


## 7. Context Switch Triggers

Various events can trigger context switches:
1. Timer Interrupt
2. System Calls
3. I/O Operations
4. Inter-process Communication
5. Signal Handling

Here's a simplified implementation of a timer interrupt handler:

```c
void timer_interrupt_handler(void) {
    // Disable interrupts
    cli();

    // Save current process context
    save_context(&current_process->cpu_regs);

    // Update process statistics
    current_process->total_time += time_since_last_switch;

    // Run scheduler to select next process
    schedule();

    // Enable interrupts
    sti();
}

void save_context(struct cpu_context *ctx) {
    asm volatile(
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        : "=m" (ctx->rax), "=m" (ctx->rbx),
          "=m" (ctx->rcx), "=m" (ctx->rdx),
          "=m" (ctx->rsi), "=m" (ctx->rdi),
          "=m" (ctx->rbp), "=m" (ctx->rsp)
        :
        : "memory"
    );
}
```

## 8. The Context Switch Mechanism

The context switch process involves several steps:
1. Save the current process state
2. Update PCB
3. Select next process
4. Load new process state
5. Update memory management
6. Resume execution

Here's a low-level implementation of the context switch mechanism:

```c
void context_switch(struct process_control_block *prev,
                   struct process_control_block *next) {
    // Save CPU registers of current process
    save_cpu_context(&prev->cpu_regs);

    // Save FPU state if necessary
    if (prev->cpu_regs.fpu_used) {
        save_fpu_state(&prev->fpu_state);
    }

    // Update page table (CR3 register on x86)
    unsigned long next_pgd = next->mm.pgd;
    asm volatile(
        "movq %0, %%cr3"
        :
        : "r" (next_pgd)
        : "memory"
    );

    // Load CPU registers of new process
    restore_cpu_context(&next->cpu_regs);

    // Restore FPU state if necessary
    if (next->cpu_regs.fpu_used) {
        restore_fpu_state(&next->fpu_state);
    }

    // Update TSS (Task State Segment)
    update_tss(next);
}

void save_cpu_context(struct cpu_context *ctx) {
    // Save general-purpose registers
    asm volatile(
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        "pushfq\n"
        "popq %8\n"
        : "=m" (ctx->rax), "=m" (ctx->rbx),
          "=m" (ctx->rcx), "=m" (ctx->rdx),
          "=m" (ctx->rsi), "=m" (ctx->rdi),
          "=m" (ctx->rbp), "=m" (ctx->rsp),
          "=m" (ctx->rflags)
        :
        : "memory"
    );
}
```


## 9. Context Switching Costs

Context switching introduces various overhead costs:
1. Direct Costs: Involving saving and restoring registers, switching address spaces, and updating kernel data structures.
2. Indirect Costs: Including cold cache effects, TLB flush, pipeline flush, and branch predictor reset.

### Context Switching Cost


## 10. Implementation of Context Switch Handler

```c
#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/* CPU registers structure */
struct cpu_registers {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi;
    uint64_t rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip;
    uint64_t rflags;
    uint16_t cs, ds, es, fs, gs, ss;
    bool fpu_used;
    uint32_t mxcsr;
    uint16_t fpu_cw;
    uint8_t fpu_state[512] __attribute__((aligned(16)));
};

/* Memory management context */
struct mm_context {
    uint64_t pgd;
    uint64_t context_id;
    uint64_t start_code, end_code;
    uint64_t start_data, end_data;
    uint64_t start_brk, brk;
    uint64_t start_stack;
    uint64_t arg_start, arg_end;
    uint64_t env_start, env_end;
};

struct file {
    int fd;
};

struct io_context {
    int nr_files;
    struct file *files[256];
    int cwd_fd;
    uint64_t umask;
};

struct sched_info {
    int policy;
    int priority;
    uint64_t time_slice;
    uint64_t total_time;
    uint64_t start_time;
    uint64_t last_ran;
    int processor;
    uint64_t preempt_count;
};

enum process_state {
    TASK_RUNNING = 0,
    TASK_INTERRUPTIBLE = 1,
    TASK_UNINTERRUPTIBLE = 2,
    TASK_STOPPED = 4,
    TASK_ZOMBIE = 8
};

struct sighand_struct {
    void (*handlers[32])(int);
};

/* Process Control Block structure */
struct pcb {
    pid_t pid;
    pid_t ppid;
    uid_t uid;
    gid_t gid;
    char name[16];  // Added name field for better identification

    enum process_state state;
    struct cpu_registers regs;
    struct mm_context mm;
    struct io_context io;
    struct sched_info sched;

    struct pcb *parent;
    struct pcb *children;
    struct pcb *siblings;

    sigset_t blocked;
    sigset_t pending;
    struct sighand_struct *sighand;

    unsigned long flags;

    struct rusage rusage;
    struct rusage rusage_children;

    uint64_t voluntary_switches;
    uint64_t involuntary_switches;
};

/* Performance statistics structure */
struct perf_stats {
    uint64_t context_switches;
    uint64_t cache_misses;
    uint64_t tlb_misses;
    uint64_t branch_misses;
    uint64_t avg_switch_time;
    uint64_t max_switch_time;
};

#define MAX_PROCESSES 4
static struct perf_stats global_stats = {0};
static struct pcb *current_process = NULL;
static struct pcb processes[MAX_PROCESSES];
static int num_processes = 0;

static inline uint64_t get_current_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static uint64_t calculate_time_slice(struct pcb *process) {
    // Give each process a smaller time slice (between 1-5 ticks based on priority)
    return 5 - (process->sched.priority % 5);
}

static void save_cpu_state(struct cpu_registers *regs) {
    // Simulate saving CPU state with some dummy values
    regs->rax = rand() % 1000;
    regs->rbx = rand() % 1000;
    regs->rcx = rand() % 1000;
}

static void restore_cpu_state(struct cpu_registers *regs) {
    // Simulate restoring CPU state
    printf("Restoring CPU state: RAX=%lu, RBX=%lu, RCX=%lu\n",
           regs->rax, regs->rbx, regs->rcx);
}

static void save_fpu_state(struct cpu_registers *regs) {
    if (regs->fpu_used) {
        regs->mxcsr = 0x1F80;
    }
}

static void restore_fpu_state(struct cpu_registers *regs) {
    if (regs->fpu_used) {
        (void)regs;
    }
}

static void switch_mm_context(struct pcb *prev, struct pcb *next) {
    if (prev->mm.pgd != next->mm.pgd) {
        printf("Memory context switch: %s (PGD %lx) -> %s (PGD %lx)\n",
               prev->name, prev->mm.pgd, next->name, next->mm.pgd);
    }
}

struct pcb* scheduler_select_next(void) {
    static int current_index = 0;
    int starting_index = current_index;

    do {
        current_index = (current_index + 1) % num_processes;
        if (processes[current_index].state != TASK_RUNNING) {
            continue;
        }
        return &processes[current_index];
    } while (current_index != starting_index);

    return current_process; // Fallback to current if no other process is ready
}

static void update_process_accounting(struct pcb *prev, struct pcb *next) {
    uint64_t current_time = get_current_time();
    prev->sched.total_time += current_time - prev->sched.last_ran;
    next->sched.last_ran = current_time;
    next->sched.time_slice = calculate_time_slice(next);

    if (prev->state == TASK_INTERRUPTIBLE) {
        prev->voluntary_switches++;
    } else {
        prev->involuntary_switches++;
    }
}

static void update_perf_stats(uint64_t switch_start_time) {
    uint64_t switch_time = get_current_time() - switch_start_time;

    global_stats.context_switches++;
    global_stats.avg_switch_time =
        (global_stats.avg_switch_time * (global_stats.context_switches - 1) +
         switch_time) / global_stats.context_switches;

    if (switch_time > global_stats.max_switch_time) {
        global_stats.max_switch_time = switch_time;
    }
}

int context_switch(struct pcb *prev, struct pcb *next) {
    uint64_t switch_start_time = get_current_time();

    printf("\nContext switch: %s (PID %d) -> %s (PID %d)\n",
           prev->name, prev->pid, next->name, next->pid);

    save_cpu_state(&prev->regs);
    save_fpu_state(&prev->regs);

    prev->state = TASK_INTERRUPTIBLE;
    next->state = TASK_RUNNING;

    switch_mm_context(prev, next);
    update_process_accounting(prev, next);

    restore_fpu_state(&next->regs);
    restore_cpu_state(&next->regs);

    current_process = next;
    update_perf_stats(switch_start_time);

    return 0;
}

void timer_interrupt_handler(void) {
    struct pcb *prev = current_process;
    struct pcb *next;

    printf("\nTimer interrupt: Current process = %s\n", prev->name);

    // Force a switch after time slice expires or every few cycles
    if (--prev->sched.time_slice == 0 || (rand() % 3 == 0)) {
        printf("Time slice expired for %s\n", prev->name);
        next = scheduler_select_next();

        if (next != prev) {
            context_switch(prev, next);
        } else {
            prev->sched.time_slice = calculate_time_slice(prev);
            printf("No other processes ready, continuing with %s\n", prev->name);
        }
    } else {
        printf("Remaining time slice for %s: %lu\n",
               prev->name, prev->sched.time_slice);
    }
}

static void initialize_process(struct pcb *proc, const char *name, int priority) {
    proc->pid = getpid() + num_processes;
    proc->ppid = getppid();
    proc->uid = getuid();
    proc->gid = getgid();
    snprintf(proc->name, sizeof(proc->name), "%s", name);

    proc->state = TASK_RUNNING;
    proc->sched.priority = priority;
    proc->sched.time_slice = calculate_time_slice(proc);
    proc->sched.last_ran = get_current_time();

    // Simulate different memory contexts
    proc->mm.pgd = (uint64_t)rand() << 32 | rand();
    proc->regs.fpu_used = (rand() % 2) == 0;

    num_processes++;
}

int main(void) {
    // Adding seed for random number generation
    srand(time(NULL));

    printf("Context Switch Simulation Starting\n");
    printf("==================================\n\n");

    // Initialize multiple processes with different priorities
    initialize_process(&processes[0], "Init", 2);
    initialize_process(&processes[1], "Web_Server", 1);
    initialize_process(&processes[2], "Database", 0);
    initialize_process(&processes[3], "Backup", 3);

    current_process = &processes[0];

    printf("Initialized Processes:\n");
    for (int i = 0; i < num_processes; i++) {
        printf("- %s (PID: %d, Priority: %d, Initial time slice: %lu)\n",
               processes[i].name, processes[i].pid,
               processes[i].sched.priority,
               processes[i].sched.time_slice);
    }
    printf("\nStarting simulation...\n");
    printf("==================================\n");

    // Simulate several timer interrupts
    for (int i = 0; i < 10; i++) {
        printf("\nSimulation Cycle %d\n", i + 1);
        printf("-----------------\n");

        timer_interrupt_handler();

        printf("\nStatistics after cycle %d:\n", i + 1);
        printf("Context switches: %lu\n", global_stats.context_switches);
        printf("Average switch time: %lu ns\n", global_stats.avg_switch_time);
        printf("Maximum switch time: %lu ns\n", global_stats.max_switch_time);

        usleep(100000); // Sleep for 100ms between simulations
    }

    printf("\nSimulation Complete\n");
    printf("==================================\n");
    printf("Final Statistics:\n");
    printf("Total context switches: %lu\n", global_stats.context_switches);
    printf("Final average switch time: %lu ns\n", global_stats.avg_switch_time);
    printf("Maximum switch time: %lu ns\n", global_stats.max_switch_time);

    return 0;
}
```

To compile and run:

```bash
gcc -o contextSwitch contextSwitchHandler.c -Wall
./contextSwitch
```

The output of this program will be like below:

```bash
Context Switch Simulation Starting
==================================

Initialized Processes:
- Init (PID: 29766, Priority: 2, Initial time slice: 3)
- Web_Server (PID: 29767, Priority: 1, Initial time slice: 4)
- Database (PID: 29768, Priority: 0, Initial time slice: 5)
- Backup (PID: 29769, Priority: 3, Initial time slice: 2)

Starting simulation...
==================================

Simulation Cycle 1
-----------------

Timer interrupt: Current process = Init
Remaining time slice for Init: 2

Statistics after cycle 1:
Context switches: 0
Average switch time: 0 ns
Maximum switch time: 0 ns

Simulation Cycle 2
-----------------

Timer interrupt: Current process = Init
Time slice expired for Init

Context switch: Init (PID 29766) -> Web_Server (PID 29767)
Memory context switch: Init (PGD 4bc44a9a0f2f555b) -> Web_Server (PGD 521893f200a6d3ac)
Restoring CPU state: RAX=0, RBX=0, RCX=0

Statistics after cycle 2:
Context switches: 1
Average switch time: 18385 ns
Maximum switch time: 18385 ns

Simulation Cycle 3
-----------------

Timer interrupt: Current process = Web_Server
Time slice expired for Web_Server

Context switch: Web_Server (PID 29767) -> Database (PID 29768)
Memory context switch: Web_Server (PGD 521893f200a6d3ac) -> Database (PGD 2a2921bb36ad0e2d)
Restoring CPU state: RAX=0, RBX=0, RCX=0

Statistics after cycle 3:
Context switches: 2
Average switch time: 18437 ns
Maximum switch time: 18489 ns

Simulation Cycle 4
-----------------

Timer interrupt: Current process = Database
Remaining time slice for Database: 4

Statistics after cycle 4:
Context switches: 2
Average switch time: 18437 ns
Maximum switch time: 18489 ns

Simulation Cycle 5
-----------------

Timer interrupt: Current process = Database
Time slice expired for Database

Context switch: Database (PID 29768) -> Backup (PID 29769)
Memory context switch: Database (PGD 2a2921bb36ad0e2d) -> Backup (PGD 7fce95be7d71350d)
Restoring CPU state: RAX=0, RBX=0, RCX=0

Statistics after cycle 5:
Context switches: 3
Average switch time: 18445 ns
Maximum switch time: 18489 ns

Simulation Cycle 6
-----------------

Timer interrupt: Current process = Backup
Time slice expired for Backup
No other processes ready, continuing with Backup

Statistics after cycle 6:
Context switches: 3
Average switch time: 18445 ns
Maximum switch time: 18489 ns

Simulation Cycle 7
-----------------

Timer interrupt: Current process = Backup
Time slice expired for Backup
No other processes ready, continuing with Backup

Statistics after cycle 7:
Context switches: 3
Average switch time: 18445 ns
Maximum switch time: 18489 ns

Simulation Cycle 8
-----------------

Timer interrupt: Current process = Backup
Time slice expired for Backup
No other processes ready, continuing with Backup

Statistics after cycle 8:
Context switches: 3
Average switch time: 18445 ns
Maximum switch time: 18489 ns

Simulation Cycle 9
-----------------

Timer interrupt: Current process = Backup
Remaining time slice for Backup: 1

Statistics after cycle 9:
Context switches: 3
Average switch time: 18445 ns
Maximum switch time: 18489 ns

Simulation Cycle 10
-----------------

Timer interrupt: Current process = Backup
Time slice expired for Backup
No other processes ready, continuing with Backup

Statistics after cycle 10:
Context switches: 3
Average switch time: 18445 ns
Maximum switch time: 18489 ns

Simulation Complete
==================================
Final Statistics:
Total context switches: 3
Final average switch time: 18445 ns
Maximum switch time: 18489 ns
```


## 11. Real-world Examples

Let's examine a complete context switch scenario:

```c
// System call that might trigger a context switch
SYSCALL_DEFINE4(sched_yield, void) {
    struct task_struct *prev, *next;
    struct rq *rq;

    // Get current run queue
    rq = this_rq();
    prev = rq->curr;

    // Find next task to run
    next = pick_next_task(rq, prev);
    if (next == prev)
        return 0;

    // Prepare for context switch
    rq->curr = next;
    ++rq->nr_switches;

    // Perform the actual switch
    struct context_switch_flags flags = {
        .save_fpu = test_thread_flag(TIF_NEED_FPU_LOAD),
        .save_vec = test_thread_flag(TIF_NEED_VEC_LOAD)
    };

    perform_context_switch(prev, next, flags);

    return 0;
}
```

## 12. Performance Considerations

Several factors affect context-switching performance:
1. Hardware Architecture: Influenced by cache size/hierarchy, TLB size/organization, pipeline depth, and register file size.
2. Software Design: Affected by process scheduling algorithms, memory management policies, and I/O scheduling.
Here's a monitoring implementation:


```c
struct context_switch_stats {
    unsigned long total_switches;
    unsigned long voluntary_switches;
    unsigned long involuntary_switches;
    unsigned long avg_switch_time;
    unsigned long max_switch_time;
    unsigned long cache_misses;
    unsigned long tlb_misses;
};

void update_context_switch_stats(struct process_control_block *prev,
                               struct process_control_block *next,
                               unsigned long switch_time) {
    struct context_switch_stats *stats = &system_stats.cs_stats;

    // Update counters
    stats->total_switches++;
    if (prev->state == TASK_INTERRUPTIBLE)
        stats->voluntary_switches++;
    else
        stats->involuntary_switches++;

    // Update timing statistics
    stats->avg_switch_time =
        (stats->avg_switch_time * (stats->total_switches - 1) +
         switch_time) / stats->total_switches;

    if (switch_time > stats->max_switch_time)
        stats->max_switch_time = switch_time;

    // Update cache and TLB statistics
    update_cache_stats();
    update_tlb_stats();
}
```
## Further Reading
### Books

1. **Modern Operating Systems** by Andrew S. Tanenbaum and Herbert Bos
   - [Amazon Link](https://www.amazon.com/Modern-Operating-Systems-Andrew-Tanenbaum/dp/013359162X)

2. **Operating System Concepts** by Abraham Silberschatz, Peter B. Galvin, and Greg Gagne
   - [Amazon Link](https://www.amazon.com/Operating-System-Concepts-Abraham-Silberschatz/dp/1119456339)

3. **The Design of the UNIX Operating System** by Maurice J. Bach
   - [Amazon Link](https://www.amazon.com/Design-UNIX-Operating-System/dp/0132017997)

4. **Operating Systems: Internals and Design Principles** by William Stallings
   - [Amazon Link](https://www.amazon.com/Operating-Systems-Internals-Design-Principles/dp/0134670957)

5. **Linux Kernel Development** by Robert Love
   - [Amazon Link](https://www.amazon.com/Linux-Kernel-Development-Robert-Love/dp/0672329468)

### YouTube Playlists

1. **Linux Operating System Tutorials** by ProgrammingKnowledge
   - [YouTube Link](https://www.youtube.com/playlist?list=PLZ66CyB1h160X8_1eZGsW4P14RqI_kZ-)

2. **Operating Systems (OS) Playlist** by Neso Academy
   - [YouTube Link](https://www.youtube.com/playlist?list=PLE7DDD91010BD588B)

3. **CS 162: Operating Systems and Systems Programming** by UC Berkeley CS
   - [YouTube Link](https://youtube.com/playlist?list=PLF2K2xZjNEf97A_uBCwEl61sdxWVP7VWC&si=d6v0Ojivm6zWDF83)

4. **OS Concepts and Design** by Gate Lectures by Ravindrababu Ravula
   - [YouTube Link](https://www.youtube.com/playlist?list=PLXXV6p_49jVmqW846O32VLwH9b04XzMnG)

5. **Operating System Fundamentals** by CS Dojo
   - [YouTube Link](https://www.youtube.com/playlist?list=PLBZBJbE_rGRVnpitdvpdY9952IsKMDNO-)

### Research Papers

1. **"The Multics Virtual Memory: Concepts and Design"** by Robert C. Daley and Jack B. Dennis
   - [ACM Digital Library Link](https://dl.acm.org/doi/10.1145/362969.362976)

2. **"Lottery Scheduling: Flexible Proportional-Share Resource Management"** by Carl A Waldspurger and William E. Weihl
   - [ResearchGate Link](https://www.researchgate.net/publication/2412153_Lottery_Scheduling_Flexible_Proportional-Share_Resource_Management)

3. **"A Comparison of Scheduling Algorithms for Multiprogramming in a Hard-Real-Time Environment"** by Liu, Chung Laung, and James W. Layland
   - [IEEE Xplore Link](https://ieeexplore.ieee.org/document/1087666)

4. **"The Linux Scheduler: a Decade of Wasted Cores"** by Con Kolivas
   - [PDF Link](http://ck.kolivas.org/patches/bfs/scheduler.pdf)

5. **"Scheduling Algorithms for Multiprogramming in a Hard-Real-Time Environment"** by Liu, Chung Laung, and James W. Layland
   - [Semantic Scholar Link](https://www.semanticscholar.org/paper/Scheduling-algorithms-for-multiprogramming-in-a-Liu-Layland/04234f0f306b632d99c2928b11d07ad65c15b257)

## 13. Conclusion

Context switching is a fundamental operation that enables multitasking in modern operating systems. While the overhead cannot be eliminated entirely, understanding the low-level details helps in optimizing system performance and making informed design decisions.
Key takeaways:
1. Context switching encompasses hardware and software state changes.
2. It's inherently complex and expensive.
3. CPU state management is critical for proper implementation.
4. Performance optimization necessitates knowledge of hardware.
5. Monitoring tools are essential for system performance tuning.

This understanding is vital for OS developers, performance engineers, system administrators, and developers of performance-critical applications.
