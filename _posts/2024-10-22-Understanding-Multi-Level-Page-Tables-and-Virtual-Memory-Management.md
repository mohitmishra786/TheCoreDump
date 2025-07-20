---
layout: post
title: "Understanding Multi-Level Page Tables and Virtual Memory Management"
date: 2024-10-22 12:00:00 -0400
categories: [Computer Architecture]
tags: [Virtual Memory, Page Tables, Memory Management, MMU, Operating Systems, Computer Architecture]
author: mohitmishra786
description: "A comprehensive exploration of multi-level page tables and virtual memory management, covering address translation, memory protection, and modern MMU implementations."
toc: true
---

## Table of Contents
1. Introduction
2. The Challenge of Memory Management
3. Multi-Level Page Tables: A Practical Solution
4. Implementation Details
5. Practical Examples with C
6. Memory Sharing and Segmentation
7. Performance Considerations
8. Visual Representation
9. Further Reading
10. Conclusion

## Introduction

Modern operating systems face a significant challenge in managing memory efficiently while providing isolation between processes. This blog post explores the elegant solution of multi-level page tables, getting into their implementation, practical applications, and performance implications.

## The Challenge of Memory Management

In a 64-bit system with a 48-bit virtual address space and 4KB pages, a single-level page table would require 512GB of memory per process - clearly impractical. Let's break down why:

- Virtual address space: 2^48 bytes
- Page size: 4KB (2^12 bytes)
- Number of pages: 2^36
- Page table entry size: 8 bytes
- Total page table size: 2^36 * 8 = 512GB

