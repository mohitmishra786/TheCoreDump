---
layout: post
title: "Conquering the Unknown: A Deep Dive into Freestanding C++"
date: 2024-10-07 12:00:00 -0400
categories: [C++ Programming]
tags: [Freestanding C++, Embedded Programming, System Programming, C++ Standards, Low-Level Programming]
author: mohitmishra786
description: "Comprehensive exploration of freestanding C++ programming, covering embedded systems development and low-level programming techniques."
toc: true
---
The world of software development often feels divided. On one hand, there's the familiar comfort of high-level languages like C++, with their rich libraries and the comforting embrace of an operating system. On the other, lies the daunting realm of bare metal, where every byte is precious, and the OS is a distant luxury. This is the challenging, yet exhilarating, world of **freestanding C++**.

This comprehensive guide is your passport to this world. We'll journey together from the foundational principles of freestanding environments to the intricacies of crafting your own runtime support, and even delve into the art of injecting C++ into a running program. 

This is not a casual read. It's a deep dive, a spelunking expedition into the very core of how C++ operates. Be prepared to get your hands dirty with compiler flags, assembly code, and the arcane rituals of linker scripts. But fear not, for the rewards are immense: the ability to wield the power of C++ in any environment, no matter how resource-constrained or unconventional.

### Part 1: Embracing the Void - Navigating the Freestanding Landscape

Our journey begins with understanding the very essence of freestanding environments.  Imagine a world stripped bare, devoid of the conveniences we take for granted. No operating system to act as an intermediary, no standard library to provide pre-built solutions. This is the reality of freestanding development.

**1.1. The Pillars of Hosted C++:**

In the comfortable realm of hosted C++, the operating system acts as a benevolent guardian, providing a rich set of services:

* **Standard Library (The C++ Comfort Zone):**  The C++ standard library, with its versatile containers like `std::vector` and algorithms like `std::sort`, is often taken for granted. But in a freestanding environment, each of these components would need to be meticulously crafted by hand.
* **Dynamic Memory Allocation (The Illusion of Abundance):** `new` and `delete` are the magic words that allow our programs to request and release memory as needed. But beneath this seemingly simple abstraction lies a complex dance between the program and the operating system's memory manager.
* **System Calls (Whispering to the Machine):**  System calls are the bridge between our high-level code and the low-level operations of the operating system. Want to write to a file? Open a network connection? Create a new thread? It all boils down to system calls.

**1.2.  The Freestanding Reality: A Wilderness of Bits**

Freestanding environments present a stark contrast:

* **No Safety Net of Libraries:** The standard C++ library, that familiar safety net, is gone. You'll need to craft your own data structures, algorithms, and I/O routines.
* **Memory Management as a Way of Life:**  Dynamic memory allocation, with its implicit reliance on the OS, becomes a luxury you can ill afford. Instead, you'll carefully plan your memory usage, often relying on fixed-size buffers and static allocation.
* **Speaking the Language of Hardware:**  Interacting with the outside world requires a direct line to the hardware. This might involve writing to memory-mapped registers, crafting intricate device drivers, or relying on low-level APIs provided by the target environment.

### Part 2: Laying the Foundation - Constructing Your Freestanding Toolchain

Our exploration of the freestanding world requires the right set of tools. Fortunately, the open-source community has gifted us with a powerful and versatile toolkit: The LLVM Compiler Infrastructure. 

**2.1.  LLVM: The Architect's Toolkit**

LLVM is more than just a compiler; it's a modular and extensible framework for building compilers, analyzers, and optimizers.  For our freestanding C++ journey, we'll focus on four key components:

* **Clang (The C++ Whisperer):** Clang is LLVM's C++ frontend, meticulously designed to parse and understand the intricacies of the C++ language. It's known for its clear and concise error messages, making it a developer's best friend, especially when venturing into the unforgiving territory of freestanding environments.
* **libc++ (The Standard Bearer):**  libc++ is LLVM's implementation of the C++ standard library. While we won't be using it directly in our freestanding endeavors, understanding its structure and design can be invaluable when crafting our own runtime support.
* **libc++abi (The Unsung Hero):**  libc++abi handles the low-level details of the C++ ABI (Application Binary Interface), including exception handling, type information (RTTI), and guard variables. These are critical components that ensure C++ code behaves as expected, even in the absence of an operating system.
* **lld (The Master Assembler):**  lld is LLVM's linker, responsible for combining object files and libraries into a final executable.  Its speed and flexibility make it an ideal choice for freestanding development, especially when dealing with custom linker scripts or unusual target platforms.

#### 2.2. Speaking the Compiler's Language: Mastering Freestanding Flags

