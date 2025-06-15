---
layout: post
title: "Mastering All Design Patterns in C"
date: 2024-10-07 12:00:00 -0400
categories: [Software Design]
tags: [Design Patterns, C Programming, Software Architecture, Object-Oriented Design, Programming Patterns]
author: mohitmishra786
description: "Comprehensive guide to implementing design patterns in C programming, covering creational, structural, and behavioral patterns with practical examples."
toc: true
---

Design patterns are essential tools in a programmer's toolkit, offering tried-and-true solutions to common software design problems. In this blog post, we'll explore several key design patterns, their applications, and provide concrete implementations in C. We'll also delve into the assembly-level implications of these patterns, offering a holistic view from high-level design to low-level execution.

### 1. The Factory Pattern: Creating Objects with Flexibility

The Factory pattern is a creational design pattern that provides an interface for creating objects in a superclass, allowing subclasses to alter the type of objects that will be created. This pattern is particularly useful when dealing with complex object creation processes or when you need to create objects without specifying their exact classes.

Let's implement a simple Factory pattern in C:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Product interface
typedef struct {
    void (*operation)(void);
} Product;

// Concrete products
typedef struct {
    Product base;
} ConcreteProductA;

typedef struct {
    Product base;
} ConcreteProductB;

void operationA(void) {
    printf("ConcreteProductA operation\n");
}

void operationB(void) {
    printf("ConcreteProductB operation\n");
}

// Factory function
Product* createProduct(const char* type) {
    if (strcmp(type, "A") == 0) {
        ConcreteProductA* product = malloc(sizeof(ConcreteProductA));
        product->base.operation = operationA;
        return (Product*)product;
    } else if (strcmp(type, "B") == 0) {
        ConcreteProductB* product = malloc(sizeof(ConcreteProductB));
        product->base.operation = operationB;
        return (Product*)product;
    }
    return NULL;
}

int main() {
    Product* productA = createProduct("A");
    Product* productB = createProduct("B");

    if (productA) {
        productA->operation();
        free(productA);
    }

    if (productB) {
        productB->operation();
        free(productB);
    }

    return 0;
}
```

To compile and run this code:

1. Save it to a file, e.g., `factory_pattern.c`
2. Compile it using: `gcc -o factory_pattern factory_pattern.c`
3. Run the executable: `./factory_pattern`

Expected output:
```
ConcreteProductA operation
ConcreteProductB operation
```

This implementation demonstrates the Factory pattern by providing a `createProduct` function that returns different product types based on the input. The `main` function showcases how to use this factory to create and use products.

To view the assembly code generated from this C code:

1. Use the command: `gcc -S factory_pattern.c`
2. This will generate a file named `factory_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `call` instruction for invoking the `createProduct` function
* The `cmp` and `je` instructions in the `createProduct` function, which correspond to the if-else logic for product creation
* The `mov` instructions for setting up function pointers (e.g., `product->base.operation = operationA;`)

