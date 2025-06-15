---
layout: post
title: "Context Switching in Operating Systems"
date: 2024-10-04 12:00:00 -0400
categories: [Operating Systems]
tags: [Context Switching, Process Management, Operating Systems, Kernel Programming, Performance]
author: mohitmishra786
description: "Comprehensive exploration of context switching mechanisms in operating systems, covering implementation details, performance implications, and optimization techniques."
toc: true
---

## Table of Contents
1. Introduction
2. What is a Process Context?
3. Anatomy of a Context Switch
4. Hardware Support for Context Switching
5. Implementation Details
6. Performance Implications
7. Real-World Examples
8. Advanced Topics
9. Conclusion

## 1. Introduction

Context switching is one of the most fundamental operations in modern operating systems, enabling multitasking by allowing multiple processes to share a single CPU. While it might seem magical from the user perspective, under the hood it's a complex dance of hardware and software working in perfect harmony. In this article, we'll peel back the layers and examine exactly what happens when your OS performs a context switch, right down to the register level.

## 2. What is a Process Context?

Before we dive into switching contexts, let's understand what makes up a process context. At its core, a process context consists of:

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

This structure represents the minimal context that must be saved and restored during a context switch. Let's examine each component:

1. **General Purpose Registers**: Store intermediate computational results
2. **Stack Pointers**: Track the current execution stack
3. **Instruction Pointer**: Points to the next instruction to execute
4. **CPU Flags**: Store status information about the last arithmetic operation
5. **Segment Registers**: Used for memory segmentation
6. **CR3**: Contains the physical address of the page directory
7. **FPU/SSE State**: Floating-point and SIMD execution state

## 3. Anatomy of a Context Switch

A context switch can occur for several reasons:
- Timer interrupt (preemption)
- System call
- I/O request
- Inter-process communication

Here's a simplified view of the context switch handler:

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
        // ... save other registers ...
        : "=m" (ctx->rax), "=m" (ctx->rbx),
          "=m" (ctx->rcx), "=m" (ctx->rdx),
          "=m" (ctx->rsi), "=m" (ctx->rdi),
          "=m" (ctx->rbp), "=m" (ctx->rsp)
        :
        : "memory"
    );
}
```

## 4. Hardware Support for Context Switching

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

## 5. Implementation Details

Let's look at a more detailed context switch implementation:

```c
void schedule() {
    struct task_struct *prev, *next;

    // Disable interrupts during switch
    local_irq_disable();

    prev = current;
    next = pick_next_task();

    if (prev != next) {
        // Update runtime statistics
        update_task_runtime_stats(prev);

        // Switch process memory space
        switch_mm(prev->mm, next->mm);

        // Switch kernel stacks
        switch_to(prev, next);
    }

    local_irq_enable();
}

void switch_to(struct task_struct *prev, struct task_struct *next) {
    struct thread_struct *prev_thread = &prev->thread;
    struct thread_struct *next_thread = &next->thread;

    // Save FPU state if necessary
    if (task_thread_info(prev)->status & TS_USEDFPU) {
        save_fpu(prev_thread);
        task_thread_info(prev)->status &= ~TS_USEDFPU;
    }

    // Save debug registers if used
    if (unlikely(prev_thread->debugreg7)) {
        loaddebug_inactive(prev);
    }

    // Perform actual context switch
    __switch_to(prev, next);

    // Handle return path
    if (unlikely(task_thread_info(current)->flags & _TIF_WORK_CTXSW))
        __work_ctxsw();
}
```

## 6. Performance Implications

Context switching isn't free. Here's a simple benchmark tool:

```c
#define ITERATIONS 1000000

uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

void measure_context_switch() {
    pid_t pid;
    int pipe_fd[2];
    char byte = 0;
    uint64_t start, end;

    pipe(pipe_fd);
    pid = fork();

    if (pid == 0) {  // Child
        for (int i = 0; i < ITERATIONS; i++) {
            read(pipe_fd[0], &byte, 1);
            write(pipe_fd[1], &byte, 1);
        }
        exit(0);
    } else {  // Parent
        start = rdtsc();

        for (int i = 0; i < ITERATIONS; i++) {
            write(pipe_fd[1], &byte, 1);
            read(pipe_fd[0], &byte, 1);
        }

        end = rdtsc();
        printf("Average context switch time: %lu cycles\n",
               (end - start) / (ITERATIONS * 2));
    }
}
```

Common context switch costs include:

1. **Direct Costs**
   - Saving CPU registers
   - Saving FPU state
   - Loading new process state
   - Switching page tables

2. **Indirect Costs**
   - TLB flush
   - Cache pollution
   - Pipeline flush
   - Branch predictor reset

## 7. Real-World Examples

Let's look at how the Linux kernel handles context switching in practice:

```c
/*
 * This is the actual context switch function.
 * It only needs to be this big because everybody else is eating
 * up all the register space.
 * This is only called from schedule() and schedule_tail().
 */