Compilers, like any skilled artisan, require precise instructions to produce their finest work. In the world of freestanding C++, compiler flags are our way of communicating our intentions and shaping the final executable.

**Table 1: Essential Freestanding C++ Compiler Flags**

| Flag            | Purpose                                                                                               |
|-----------------|-------------------------------------------------------------------------------------------------------|
| `-ffreestanding` | Informs Clang that it's operating in a freestanding environment, disabling assumptions about the standard library and runtime environment.                      |
| `-target <target>` | Specifies the target architecture for which we're compiling our code. This could be a specific processor architecture, like `x86_64-elf`, `arm-none-eabi`, or even a custom target.    |
| `-fno-exceptions` | Disables C++ exception handling. While powerful, exceptions often rely on runtime support that might not be present in a freestanding environment.          |
| `-fno-rtti`     | Disables runtime type information (RTTI). RTTI allows programs to query the type of an object at runtime, but it incurs a cost in both code size and complexity.                  |
| `-nostdlib`     |  Instructs the compiler not to link against the standard C library (libc) or its associated startup files.  This is crucial for ensuring our code has no external dependencies.        |
| `-static`        | Forces the linker to statically link all libraries, resulting in a single, self-contained executable. This is essential for freestanding environments where dynamic linking is not available.                       |

**Example: Compiling for a 64-bit x86 Target**

```bash
clang++ -ffreestanding -target x86_64-elf -fno-exceptions -fno-rtti -nostdlib -static my_freestanding_program.cpp -o my_freestanding_program
```

This command compiles `my_freestanding_program.cpp` for a freestanding environment, targeting a 64-bit x86 architecture. The resulting executable, `my_freestanding_program`, will be statically linked and have no dependencies on external libraries or the operating system.

#### 2.3. The Linker's Dance: Choreographing Memory with Custom Scripts

The linker is the unsung hero of the compilation process, taking the pieces assembled by the compiler and weaving them into a coherent whole. In a freestanding environment, its role becomes even more critical, as we often need to precisely control the memory layout of our program.

**Example: A Simple Linker Script**

```bash
// linker script
ENTRY(_start)

SECTIONS
{
  .text 0x1000 : {
    *(.text*)
  }

  .data : {
    *(.data*)
  }

  .bss : {
    *(.bss*)
    . = ALIGN(4096);
  }
}
```

Let's break this down:

* `ENTRY(_start)`: This directive tells the linker that the entry point of our program is the `_start` function, not the usual `main` function.
* `SECTIONS`: This keyword introduces the section definitions, where we specify how different parts of our program are mapped into memory.
* `.text 0x1000`: This defines the `.text` section, which typically contains the program code. We're placing it at address `0x1000`.
* `*(.text*)`: This wildcard pattern instructs the linker to place all object file sections matching the pattern `.text*` into this section.
* `.data`: This defines the `.data` section, typically containing initialized global variables.
* `.bss`: This defines the `.bss` section, typically containing uninitialized global variables.
* `. = ALIGN(4096);`: This directive aligns the next section to a 4096-byte boundary. This is often required by certain hardware or memory management schemes.

#### 2.4.  CMake: Orchestrating the Build Process

As our freestanding projects grow in complexity, manually managing compiler and linker flags can quickly become unwieldy. CMake, a powerful cross-platform build system, comes to the rescue, providing a more manageable and maintainable way to define our build process.

**Example: A Basic CMake Configuration for Freestanding C++**

```cmake
cmake_minimum_required(VERSION 3.10)

project(FreestandingProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -target x86_64-elf -fno-exceptions -fno-rtti -nostdlib -static")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T linker_script.ld")

add_executable(my_freestanding_program my_freestanding_program.cpp)
```

In this CMake configuration:

* We define a project named `FreestandingProject`.
* We set the C++ standard to C++17 using `set(CMAKE_CXX_STANDARD 17)`.
* We append our freestanding compiler flags to the default C++ flags using `set(CMAKE_CXX_FLAGS ...)`.
* We specify our custom linker script using `set(CMAKE_EXE_LINKER_FLAGS ...)`.
* Finally, we define an executable target named `my_freestanding_program` built from `my_freestanding_program.cpp`.

### Part 3:  Breathing Life into the Machine - Crafting Your Own Runtime

With our compilation and linking process configured, we turn our attention to a fundamental challenge: breathing life into our freestanding C++ programs. Without the comforting presence of an operating system, many tasks we take for granted must be handled explicitly.

#### 3.1. The C Runtime: A Behind-the-Scenes Orchestrator