**Factory Pattern Flow** 
![image](https://github.com/user-attachments/assets/f320af22-cf9b-471c-95d0-f8497970dfbd)


### 2. The Builder Pattern: Constructing Complex Objects Step by Step

The Builder pattern is another creational pattern that lets you construct complex objects step by step. It's particularly useful when an object needs to be created with numerous possible configurations.

Here's a C implementation of the Builder pattern:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 50

// Product
typedef struct {
    char type[MAX_LENGTH];
    char size[MAX_LENGTH];
    char topping[MAX_LENGTH];
} Pizza;

// Builder
typedef struct PizzaBuilder {
    Pizza pizza;
    void (*setType)(struct PizzaBuilder*, const char*);
    void (*setSize)(struct PizzaBuilder*, const char*);
    void (*addTopping)(struct PizzaBuilder*, const char*);
    Pizza* (*build)(struct PizzaBuilder*);
} PizzaBuilder;

// Builder methods
void setType(PizzaBuilder* builder, const char* type) {
    strncpy(builder->pizza.type, type, MAX_LENGTH - 1);
}

void setSize(PizzaBuilder* builder, const char* size) {
    strncpy(builder->pizza.size, size, MAX_LENGTH - 1);
}

void addTopping(PizzaBuilder* builder, const char* topping) {
    strncpy(builder->pizza.topping, topping, MAX_LENGTH - 1);
}

Pizza* build(PizzaBuilder* builder) {
    Pizza* pizza = malloc(sizeof(Pizza));
    *pizza = builder->pizza;
    return pizza;
}

// Initialize a new builder
PizzaBuilder* newPizzaBuilder() {
    PizzaBuilder* builder = malloc(sizeof(PizzaBuilder));
    builder->setType = setType;
    builder->setSize = setSize;
    builder->addTopping = addTopping;
    builder->build = build;
    return builder;
}

int main() {
    PizzaBuilder* builder = newPizzaBuilder();
    
    builder->setType(builder, "Margherita");
    builder->setSize(builder, "Large");
    builder->addTopping(builder, "Extra cheese");

    Pizza* pizza = builder->build(builder);

    printf("Pizza order: %s, %s, with %s\n", pizza->type, pizza->size, pizza->topping);

    free(pizza);
    free(builder);

    return 0;
}
```

To compile and run this code:

1. Save it to a file, e.g., `builder_pattern.c`
2. Compile it using: `gcc -o builder_pattern builder_pattern.c`
3. Run the executable: `./builder_pattern`

Expected output:
```
Pizza order: Margherita, Large, with Extra cheese
```

This implementation showcases the Builder pattern by providing a `PizzaBuilder` that allows step-by-step construction of a `Pizza` object. The `main` function demonstrates how to use the builder to create a customized pizza.

To view the assembly code:

1. Use the command: `gcc -S builder_pattern.c`
2. This will generate a file named `builder_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `call` instructions for invoking builder methods (`setType`, `setSize`, `addTopping`, `build`)
* The `mov` instructions for setting up function pointers in the `newPizzaBuilder` function
* The `lea` (Load Effective Address) instructions used in string operations

**Builder Pattern Flow**
![image](https://github.com/user-attachments/assets/f9ae16f8-182d-4537-b8c4-137a90a5c916)


### 3. The Singleton Pattern: Ensuring a Single Instance

The Singleton pattern is a creational pattern that ensures a class has only one instance and provides a global point of access to it. While it's often criticized for introducing global state, it can be useful in certain scenarios like managing a shared resource.

Here's a C implementation of the Singleton pattern:

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int value;
} Singleton;

static Singleton* instance = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Singleton* getInstance() {
    if (instance == NULL) {
        pthread_mutex_lock(&mutex);
        if (instance == NULL) {
            instance = (Singleton*)malloc(sizeof(Singleton));
            instance->value = 0;
        }
        pthread_mutex_unlock(&mutex);
    }
    return instance;
}

void incrementValue() {
    Singleton* s = getInstance();
    s->value++;
}

int getValue() {
    Singleton* s = getInstance();
    return s->value;
}

void cleanupSingleton() {
    pthread_mutex_lock(&mutex);
    if (instance != NULL) {
        free(instance);
        instance = NULL;
    }
    pthread_mutex_unlock(&mutex);
}

int main() {
    incrementValue();
    incrementValue();
    printf("Singleton value: %d\n", getValue());

    cleanupSingleton();
    return 0;
}
```

To compile and run this code:

1. Save it to a file, e.g., `singleton_pattern.c`
2. Compile it using: `gcc -o singleton_pattern singleton_pattern.c -lpthread`
3. Run the executable: `./singleton_pattern`

Expected output:
```
Singleton value: 2
```

This implementation demonstrates the Singleton pattern with thread-safety using a mutex. The `getInstance` function ensures that only one instance of the `Singleton` is created, and subsequent calls return the same instance.

To view the assembly code:

1. Use the command: `gcc -S singleton_pattern.c`
2. This will generate a file named `singleton_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `cmp` and `je` instructions in the `getInstance` function, corresponding to the null checks
* The `call` instructions for `pthread_mutex_lock` and `pthread_mutex_unlock`
* The `mov` instructions for accessing and modifying the singleton's value

**Singleton Pattern Flow**
![image](https://github.com/user-attachments/assets/d4078b94-0b31-424f-801b-1d46152cca09)


### 4. The Observer Pattern: Implementing Event Handling

The Observer pattern is a behavioral design pattern that lets you define a subscription mechanism to notify multiple objects about any events that happen to the object they're observing. It's widely used for implementing distributed event handling systems.

Here's a C implementation of the Observer pattern:

```c
#include <stdio.h>
#include <stdlib.h>

#define MAX_OBSERVERS 10

// Observer interface
typedef struct Observer {
    void (*update)(struct Observer*, int);
} Observer;

// Concrete Observer
typedef struct ConcreteObserver {
    Observer base;
    int id;
} ConcreteObserver;

void concreteUpdate(Observer* self, int value) {
    ConcreteObserver* concrete = (ConcreteObserver*)self;
    printf("Observer %d: Received update with value %d\n", concrete->id, value);
}

// Subject
typedef struct {
    Observer* observers[MAX_OBSERVERS];
    int observerCount;
    int state;
} Subject;

void initSubject(Subject* subject) {
    subject->observerCount = 0;
    subject->state = 0;
}

void attachObserver(Subject* subject, Observer* observer) {
    if (subject->observerCount < MAX_OBSERVERS) {
        subject->observers[subject->observerCount++] = observer;
    }
}

void setState(Subject* subject, int state) {
    subject->state = state;
    for (int i = 0; i < subject->observerCount; i++) {
        subject->observers[i]->update(subject->observers[i], state);
    }
}
```

To compile and run this code:

1. Save it to a file, e.g., `observer_pattern.c`
2. Compile it using: `gcc -o observer_pattern observer_pattern.c`
3. Run the executable: `./observer_pattern`

Expected output:
```
Observer 1: Received update with value 5
Observer 2: Received update with value 5
Observer 1: Received update with value 10
Observer 2: Received update with value 10
```

This implementation demonstrates the Observer pattern by defining a `Subject` that maintains a list of observers and notifies them when its state changes. The `ConcreteObserver` implements the `Observer` interface and receives updates from the subject.

To view the assembly code:

1. Use the command: `gcc -S observer_pattern.c`
2. This will generate a file named `observer_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `call` instructions for invoking the `update` function on each observer
* The `mov` instructions for setting up function pointers (e.g., `concreteUpdate`)
* The loop structure in the `setState` function, which iterates through the observers

**Observer Pattern Flow**
![image](https://github.com/user-attachments/assets/6486cda5-2de8-4c49-87b0-9d0656c35fcc)


### 5. The Strategy Pattern: Encapsulating Algorithms

The Strategy pattern is a behavioral design pattern that lets you define a family of algorithms, put each of them into a separate class, and make their objects interchangeable. This pattern is particularly useful when you have multiple algorithms for a specific task and want to be able to switch between them dynamically.

Here's a C implementation of the Strategy pattern:

```c
#include <stdio.h>
#include <stdlib.h>

// Strategy interface
typedef struct {
    int (*execute)(int, int);
} Strategy;

// Concrete strategies
int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

// Context
typedef struct {
    Strategy* strategy;
} Context;

void setStrategy(Context* context, Strategy* strategy) {
    context->strategy = strategy;
}

int executeStrategy(Context* context, int a, int b) {
    return context->strategy->execute(a, b);
}

int main() {
    Context context;
    Strategy addStrategy = { add };
    Strategy subtractStrategy = { subtract };
    Strategy multiplyStrategy = { multiply };

    setStrategy(&context, &addStrategy);
    printf("10 + 5 = %d\n", executeStrategy(&context, 10, 5));

    setStrategy(&context, &subtractStrategy);
    printf("10 - 5 = %d\n", executeStrategy(&context, 10, 5));

    setStrategy(&context, &multiplyStrategy);
    printf("10 * 5 = %d\n", executeStrategy(&context, 10, 5));

    return 0;
}
```

To compile and run this code:

1. Save it to a file, e.g., `strategy_pattern.c`
2. Compile it using: `gcc -o strategy_pattern strategy_pattern.c`
3. Run the executable: `./strategy_pattern`

Expected output:
```
10 + 5 = 15
10 - 5 = 5
10 * 5 = 50
```

This implementation showcases the Strategy pattern by defining different algorithms (`add`, `subtract`, `multiply`) as interchangeable strategies. The `Context` holds a reference to the current strategy and can switch between them dynamically.

To view the assembly code:

1. Use the command: `gcc -S strategy_pattern.c`
2. This will generate a file named `strategy_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `call` instructions for invoking the strategy's `execute` function
* The `mov` instructions for setting up function pointers in the strategy structures
* The indirect function calls through the strategy pointer in the `executeStrategy` function

**Strategy Pattern Flow**
![image](https://github.com/user-attachments/assets/0147965c-a855-414e-bbae-f5b4f3f769c6)


### 6. The Adapter Pattern: Bridging Incompatible Interfaces

The Adapter pattern is a structural design pattern that allows objects with incompatible interfaces to collaborate. It's particularly useful when you want to use an existing class, but its interface isn't compatible with the rest of your code.

Here's a C implementation of the Adapter pattern:

```c
#include <stdio.h>
#include <stdlib.h>

// Target interface
typedef struct {
    void (*request)(void*); 
} Target;

// Adaptee (the class that needs adapting)
typedef struct {
    void (*specificRequest)(void);
} Adaptee;

void specificRequest() {
    printf("Adaptee's specific request\n");
}

// Adapter
typedef struct {
    Target base;
    Adaptee* adaptee;
} Adapter;

void adapterRequest(void* context) {
    Adapter* self = (Adapter*)context;
    printf("Adapter: Translating the request\n");
    self->adaptee->specificRequest();
}

Adapter* createAdapter(Adaptee* adaptee) {
    Adapter* adapter = (Adapter*)malloc(sizeof(Adapter));
    adapter->base.request = adapterRequest;
    adapter->adaptee = adaptee;
    return adapter;
}

int main() {
    Adaptee adaptee = { specificRequest };
    Adapter* adapter = createAdapter(&adaptee);

    // Client code
    Target* target = (Target*)adapter;
    target->request(adapter);

    free(adapter);
    return 0;
}
```

To compile and run this code:

1. Save it to a file, e.g., `adapter_pattern.c`
2. Compile it using: `gcc -o adapter_pattern adapter_pattern.c`
3. Run the executable: `./adapter_pattern`

Expected output:
```
Adapter: Translating the request
Adaptee's specific request
```

This implementation demonstrates the Adapter pattern by creating an `Adapter` that wraps an `Adaptee` and presents a `Target` interface to the client. The `Adapter` translates the `request` call into a `specificRequest` call on the `Adaptee`.

To view the assembly code:

1. Use the command: `gcc -S adapter_pattern.c`
2. This will generate a file named `adapter_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `call` instructions for invoking the adapter's `request` function and the adaptee's `specificRequest` function
* The `mov` instructions for setting up function pointers in the adapter structure
* The `malloc` call in the `createAdapter` function for dynamic memory allocation

**Adapter Pattern Flow**
![image](https://github.com/user-attachments/assets/42221530-ca1d-4cca-bc7f-225463c5f78d)


### 7. The Facade Pattern: Simplifying Complex Systems

The Facade pattern is a structural design pattern that provides a simplified interface to a complex subsystem. It's useful when you want to provide a simple interface to a complex set of classes, hiding their complexities from the client.

Here's a C implementation of the Facade pattern:

```c
#include <stdio.h>

// Subsystem components
typedef struct {
    void (*operation1)(void);
    void (*operation2)(void);
} Subsystem1;

typedef struct {
    void (*operation1)(void);
    void (*operation2)(void);
} Subsystem2;

void subsystem1Operation1() {
    printf("Subsystem1: Operation1\n");
}

void subsystem1Operation2() {
    printf("Subsystem1: Operation2\n");
}

void subsystem2Operation1() {
    printf("Subsystem2: Operation1\n");
}

void subsystem2Operation2() {
    printf("Subsystem2: Operation2\n");
}

// Facade
typedef struct {
    Subsystem1 subsystem1;
    Subsystem2 subsystem2;
} Facade;

void initializeFacade(Facade* facade) {
    facade->subsystem1.operation1 = subsystem1Operation1;
    facade->subsystem1.operation2 = subsystem1Operation2;
    facade->subsystem2.operation1 = subsystem2Operation1;
    facade->subsystem2.operation2 = subsystem2Operation2;
}

void facadeOperation(Facade* facade) {
    printf("Facade: Performing complex operation\n");
    facade->subsystem1.operation1();
    facade->subsystem1.operation2();
    facade->subsystem2.operation1();
    facade->subsystem2.operation2();
}

int main() {
    Facade facade;
    initializeFacade(&facade);

    facadeOperation(&facade);

    return 0;
}
```

To compile and run this code:

1. Save it to a file, e.g., `facade_pattern.c`
2. Compile it using: `gcc -o facade_pattern facade_pattern.c`
3. Run the executable: `./facade_pattern`

Expected output:
```
Facade: Performing complex operation
Subsystem1: Operation1
Subsystem1: Operation2
Subsystem2: Operation1
Subsystem2: Operation2
```

This implementation demonstrates the Facade pattern by creating a `Facade` that wraps two subsystems (`Subsystem1` and `Subsystem2`). The `facadeOperation` function provides a simplified interface to perform a complex operation involving both subsystems.

To view the assembly code:

1. Use the command: `gcc -S facade_pattern.c`
2. This will generate a file named `facade_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `call` instructions for invoking the subsystem operations
* The `mov` instructions for setting up function pointers in the `initializeFacade` function
* The sequence of function calls in the `facadeOperation` function

**Facade Pattern Flow**
![image](https://github.com/user-attachments/assets/33b0d4a3-3d24-4a55-a414-efe5b5f47423)


### 8. The Iterator Pattern: Traversing Collections

The Iterator pattern is a behavioral design pattern that lets you traverse elements of a collection without exposing its underlying representation (list, stack, tree, etc.). It's particularly useful when you want to provide a standard way to iterate over different types of collections.

Here's a C implementation of the Iterator pattern:

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Iterator interface
typedef struct Iterator {
    bool (*hasNext)(struct Iterator*);
    int (*next)(struct Iterator*);
} Iterator;

// Concrete Iterator
typedef struct {
    Iterator base;
    int* collection;
    int size;
    int position;
} ArrayIterator;

bool arrayIteratorHasNext(Iterator* iterator) {
    ArrayIterator* arrayIterator = (ArrayIterator*)iterator;
    return arrayIterator->position < arrayIterator->size;
}

int arrayIteratorNext(Iterator* iterator) {
    ArrayIterator* arrayIterator = (ArrayIterator*)iterator;
    if (arrayIteratorHasNext(iterator)) {
        return arrayIterator->collection[arrayIterator->position++];
    }
    return -1; // Or some error value
}

ArrayIterator* createArrayIterator(int* collection, int size) {
    ArrayIterator* iterator = (ArrayIterator*)malloc(sizeof(ArrayIterator));
    iterator->base.hasNext = arrayIteratorHasNext;
    iterator->base.next = arrayIteratorNext;
    iterator->collection = collection;
    iterator->size = size;
    iterator->position = 0;
    return iterator;
}

int main() {
    int collection[] = {1, 2, 3, 4, 5};
    int size = sizeof(collection) / sizeof(collection[0]);

    ArrayIterator* iterator = createArrayIterator(collection, size);

    while (iterator->base.hasNext((Iterator*)iterator)) {
        printf("%d ", iterator->base.next((Iterator*)iterator));
    }
    printf("\n");

    free(iterator);
    return 0;
}
```

To compile and run this code:

1. Save it to a file, e.g., `iterator_pattern.c`
2. Compile it using: `gcc -o iterator_pattern iterator_pattern.c`
3. Run the executable: `./iterator_pattern`

Expected output:
```
1 2 3 4 5 
```

This implementation demonstrates the Iterator pattern by creating an `ArrayIterator` that provides a way to iterate over an array. The iterator encapsulates the logic for traversing the collection, allowing the client code to iterate without knowing the underlying structure.

To view the assembly code:

1. Use the command: `gcc -S iterator_pattern.c`
2. This will generate a file named `iterator_pattern.s` containing the assembly code.

Key assembly instructions to note:

* The `call` instructions for invoking the iterator's `hasNext` and `next` functions
* The `mov` instructions for setting up function pointers in the `createArrayIterator` function
* The `cmp` and `jl` instructions in the `arrayIteratorHasNext` function for boundary checking

**Iterator Pattern Flow**
![image](https://github.com/user-attachments/assets/2bf88835-60a7-4519-afec-40d45407f0d8)


### Conclusion

Design patterns are powerful tools in a programmer's arsenal, providing tested solutions to common software design problems. By understanding and implementing these patterns, developers can create more flexible, maintainable, and scalable code.

In this blog post, we've explored eight fundamental design patterns:

* Factory Pattern
* Builder Pattern
* Singleton Pattern
* Observer Pattern
* Strategy Pattern
* Adapter Pattern
* Facade Pattern
* Iterator Pattern

Each pattern addresses specific design challenges:

* **Creational patterns** like Factory, Builder, and Singleton deal with object creation mechanisms.
* **Structural patterns** like Adapter and Facade help in composing objects and classes into larger structures.
* **Behavioral patterns** like Observer, Strategy, and Iterator define how objects interact and distribute responsibilities.

Remember, while design patterns are valuable tools, they should be applied judiciously. Over-engineering or forcing patterns where they're not needed can lead to unnecessary complexity. Always consider the specific needs of your project and use patterns where they provide clear benefits in terms of flexibility, maintainability, or scalability.

As you continue your journey in software development, practice implementing these patterns in your projects. Experiment with different scenarios and see how they can improve your code's structure and readability. With time and experience, you'll develop an intuition for when and how to apply these patterns effectively.