__visible __notrace_funcgraph struct task_struct *
__switch_to(struct task_struct *prev_p, struct task_struct *next_p) {
    struct thread_struct *prev = &prev_p->thread;
    struct thread_struct *next = &next_p->thread;
    int cpu = smp_processor_id();

    // Switch kernel page table
    switch_mm_irqs_off(prev->mm, next->mm, next_p);

    // Switch kernel stack
    this_cpu_write(current_task, next_p);
    this_cpu_write(cpu_current_top_of_stack, task_top_of_stack(next_p));

    // Load TLS and update syscall entry/exit
    load_TLS(next, cpu);
    arch_update_syscall_work(next_p);

    // Restore all registers
    return __switch_to_asm(prev_p, next_p);
}
```

## 8. Advanced Topics

### Thread Context vs Process Context

Thread context switches are generally lighter:

```c
struct thread_context {
    // Subset of process context
    uint64_t rsp;                   // Stack pointer
    uint64_t rbp;                   // Base pointer
    uint64_t rip;                   // Instruction pointer
    uint64_t r12, r13, r14, r15;    // Callee-saved registers

    // Thread-local storage
    uint64_t fs_base;              // FS segment base address
    uint64_t gs_base;              // GS segment base address
};

void thread_switch(struct thread_context* old_thread,
                  struct thread_context* new_thread) {
    // Save minimal register set
    __asm__ volatile (
        "movq %%rsp, %0\n"
        "movq %%rbp, %1\n"
        "movq %%r12, %2\n"
        "movq %%r13, %3\n"
        "movq %%r14, %4\n"
        "movq %%r15, %5\n"
        : "=m" (old_thread->rsp),
          "=m" (old_thread->rbp),
          "=m" (old_thread->r12),
          "=m" (old_thread->r13),
          "=m" (old_thread->r14),
          "=m" (old_thread->r15)
        :
        : "memory"
    );

    // Switch to new thread
    __asm__ volatile (
        "movq %0, %%rsp\n"
        "movq %1, %%rbp\n"
        "movq %2, %%r12\n"
        "movq %3, %%r13\n"
        "movq %4, %%r14\n"
        "movq %5, %%r15\n"
        :
        : "m" (new_thread->rsp),
          "m" (new_thread->rbp),
          "m" (new_thread->r12),
          "m" (new_thread->r13),
          "m" (new_thread->r14),
          "m" (new_thread->r15)
        : "memory"
    );
}
```

### NUMA Considerations

On NUMA systems, context switching becomes more complex:

```c
struct numa_context {
    int current_node;
    uint64_t node_mask;
    struct page_table_entry* per_node_page_tables[MAX_NUMA_NODES];
};

void numa_aware_context_switch(struct process_context* old_context,
                             struct process_context* new_context,
                             struct numa_context* numa_ctx) {
    int target_node;

    // Determine optimal NUMA node
    target_node = find_optimal_numa_node(new_context);

    if (target_node != numa_ctx->current_node) {
        // Switch to page tables for target node
        load_cr3(numa_ctx->per_node_page_tables[target_node]);
        numa_ctx->current_node = target_node;
    }

    // Perform regular context switch
    context_switch(old_context, new_context);
}
```

## 9. Conclusion

Context switching is a fundamental OS operation that requires careful coordination between hardware and software. Understanding its low-level details is crucial for:

1. Operating system development
2. Performance optimization
3. Debugging system-level issues
4. Understanding process scheduling

Modern CPUs continue to evolve with new features to optimize context switching, but the basic principles remain the same. Whether you're developing an OS, optimizing performance, or just curious about how your computer juggles multiple processes, understanding context switching at this level provides valuable insights into system behavior.

The code examples provided here are simplified for clarity but demonstrate the key concepts involved in real-world context switching implementations. For production systems, additional considerations like security, error handling, and hardware-specific optimizations would need to be addressed.