In the hosted world, the C runtime library (libc) silently performs essential tasks before our `main` function ever gets a chance to run. These tasks are so ingrained in our mental model of program execution that we often forget they're even happening.

Here are just a few of the responsibilities handled by the C runtime:

* **Setting the Stage: Stack Initialization:** Before our program can even think about executing instructions, the stack pointer needs to be set up, carving out space for local variables, function arguments, and return addresses.  
* **The Constructor's Call: Global Initialization:** C++ introduces the concept of constructors, special functions that ensure objects are properly initialized before they're used. Global constructors, as their name suggests, are invoked before the `main` function to initialize global and static objects.  
* **A Graceful Exit: Program Termination:** When our program finishes execution, it's the C runtime's responsibility to clean up after itself. This includes invoking global destructors, flushing output buffers, and ultimately returning control to the operating system.

**Example: A Minimal Freestanding C Runtime Entry Point**

```c
void _start() {
  // Initialize global constructors
  extern void (*__init_array_start[])();
  extern void (*__init_array_end[])();

  for (void (**constructor)() = __init_array_start; constructor < __init_array_end; ++constructor) {
    (*constructor)();
  }

  // Call the main function
  int result = main();

  // Initialize global destructors
  extern void (*__fini_array_start[])();
  extern void (*__fini_array_end[])();

  for (void (**destructor)() = __fini_array_end - 1; destructor >= __fini_array_start; --destructor) {
    (*destructor)();
  }

  // Exit the program
  _exit(result);
}

void _exit(int status) {
  // Assembly code for exiting the program
  #if defined(__x86_64__)
    asm("movq $60, %rax\n" // System call number for exit
        "syscall"); 
  #elif defined(__i386__)
    asm("movl $1, %eax\n"
        "xorl %ebx, %ebx\n"
        "int $0x80");
  #else
    #error Unsupported architecture
  #endif 
}
```

#### 3.2.  Exceptions: Taming the Chaos (Or Choosing Not To)

Exceptions are a powerful mechanism for handling exceptional conditions in our programs. But in the unpredictable landscape of freestanding environments, exceptions introduce a new set of challenges:

* **Unwinding the Stack:** When an exception is thrown, the C++ runtime needs a way to unwind the stack, invoking destructors for objects along the way. This requires access to stack unwinding information, which might not be readily available in a freestanding environment.
* **Finding the Handler:**  Once the stack is unwound, the runtime needs to find a suitable exception handler. This involves searching through a table of exception handlers associated with each function in the call stack.  

**Example: Implementing a Simple Exception Handler**

```c++
[[noreturn]] void __cxa_pure_virtual() {
  // This function is called when a pure virtual function is invoked.
  // In a freestanding environment, we typically want to halt execution.
  while (true); 
}
```

This code defines a function named `__cxa_pure_virtual`. This function is part of the C++ ABI and is called when a program attempts to invoke a pure virtual function. In a hosted environment, this usually means the program is in an invalid state and needs to terminate. In this freestanding example, we enter an infinite loop to halt execution.

### Part 4: A Bridge to the Outside World - Implementing System Calls

System calls are the language we use to communicate with the operating system.  But in the absence of an OS, we need to become fluent in the language of hardware, directly interacting with devices to perform essential tasks like input/output.

#### 4.1. The Anatomy of a System Call

A system call typically involves the following steps:

1. **Packing the Arguments:**  The system call number and its arguments are carefully placed in specific registers or on the stack.
2. **Issuing the Trap:** A special instruction, often called a "trap" or "syscall" instruction, is executed, transferring control from the program to the operating system.
3. **Handling the Request:** The operating system identifies the requested system call and dispatches it to the appropriate handler.
4. **Returning Control:** Once the system call completes, the operating system returns control to the program, placing the result (if any) in a designated register or on the stack.

#### 4.2. Crafting Your Own: A Minimal `write` System Call

Let's consider a simple example: implementing a basic `write` system call that allows us to send characters to a serial port.

**Example: A Freestanding `write` Implementation for Serial Output**

```c
#define SERIAL_PORT_BASE 0x3F8  // Replace with the actual base address

int write(int fd, const void *buf, size_t count) {
  if (fd != 1) { // We only support stdout (fd=1) for now
    return -1;
  }

  const char *char_buf = static_cast<const char*>(buf);
  for (size_t i = 0; i < count; ++i) {
    while (!(inb(SERIAL_PORT_BASE + 5) & 0x20)); // Wait until transmit buffer is empty
    outb(SERIAL_PORT_BASE, char_buf[i]); // Send the character
  }

  return count; 
}

// Inline assembly functions for interacting with I/O ports
inline void outb(uint16_t port, uint8_t value) {
  asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
  uint8_t value;
  asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}
```

