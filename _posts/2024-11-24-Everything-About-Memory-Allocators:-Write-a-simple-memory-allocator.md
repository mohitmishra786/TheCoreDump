---
layout: post
title: "Everything About Memory Allocators: Write a Simple Memory Allocator"
date: 2024-11-24 12:00:00 -0400
categories: [Programming & Data Structures]
tags: [Memory Allocators, Memory Management, malloc, Heap Management, System Programming, Performance Optimization]
author: mohitmishra786
description: "A comprehensive guide to memory allocators, covering allocation strategies, heap management, and implementing a custom memory allocator from scratch with performance considerations."
toc: true
---
## Table of Contents
1. [Introduction](#introduction)
2. [Memory Layout Fundamentals](#memory-layout-fundamentals)
3. [Memory Management Basics](#memory-management-basics)
4. [Building a Memory Allocator](#building-a-memory-allocator)
   - [Core Data Structures](#core-data-structures)
   - [Memory Block Headers](#memory-block-headers)
   - [Implementation Details](#implementation-details)
5. [Core Functions Implementation](#core-functions-implementation)
   - [malloc()](#malloc)
   - [free()](#free)
   - [calloc()](#calloc)
   - [realloc()](#realloc)
6. [Thread Safety Considerations](#thread-safety-considerations)
7. [Testing and Usage](#testing-and-usage)
8. [Sequence Diagrams](#sequence-diagrams)
9. [Further Reading](#further-reading)
10. [Conclusion](#conclusion)

## Introduction

Memory allocation is a fundamental aspect of systems programming that every developer should understand. This article provides an in-depth exploration of implementing a basic memory allocator in C, covering malloc(), free(), calloc(), and realloc(). While our implementation won't match the sophistication of production allocators like ptmalloc or jemalloc, it will help you understand the core concepts.

## Memory Layout Fundamentals

Before diving into implementation details, let's understand how memory is organized in a Linux process. A program's virtual address space typically consists of several segments:

1. Text Segment (Code Segment)
   - Contains executable instructions
   - Read-only
   - Shared among processes running the same program

2. Data Segment
   - Contains initialized global and static variables
   - Split into read-only and read-write sections

3. BSS (Block Started by Symbol)
   - Contains uninitialized global and static variables
   - Automatically initialized to zero by the kernel
   - Takes no actual space in the executable file

4. Heap
   - Dynamically allocated memory
   - Grows upward (toward higher addresses)
   - Managed by malloc/free
   - Starts after BSS

5. Stack
   - Automatic variables
   - Function call frames
   - Grows downward (toward lower addresses)
   - Automatically managed

The program break (brk) marks the end of the heap segment. Our memory allocator will primarily interact with this boundary using the sbrk() system call.

## Memory Management Basics

### The Program Break

The program break is a virtual address that marks the end of the process's data segment. We can manipulate it using two system calls:

1. brk(void *addr) - Sets the program break to a specific address
2. sbrk(intptr_t increment) - Adjusts the program break by a specified increment

Key points about sbrk():
- sbrk(0) returns the current program break address
- sbrk(positive_value) increases heap size
- sbrk(negative_value) decreases heap size
- Returns (void*)-1 on error

## Building a Memory Allocator

### Core Data Structures

Our allocator needs to track allocated memory blocks. We'll use a linked list approach with each block containing a header structure.

```c
typedef char ALIGN[16];

union header {
    struct {
        size_t size;        // Size of the block
        unsigned is_free;   // Block status flag
        union header *next; // Next block in list
    } s;
    ALIGN stub;            // Force 16-byte alignment
};
typedef union header header_t;

// Global variables
header_t *head = NULL, *tail = NULL;
pthread_mutex_t global_malloc_lock;
```

The header structure serves several purposes:
1. Tracks block size
2. Maintains free/allocated status
3. Links blocks together
4. Ensures proper memory alignment

### Memory Block Headers

Each allocated block needs a header to store metadata:

```c
typedef struct block_header {
    size_t size;           // Size of the block (including header)
    int is_free;          // 1 if free, 0 if allocated
    struct block_header* next;    // Pointer to next block
    struct block_header* prev;    // Pointer to previous block
} block_header_t;
```

The header contains:
- **size**: Total size including the header
- **is_free**: Flag indicating if the block is available
- **next/prev**: Pointers for maintaining a doubly-linked list

#### Implementation Details {#implementation-details}

The memory allocator implementation involves several key design decisions and optimizations:

### Block Alignment
All memory blocks must be aligned to word boundaries for optimal performance:

```c
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
```

### Block Splitting and Coalescing
When allocating memory, we may need to split large blocks:

```c
static block_header_t* split_block(block_header_t* block, size_t size) {
    if (block->size >= size + sizeof(block_header_t) + ALIGNMENT) {
        block_header_t* new_block = (block_header_t*)((char*)block + size);
        new_block->size = block->size - size;
        new_block->is_free = 1;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        block->size = size;
        
        return new_block;
    }
    return NULL;
}
```

### Memory Coalescing
When freeing blocks, adjacent free blocks should be merged:

```c
static void coalesce_blocks(block_header_t* block) {
    // Merge with next block if free
    if (block->next && block->next->is_free) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    // Merge with previous block if free
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}
```

## Core Functions Implementation

### malloc()

The malloc() implementation follows these steps:

1. Check for valid size request
2. Look for existing free block of sufficient size
3. If found, mark as used and return
4. If not found, request new memory from OS
5. Setup header and return pointer

[![](https://mermaid.ink/img/pako:eNq1k8tqwzAQRX9l0KqF5Ae8CBRKV00MNd0UQ5hIY1tED1cP2hDy75XsNHUeZFct7LHm4HvvWN4zbgWxgnn6jGQ4PUtsHeraQFo9uiC57NEEeOp7QJ9vSnIM0pprZolKWZ4xPVQPj9fMiyPKREth3aR6vUng9hZZVpkre3JJzrRQ7Xygo7PxmszMF4tRtYC3HMEH0KSt243E2JtAr0kMdAz0fQFkXwVUhI53kI3BYMxP9VAd_Q8taGw0YuzklTvzMz8hOjOyf9SVoyW6bQ4aPYk72LtRF9YnWOLSLE6S0wmQ8gQrO4l0Q6SsCvAbd_oMeZXVNMyKvs5ee9NjleR76AgFuXtReoGBQEkf_iOwEWzGNDmNUqSTvc_bNQsdaapZkUpBDUYValabQ0IxBlvtDGdFcJFmzNnYdqxoME1uxuJg9vhbnHbTAf2w9vf58AO6fw4i?type=png)](https://mermaid.live/edit#pako:eNq1k8tqwzAQRX9l0KqF5Ae8CBRKV00MNd0UQ5hIY1tED1cP2hDy75XsNHUeZFct7LHm4HvvWN4zbgWxgnn6jGQ4PUtsHeraQFo9uiC57NEEeOp7QJ9vSnIM0pprZolKWZ4xPVQPj9fMiyPKREth3aR6vUng9hZZVpkre3JJzrRQ7Xygo7PxmszMF4tRtYC3HMEH0KSt243E2JtAr0kMdAz0fQFkXwVUhI53kI3BYMxP9VAd_Q8taGw0YuzklTvzMz8hOjOyf9SVoyW6bQ4aPYk72LtRF9YnWOLSLE6S0wmQ8gQrO4l0Q6SsCvAbd_oMeZXVNMyKvs5ee9NjleR76AgFuXtReoGBQEkf_iOwEWzGNDmNUqSTvc_bNQsdaapZkUpBDUYValabQ0IxBlvtDGdFcJFmzNnYdqxoME1uxuJg9vhbnHbTAf2w9vf58AO6fw4i)

Here's the implementation:

```c
void *malloc(size_t size) {
    if (!size) return NULL;
    
    pthread_mutex_lock(&global_malloc_lock);
    
    // Try to find a free block
    header_t *header = get_free_block(size);
    if (header) {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }
    
    // Need new block from OS
    size_t total_size = sizeof(header_t) + size;
    void *block = sbrk(total_size);
    if (block == (void*)-1) {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    
    // Initialize new block
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    
    // Update list pointers
    if (!head) head = header;
    if (tail) tail->s.next = header;
    tail = header;
    
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1);
}
```

### free()
[![](https://mermaid.ink/img/pako:eNqNUstqwzAQ_BWhUwLOD_gQKJSWQoohppfiy8ZeJ8J6dbWCpiH_Xql2Slq3pTpoxc4wox3pJFvXoSxlwJeItsVbBXsC01iRlgdi1SoPlsWN9wJCLlq1wMrZOeeOEDOpT3WxnONVndHKIyUBuxf1MTBOXuOe5FfrddYpRzWDxtFxBHPjE924dhAmMr7-AN4jiwNCh_QFrOoR8uTykGJHCMO1PWgWGwgsdjrJj72Z-JPvgFFoFXjGyA5hR8NiFdQbLn9TsPrb7VEHFA-WkSzoP90fgYZLxv-Xt91VEImYci7FFjmSlYU0SAZUl77BKdMayQc02MgyHTvsIWpuZGPPiQqRXX20rSyZIhaSXNwfZNlDGqCQ8SOa6Q9dKOnpn50zE-n8DnbjyEI?type=png)](https://mermaid.live/edit#pako:eNqNUstqwzAQ_BWhUwLOD_gQKJSWQoohppfiy8ZeJ8J6dbWCpiH_Xql2Slq3pTpoxc4wox3pJFvXoSxlwJeItsVbBXsC01iRlgdi1SoPlsWN9wJCLlq1wMrZOeeOEDOpT3WxnONVndHKIyUBuxf1MTBOXuOe5FfrddYpRzWDxtFxBHPjE924dhAmMr7-AN4jiwNCh_QFrOoR8uTykGJHCMO1PWgWGwgsdjrJj72Z-JPvgFFoFXjGyA5hR8NiFdQbLn9TsPrb7VEHFA-WkSzoP90fgYZLxv-Xt91VEImYci7FFjmSlYU0SAZUl77BKdMayQc02MgyHTvsIWpuZGPPiQqRXX20rSyZIhaSXNwfZNlDGqCQ8SOa6Q9dKOnpn50zE-n8DnbjyEI)

The free() implementation handles memory deallocation:

```c
void free(void *block) {
    if (!block) return;
    
    pthread_mutex_lock(&global_malloc_lock);
    
    header_t *header = (header_t*)block - 1;
    void *programbreak = sbrk(0);
    
    // Check if block is at the end of heap
    if ((char*)block + header->s.size == programbreak) {
        // Remove from list
        if (head == tail) {
            head = tail = NULL;
        } else {
            header_t *tmp = head;
            while (tmp) {
                if (tmp->s.next == tail) {
                    tmp->s.next = NULL;
                    tail = tmp;
                    break;
                }
                tmp = tmp->s.next;
            }
        }
        
        // Return memory to OS
        sbrk(0 - sizeof(header_t) - header->s.size);
    } else {
        // Mark block as free
        header->s.is_free = 1;
    }
    
    pthread_mutex_unlock(&global_malloc_lock);
}
```

### calloc()

calloc() allocates memory and initializes it to zero:

```c
void *calloc(size_t num, size_t nsize) {
    size_t size;
    void *block;
    
    if (!num || !nsize) return NULL;
    
    size = num * nsize;
    // Check for multiplication overflow
    if (nsize != size / num) return NULL;
    
    block = malloc(size);
    if (!block) return NULL;
    
    // Zero out the memory
    memset(block, 0, size);
    return block;
}
```

### realloc()
[![](https://mermaid.ink/img/pako:eNqVk8lqwzAQhl9F6NRAAj0bGihdbsmhuRVDmEpjR1SLq4U0DXn3jmJncxZaHSyh-Zj55_dozYWTyAse8CuhFfisoPZgSstoNeCjEqoBG9lj0zAIedNKQFTOnjNvCFo7kTnfHu8G59RkD5mrzKtHzERF-y7efknAaDzuKhVUMqgfZAaN86tjDnRkU8c-iPpkzrOMPdy3sby6DJRr0qXq1GRwcODa6Oi45hSXJwWP0xFHCrOumLw9wVAHZC_fKkRl61bYJTn7Mk8LJOkieY_kSJZ1wHN3s9x4SFVFphFxCN7QE8Bgv_RW1xRRMg2-Rt-P33TL4nLec-wfrl3r3DUrJiHCNTKPR9EOh9NyvhU8-JMB9oIKtLL7Q1byITfoDShJj2Kdr0seF2iw5AUdJVaQdCx5aTeEQoputrKCF9EnHHLvUr3gRQVk6ZCnhnrYvagdQuP97pzpoM0vlvkbUw?type=png)](https://mermaid.live/edit#pako:eNqVk8lqwzAQhl9F6NRAAj0bGihdbsmhuRVDmEpjR1SLq4U0DXn3jmJncxZaHSyh-Zj55_dozYWTyAse8CuhFfisoPZgSstoNeCjEqoBG9lj0zAIedNKQFTOnjNvCFo7kTnfHu8G59RkD5mrzKtHzERF-y7efknAaDzuKhVUMqgfZAaN86tjDnRkU8c-iPpkzrOMPdy3sby6DJRr0qXq1GRwcODa6Oi45hSXJwWP0xFHCrOumLw9wVAHZC_fKkRl61bYJTn7Mk8LJOkieY_kSJZ1wHN3s9x4SFVFphFxCN7QE8Bgv_RW1xRRMg2-Rt-P33TL4nLec-wfrl3r3DUrJiHCNTKPR9EOh9NyvhU8-JMB9oIKtLL7Q1byITfoDShJj2Kdr0seF2iw5AUdJVaQdCx5aTeEQoputrKCF9EnHHLvUr3gRQVk6ZCnhnrYvagdQuP97pzpoM0vlvkbUw)

realloc() changes the size of an existing allocation:

```c
void *realloc(void *block, size_t size) {
    if (!block || !size) return malloc(size);
    
    header_t *header = (header_t*)block - 1;
    if (header->s.size >= size) return block;
    
    void *ret = malloc(size);
    if (ret) {
        memcpy(ret, block, header->s.size);
        free(block);
    }
    return ret;
}
```

## Thread Safety Considerations

Our implementation uses a global mutex to ensure thread safety:

```c
pthread_mutex_t global_malloc_lock;

// Initialize in main or constructor
pthread_mutex_init(&global_malloc_lock, NULL);
```

However, this approach has limitations:
1. Single lock creates contention
2. sbrk() isn't thread-safe
3. No protection against foreign sbrk() calls

## Testing and Usage

Let's test our allocator with a comprehensive example:

```c
int main() {
    printf("Testing custom memory allocator\n");
    
    // Test basic allocation
    void* ptr1 = my_malloc(100);
    printf("Allocated 100 bytes at %p\n", ptr1);
    
    void* ptr2 = my_malloc(200);
    printf("Allocated 200 bytes at %p\n", ptr2);
    
    // Test freeing
    my_free(ptr1);
    printf("Freed first allocation\n");
    
    // Test allocation after free (should reuse space)
    void* ptr3 = my_malloc(50);
    printf("Allocated 50 bytes at %p\n", ptr3);
    
    // Test calloc
    void* ptr4 = my_calloc(10, sizeof(int));
    printf("Allocated zeroed array at %p\n", ptr4);
    
    // Test realloc
    ptr4 = my_realloc(ptr4, 20 * sizeof(int));
    printf("Reallocated array to %p\n", ptr4);
    
    // Cleanup
    my_free(ptr2);
    my_free(ptr3);
    my_free(ptr4);
    
    printf("All tests completed successfully\n");
    return 0;
}
```

## Sequence Diagrams {#sequence-diagrams}

Understanding the flow of memory allocation operations is crucial. Here are sequence diagrams showing the interaction between different components:

### malloc() Sequence
```mermaid
sequenceDiagram
    participant App as Application
    participant Malloc as my_malloc()
    participant Heap as Heap Manager
    participant OS as Operating System
    
    App->>Malloc: Request memory (size)
    Malloc->>Heap: Find suitable block
    alt Block found
        Heap->>Malloc: Return existing block
        Malloc->>Heap: Split block if needed
    else No block found
        Malloc->>OS: Request more memory (sbrk)
        OS->>Malloc: Extend heap
        Malloc->>Heap: Create new block
    end
    Malloc->>App: Return pointer to allocated memory
```

### free() Sequence
```mermaid
sequenceDiagram
    participant App as Application
    participant Free as my_free()
    participant Heap as Heap Manager
    
    App->>Free: Free memory (pointer)
    Free->>Heap: Mark block as free
    Free->>Heap: Check adjacent blocks
    alt Adjacent blocks are free
        Heap->>Free: Coalesce blocks
        Free->>Heap: Update block list
    end
    Free->>App: Return (void)
```

### Memory Layout Evolution
```mermaid
graph TD
    A[Initial Heap] --> B[First malloc(100)]
    B --> C[Second malloc(200)]
    C --> D[free(first block)]
    D --> E[malloc(50) - reuses space]
    
    subgraph "Heap States"
        A1["|------ Free Block ------|"]
        B1["|Used(100)|-- Free --|"]
        C1["|Used(100)|Used(200)|Free|"]
        D1["|Free(100)|Used(200)|Free|"]
        E1["|Used(50)|Free|Used(200)|Free|"]
    end
```

This visualization shows how the heap evolves as allocations and deallocations occur, demonstrating block splitting, coalescing, and reuse.

## Further Reading

* The Linux Programming Interface by Michael Kerrisk
* Advanced Programming in the UNIX Environment by W. Richard Stevens
* Understanding the Linux Virtual Memory Manager by Mel Gorman
* Doug Lea's Memory Allocator Documentation
* jemalloc Documentation and Source Code

## Conclusion
This implementation demonstrates the fundamental concepts of memory allocation while keeping things simple enough to understand. Production allocators add many optimizations:

* Multiple allocation strategies
* Memory pooling
* Thread-local caches
* Sophisticated fragmentation handling
* Memory mapping for large allocations

### Key takeaways:

* Memory allocation involves both userspace and kernel space interaction
* Thread safety requires careful consideration
* Memory layout understanding is crucial
* Simple implementations help learn fundamentals
* Production allocators are much more complex

Remember that this implementation is for educational purposes. Production systems should use established allocators that have been thoroughly tested and optimized.
