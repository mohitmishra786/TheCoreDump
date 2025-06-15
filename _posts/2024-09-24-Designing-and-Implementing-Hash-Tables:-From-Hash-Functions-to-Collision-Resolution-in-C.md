---
layout: post
title: "Designing and Implementing Hash Tables: From Hash Functions to Collision Resolution in C"
date: 2024-09-24 12:00:00 -0400
categories: [Data Structures]
tags: [Hash Tables, C Programming, Data Structures, Algorithms, Collision Resolution]
author: mohitmishra786
description: "A comprehensive guide to hash tables implementation in C, covering hash functions, collision resolution strategies, and performance optimization techniques."
toc: true
---

Hash tables are a fundamental data structure in computer science, offering a powerful combination of fast lookups, insertions, and deletions. In this article, we'll explore the inner workings of hash tables, implement them in C, and analyze their performance characteristics. We'll also discuss various collision resolution strategies and provide optimized code examples for real-world applications.

## 1. Introduction to Hash Tables

Hash tables, also known as hash maps, are data structures that implement an associative array abstract data type. They use a hash function to compute an index into an array of buckets or slots, from which the desired value can be found. The primary advantage of hash tables is their ability to achieve constant-time average complexity for insertions, deletions, and lookups, making them extremely efficient for large datasets.

## 2. Hash Functions

The heart of a hash table is its hash function. A good hash function should have the following properties:

1. Deterministic: The same input should always produce the same output.
2. Efficient: It should be quick to compute.
3. Uniform distribution: It should map inputs as evenly as possible over the output range.

Let's implement a simple hash function for strings in C:

```c
unsigned int hash(const char *key, int table_size) {
    unsigned int hash_value = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash_value = (hash_value * 31 + key[i]) % table_size;
    }
    return hash_value;
}
```

This hash function uses the popular "31" multiplier and performs modulo arithmetic to ensure the result fits within the table size. While simple, it provides a good distribution for most use cases.

## 3. Collision Resolution Strategies

When two different keys hash to the same index, we have a collision. There are two main strategies for handling collisions: open addressing and external chaining.

### 3.1 Open Addressing

In open addressing, all elements are stored in the hash table itself. When a collision occurs, we probe for the next available slot in the table. There are several probing techniques:

1. Linear Probing: Check the next slot sequentially.
2. Quadratic Probing: Use a quadratic function to determine the next slot.
3. Double Hashing: Use a second hash function to determine the probe sequence.

Let's implement linear probing:

```c
#define TABLE_SIZE 100
#define DELETED_NODE (Person *)(0xFFFFFFFFFFFFFFFFUL)

typedef struct {
    char name[256];
    int age;
} Person;

Person *hash_table[TABLE_SIZE];

bool insert(Person *p) {
    if (p == NULL) return false;

    int index = hash(p->name, TABLE_SIZE);
    int original_index = index;

    do {
        if (hash_table[index] == NULL || hash_table[index] == DELETED_NODE) {
            hash_table[index] = p;
            return true;
        }

        index = (index + 1) % TABLE_SIZE;
    } while (index != original_index);

    return false;  // Table is full
}
```

This implementation uses linear probing to find the next available slot. If we loop back to the original index, the table is full.

### 3.2 External Chaining

In external chaining, each bucket in the hash table contains a linked list of elements that hash to the same index. This method allows the hash table to grow beyond its initial size.

Let's implement external chaining:

```c
#define TABLE_SIZE 100

typedef struct Node {
    char *key;
    int value;
    struct Node *next;
} Node;

Node *hash_table[TABLE_SIZE] = {NULL};

void insert(const char *key, int value) {
    int index = hash(key, TABLE_SIZE);

    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = NULL;

    if (hash_table[index] == NULL) {
        hash_table[index] = new_node;
    } else {
        new_node->next = hash_table[index];
        hash_table[index] = new_node;
    }
}
```

This implementation adds new nodes to the beginning of the linked list at each index, which is efficient for insertions.

## 4. Implementing a Hash Table in C

Now that we've covered the basics, let's implement a complete hash table using external chaining:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TABLE_SIZE 100

typedef struct Node {
    char *key;
    int value;
    struct Node *next;
} Node;

Node *hash_table[TABLE_SIZE] = {NULL};

unsigned int hash(const char *key) {
    unsigned int hash_value = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash_value = (hash_value * 31 + key[i]) % TABLE_SIZE;
    }
    return hash_value;
}

void insert(const char *key, int value) {
    int index = hash(key);

    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = NULL;

    if (hash_table[index] == NULL) {
        hash_table[index] = new_node;
    } else {
        new_node->next = hash_table[index];
        hash_table[index] = new_node;
    }
}

bool search(const char *key, int *result) {
    int index = hash(key);
    Node *current = hash_table[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            *result = current->value;
            return true;
        }
        current = current->next;
    }

    return false;
}

bool delete(const char *key) {
    int index = hash(key);
    Node *current = hash_table[index];
    Node *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                hash_table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->key);
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

void print_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hash_table[i] != NULL) {
            printf("Index %d: ", i);
            Node *current = hash_table[i];
            while (current != NULL) {
                printf("(%s: %d) -> ", current->key, current->value);
                current = current->next;
            }
            printf("NULL\n");
        }
    }
}

int main() {
    insert("apple", 5);
    insert("banana", 8);
    insert("cherry", 12);
    insert("date", 7);
    insert("elderberry", 3);

    print_table();

    int result;
    if (search("banana", &result)) {
        printf("Value for 'banana': %d\n", result);
    } else {
        printf("'banana' not found\n");
    }

    if (delete("cherry")) {
        printf("'cherry' deleted successfully\n");
    } else {
        printf("'cherry' not found\n");
    }

    print_table();

    return 0;
}
```

This implementation provides a complete hash table with insert, search, and delete operations, as well as a function to print the contents of the table for debugging purposes.

## 5. Performance Analysis

The performance of hash tables depends on several factors:

1. Load factor: The ratio of the number of elements to the table size.
2. Quality of the hash function: How well it distributes keys across the table.
3. Collision resolution strategy: Open addressing vs. external chaining.

In the best-case scenario, with a good hash function and low load factor, hash tables can achieve O(1) average time complexity for insertions, deletions, and lookups. However, in the worst case (e.g., all keys hashing to the same index), the time complexity can degrade to O(n), where n is the number of elements in the table.

To maintain good performance, it's crucial to resize the hash table when the load factor exceeds a certain threshold (typically 0.7 or 0.8). Resizing involves creating a new, larger table and rehashing all existing elements.

## 6. Advanced Techniques and Optimizations

1. **Dynamic resizing**: Implement automatic resizing when the load factor exceeds a threshold.
2. **Robin Hood hashing**: An open addressing technique that reduces variance in probe sequence length.
3. **Cuckoo hashing**: Uses multiple hash functions to achieve worst-case O(1) lookups.
4. **Perfect hashing**: For static sets of keys, it's possible to achieve O(1) worst-case lookups.
5. **Bloom filters**: A space-efficient probabilistic data structure for set membership queries.

## 7. Conclusion

Hash tables are a powerful and versatile data structure that offer excellent average-case performance for many common operations. By understanding the underlying principles and implementing them correctly, you can leverage hash tables to solve a wide range of problems efficiently.

As you continue to work with hash tables, remember to consider the specific requirements of your application, such as expected data size, frequency of operations, and memory constraints. With careful tuning and the right optimizations, hash tables can be an invaluable tool in your programming toolkit.