**Explanation:**

1. **Serial Port Access:** This code assumes a standard PC-compatible serial port. The base address `0x3F8` should be replaced with the actual base address of the serial port you're targeting.

2. **File Descriptor Handling:**  For simplicity, this implementation only handles output to the standard output stream (stdout), which is typically associated with file descriptor 1.

3. **Character Transmission:** The code iterates through the provided buffer, sending each character to the serial port. Before sending each character, it checks the serial port's status register to ensure the transmit buffer is empty. 

4. **Inline Assembly:** The `inb` and `outb` functions use inline assembly to interact with I/O ports. The specific instructions used (`inb` and `outb`) are specific to x86 architectures.

### Part 5:  Venturing into the Real World - UEFI and Shellcode

With a solid understanding of the core principles of freestanding C++, let's explore how these techniques can be applied in two very different, yet equally compelling, real-world scenarios: UEFI application development and shellcode injection.

#### 5.1. UEFI: Breathing C++ into the Firmware

UEFI (Unified Extensible Firmware Interface) is the modern successor to the venerable BIOS, responsible for initializing hardware and loading operating systems during the boot process.  Writing UEFI applications in C++ provides several advantages, including improved code organization, type safety, and the ability to leverage existing C++ libraries.

**Example: A Simple UEFI Application Displaying "Hello, UEFI!"**

```c++
#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
UefiMain (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE  *SystemTable)
{
  // Initialize UEFI environment and services
  InitializeLib(ImageHandle, SystemTable);

  // Print "Hello, UEFI!" to the console
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello, UEFI!\n");

  // Indicate successful execution
  return EFI_SUCCESS;
}
```

**Explanation:**

1. **UEFI Headers:** The `efi.h` and `efilib.h` headers provide the necessary definitions and functions for interacting with the UEFI environment.

2. **Entry Point:** The `UefiMain` function is the entry point for UEFI applications, analogous to the `main` function in traditional C++ programs.

3. **UEFI System Table:** The `SystemTable` pointer provides access to various UEFI services, including console output, memory allocation, and boot services.

4. **Console Output:** The `OutputString` function is used to print a string to the console. 

**Building and Running the UEFI Application:**

1. **EDK II Setup:** Download and set up the appropriate EDK II (EFI Development Kit II) environment for your system.

2. **Project Configuration:** Create a new EDK II project and configure it to compile and link your C++ code.

3. **Build:** Build the project to generate a UEFI application image.

4. **Boot:** Create a bootable USB drive or use a UEFI-compatible virtual machine to boot the application image.

#### 5.2. Shellcode Injection: The Art of Stealthy C++

Shellcode injection is a powerful technique often used in security research and exploitation. It involves injecting and executing arbitrary code within the memory space of a running process. While traditionally written in assembly language, injecting and executing C++ code as shellcode opens up new possibilities.

**Example: Injecting and Executing a Simple C++ Shellcode**

```c++
#include <windows.h>

int main() {
  // Message box shellcode (for demonstration purposes)
  unsigned char shellcode[] = 
    "\x31\xc0\x50\x68\x65\x6c\x6c\x6f"  // xor eax, eax; push eax; push "olle"
    "\x68\x20\x57\x6f\x72\x68\x48\x65"  // push "World"; push "eH ";
    "\x6c\x6c\x89\xe6\x56\x50\x68\x4d"  // mov esi, esp; push esi; push 'M'
    "\x65\x73\x73\x68\x61\x67\x65\x42"  // push "essageB"; push "ega"
    "\x68\x72\x20\x42\x6f\x68\x6f\x78"  // push "ox "; push "xoo"
    "\x20\x2d\x68\x30\x2e\x30\x20\x68"  // push " - 0.0"; push "0 "
    "\x53\x68\x65\x6c\x6c\x43\x68\x6f"  // push "CShell"; push "oh "
    "\x64\x65\x54\xff\xd6\x83\xc4\x10"  // push "TCode"; call esi; add esp, 16
    "\x50\x6a\x00\x6a\x00\xff\x35\x08"  // push eax; push 0; push 0; push [esp+8]
    "\x11\x40\x00\xff\xd5";              // call [eax+11h]; call ebp

  // Allocate memory for the shellcode
  void *exec = VirtualAlloc(0, sizeof(shellcode), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (exec == NULL) {
    return 1;
  }

  // Copy the shellcode into the allocated memory
  memcpy(exec, shellcode, sizeof(shellcode));

  // Create a thread to execute the shellcode
  HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)exec, NULL, 0, NULL);
  if (thread == NULL) {
    return 1;
  }

  // Wait for the thread to finish
  WaitForSingleObject(thread, INFINITE);

  return 0;
}
```