![image](https://github.com/user-attachments/assets/22add64a-93e8-47b0-bb35-9a5d2d85e33e)


## Multi-Level Page Tables: A Practical Solution

Multi-level page tables solve this problem by breaking down the virtual address into multiple parts:
- Page offset (typically 12 bits for 4KB pages)
- Multiple page table indices (typically 9 bits each)

Let's implement a simple simulation of a two-level page table system in C:

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096
#define PAGE_BITS 12
#define INDEX_BITS 9
#define NUM_ENTRIES (1 << INDEX_BITS)

typedef struct {
    uint64_t physical_page_number : 40;
    uint64_t present : 1;
    uint64_t writable : 1;
    uint64_t user_accessible : 1;
    uint64_t reserved : 21;
} PageTableEntry;

typedef struct {
    PageTableEntry entries[NUM_ENTRIES];
} PageTable;

typedef struct {
    PageTable* level1;
    PageTable** level2;
} TwoLevelPageTable;

// Initialize a two-level page table
TwoLevelPageTable* init_page_table() {
    TwoLevelPageTable* table = malloc(sizeof(TwoLevelPageTable));
    table->level1 = malloc(sizeof(PageTable));
    table->level2 = calloc(NUM_ENTRIES, sizeof(PageTable*));

    // Initialize level 1 entries as not present
    for (int i = 0; i < NUM_ENTRIES; i++) {
        table->level1->entries[i].present = 0;
    }

    return table;
}

// Map a virtual address to a physical address
bool map_address(TwoLevelPageTable* table, uint64_t virtual_addr, uint64_t physical_addr) {
    uint64_t l1_index = (virtual_addr >> (PAGE_BITS + INDEX_BITS)) & ((1 << INDEX_BITS) - 1);
    uint64_t l2_index = (virtual_addr >> PAGE_BITS) & ((1 << INDEX_BITS) - 1);

    // Allocate level 2 table if not present
    if (!table->level1->entries[l1_index].present) {
        table->level2[l1_index] = malloc(sizeof(PageTable));
        table->level1->entries[l1_index].present = 1;
    }

    // Set up the level 2 entry
    PageTableEntry* l2_entry = &table->level2[l1_index]->entries[l2_index];
    l2_entry->physical_page_number = physical_addr >> PAGE_BITS;
    l2_entry->present = 1;
    l2_entry->writable = 1;
    l2_entry->user_accessible = 1;

    return true;
}

// Translate a virtual address to physical address
uint64_t translate_address(TwoLevelPageTable* table, uint64_t virtual_addr) {
    uint64_t l1_index = (virtual_addr >> (PAGE_BITS + INDEX_BITS)) & ((1 << INDEX_BITS) - 1);
    uint64_t l2_index = (virtual_addr >> PAGE_BITS) & ((1 << INDEX_BITS) - 1);
    uint64_t offset = virtual_addr & ((1 << PAGE_BITS) - 1);

    // Check if pages are present
    if (!table->level1->entries[l1_index].present) {
        printf("Page fault: Level 1 entry not present\n");
        return (uint64_t)-1;
    }

    PageTableEntry* l2_entry = &table->level2[l1_index]->entries[l2_index];
    if (!l2_entry->present) {
        printf("Page fault: Level 2 entry not present\n");
        return (uint64_t)-1;
    }

    return (l2_entry->physical_page_number << PAGE_BITS) | offset;
}

int main() {
    TwoLevelPageTable* table = init_page_table();

    // Map some virtual addresses to physical addresses
    uint64_t virtual_addr1 = 0x123456789000;  // Example virtual address
    uint64_t physical_addr1 = 0x987654321000; // Example physical address

    map_address(table, virtual_addr1, physical_addr1);

    // Try to translate addresses
    uint64_t result = translate_address(table, virtual_addr1);
    printf("Virtual address 0x%lx translates to physical address 0x%lx\n",
           virtual_addr1, result);

    // Try an unmapped address
    uint64_t unmapped_addr = 0x999999999000;
    result = translate_address(table, unmapped_addr);
    if (result == (uint64_t)-1) {
        printf("Translation failed for unmapped address 0x%lx\n", unmapped_addr);
    }

    return 0;
}
```

To compile and run this code:
```bash
gcc -o page_table page_table.c
./page_table
```

Expected output:
```
Virtual address 0x123456789000 translates to physical address 0x987654321000
Translation failed for unmapped address 0x999999999000
```

## Memory Sharing and Segmentation

One of the powerful features of virtual memory is the ability to share pages between processes. Here's an implementation demonstrating shared memory between two processes:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define SHARED_MEM_SIZE 4096

int main() {
    // Create shared memory object
    const char *name = "/shared_memory_example";
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    // Set size of shared memory object
    if (ftruncate(fd, SHARED_MEM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // Map shared memory
    void *addr = mmap(NULL, SHARED_MEM_SIZE,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {  // Child process
        // Write to shared memory
        strcpy((char*)addr, "Hello from child process!");
        exit(0);
    } else {  // Parent process
        // Wait for child to finish
        wait(NULL);

        // Read from shared memory
        printf("Parent reads: %s\n", (char*)addr);

        // Clean up
        if (munmap(addr, SHARED_MEM_SIZE) == -1) {
            perror("munmap failed");
            return 1;
        }

        if (shm_unlink(name) == -1) {
            perror("shm_unlink failed");
            return 1;
        }
    }

    return 0;
}
```

To compile and run:
```bash
gcc -o shared_memory shared_memory.c -lrt
./shared_memory
```

Expected output:
```
Parent reads: Hello from child process!
```

## Performance Considerations

To demonstrate the impact of TLB (Translation Lookaside Buffer) hits vs. misses, here's a program that measures memory access patterns:

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define NUM_PAGES 1024
#define NUM_ACCESSES 1000000

uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

int main() {
    // Allocate memory for pages
    char *memory = malloc(PAGE_SIZE * NUM_PAGES);

    // Sequential access timing
    uint64_t start_seq = rdtsc();
    for (int i = 0; i < NUM_ACCESSES; i++) {
        memory[i % (PAGE_SIZE * NUM_PAGES)] += 1;
    }
    uint64_t end_seq = rdtsc();

    // Random access timing
    uint64_t start_rand = rdtsc();
    for (int i = 0; i < NUM_ACCESSES; i++) {
        int random_offset = rand() % (PAGE_SIZE * NUM_PAGES);
        memory[random_offset] += 1;
    }
    uint64_t end_rand = rdtsc();

    printf("Sequential access cycles: %lu\n", end_seq - start_seq);
    printf("Random access cycles: %lu\n", end_rand - start_rand);

    free(memory);
    return 0;
}
```

To compile and run:
```bash
gcc -o tlb_test tlb_test.c
./tlb_test
```

## Further Reading

For more detailed information on virtual memory and page tables, consider exploring:
- Intel's Software Developer Manual, Volume 3: System Programming Guide
- Operating Systems: Three Easy Pieces (Chapter on Virtual Memory)
- Linux kernel documentation on memory management

## Conclusion

Multi-level page tables represent a crucial innovation in memory management, enabling efficient use of physical memory while providing process isolation and shared memory capabilities. Through the combination of hardware support (TLB, MMU) and software management (operating system page table handlers), modern systems can provide the illusion of vast, contiguous memory spaces to processes while maintaining security and performance.

The practical implementations provided here demonstrate the core concepts, though real-world systems typically use four or five levels of page tables and include additional optimizations. Understanding these fundamentals is essential for systems programmers and anyone working on performance-critical applications.