**Explanation:**

1. **Shellcode Array:** The `shellcode` array contains the actual machine code that will be injected and executed. In this example, the shellcode displays a simple message box.

2. **Memory Allocation:** The `VirtualAlloc` function is used to allocate a block of memory with execute, read, and write permissions. This is necessary because we'll be injecting and executing code.

3. **Shellcode Copying:** The `memcpy` function copies the shellcode from the `shellcode` array into the allocated memory.

4. **Thread Creation:** The `CreateThread` function creates a new thread that starts executing at the address of the injected shellcode.

**Disclaimer:** Shellcode injection is a powerful technique that can be used for both legitimate and malicious purposes. It's essential to understand the ethical and legal implications before experimenting with these techniques.

### Part 6:  The Assembly Connection: Peering Through the Looking Glass

To truly master the art of freestanding C++, we must venture beyond the comfort of high-level abstractions and peer into the world of assembly language.  Assembly language provides a low-level view of how our C++ code is translated into the instructions that the processor understands.

#### 6.1. The C++ to Assembly Pipeline

Most C++ compilers provide an option to generate assembly code instead of directly producing machine code. This allows us to examine the compiler's handiwork and gain a deeper understanding of how our code is transformed into executable instructions.

**Example: Generating Assembly Code with Clang**

```bash
clang++ -S -o my_program.s my_program.cpp
```

This command instructs Clang to compile `my_program.cpp` and generate assembly code, saving it to a file named `my_program.s`.

#### 6.2. Deciphering the Assembly Language

Assembly language is specific to the target processor architecture. Let's consider a simple C++ function and its corresponding assembly code for the x86-64 architecture:

**C++ Code:**

```c++
int multiply(int a, int b) {
  return a * b;
}
```

**Assembly Code (x86-64):**

```assembly
multiply:
  pushq %rbp         # Save the base pointer
  movq %rsp, %rbp     # Set up the stack frame
  movl %edi, -4(%rbp)  # Store the first argument (a)
  movl %esi, -8(%rbp)  # Store the second argument (b)
  movl -4(%rbp), %eax # Load 'a' into the accumulator register
  imull -8(%rbp), %eax # Multiply 'a' by 'b', storing the result in 'eax'
  popq %rbp         # Restore the base pointer
  retq               # Return from the function
```

**Explanation:**

1. **Function Prologue:** The first two instructions (`pushq %rbp` and `movq %rsp, %rbp`) set up the function's stack frame, reserving space for local variables and saving the caller's base pointer.

2. **Argument Passing:** On x86-64, the first two integer arguments are passed in the `rdi` and `rsi` registers, respectively.  The code stores these arguments on the stack.

3. **Multiplication:** The `imull` instruction performs a signed multiplication, multiplying the value in the `eax` register (which holds the value of `a`) by the value at the memory address `-8(%rbp)` (which holds the value of `b`).  The result is stored in the `eax` register.

4. **Function Epilogue:** The `popq %rbp` instruction restores the caller's base pointer, and the `retq` instruction returns control to the caller.

#### 6.3. Why Speak Assembly?

Understanding assembly language is crucial for:

* **Debugging:** When things go wrong in a freestanding environment, you often don't have the luxury of debuggers or stack traces. Being able to examine the generated assembly code can be invaluable for tracking down the root cause of a problem.

* **Optimization:**  Hand-optimizing critical sections of code for performance often requires a deep understanding of the target architecture's instruction set.

* **Interfacing with Hardware:** Interacting with hardware often requires understanding and manipulating data at the bit level.  Assembly language provides the fine-grained control necessary for these tasks.

### Part 7:  A Visual Guide - Mapping the Freestanding C++ Journey

Visual representations can be invaluable for understanding complex processes. Graphviz, a graph visualization software, can help us map out the entire journey of building and executing freestanding C++ code.

![Visual](https://github.com/user-attachments/assets/f7186323-3605-4a6b-aeed-bf7640409662)

### Conclusion: Embracing the Power (and Responsibility) of Freestanding C++

Freestanding C++ is not for the faint of heart. It demands a deep understanding of the language, the compiler toolchain, and often, the intricate details of the target hardware. But the rewards are significant. By shedding the reliance on an operating system, we unlock the ability to craft highly efficient, tailored solutions for a vast array of applications, from embedded systems and firmware to operating system kernels and beyond.

This guide has equipped you with the knowledge and tools to embark on your own freestanding C++ adventures. Embrace the challenge, experiment fearlessly, and never stop exploring the limitless possibilities that lie at the intersection of high-level languages and low-level control. 
