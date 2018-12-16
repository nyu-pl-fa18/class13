# Class 13

## Memory Management

Remember from the discussion in [Class 2](../../../class02) that the memory used to
store the data and code of a program at run-time is split into three
parts:

1. the data segment
1. the stack, and
1. the heap

The data segment stores objects whose lifetime comprises the entire
program execution (e.g. global variables, code of functions, auxiliary
tables generated by the compiler such as vtables etc.).

The stack is used to store the values of local variables in function
activation records. When it comes to memory management, allocating
data on the stack is generally prefered because it has low overhead
and the allocated space is automatically freed when the data is no
longer needed (e.g. a local variable goes out of scope). However,
there are two limitations for stack allocated data:

* The lifetime of objects stored on the heap is given by the FIFO
  order of function invocations. If the lifetime of an object is
  dynamic and not correlated with any function invocation, then it
  cannot be allocated on the stack (e.g. function closures, .

* Objects that are allocated on the stack cannot change their size
  dynamically. For instance, if we want to implement resizable arrays,
  then such data structures cannot be stored on the stack.
  
Objects whose lifetime or size is dynamic must be allocated on the
heap.

With heap allocated objects, memory management becomes more
complicated because it is more difficult to determine when an object
should be deallocated. Broadly programming languages fall into two
categories:

* heap memory is managed manually by the programmer, or
* heap memory is managed automatically by the language run-time environment

Manual memory management is generally more efficient than automatic
memory management. However, it is also a source of common bugs such as

* use after free errors (accessing dangling pointers)
* double free errors (deallocating objects multiple times)
* memory leaks (not deallocating objects after they are no longer
  used)
  
These errors can be eliminated or mitigated by relying on automatic
memory management techniques. In the following, we will discuss three
techniques for automatic memory management that are widely used by
different languages:

* Garbage collection
* Reference counting
* Ownership types

### Garbage Collection

*Garbage collection* (GC) refers to any algorithm for automatic
deallocation. These algorithms are typically implemented in the
run-time environments of high-level programming languages (e.g. the
JVM in the case of Java/Scala, and the OCaml or Python run-time).

The most common garbage collection variations are:

* Mark/sweep
  * Variant: compacting
  * Variant: non-recursive

* Copying
  * Variant: incremental
  * Variant: generational

During garbage collection, the aim is to deallocate any object that
will never be used again. Though, how do you know if an object will
never be used again?

An approximation is the set of objects that are *live*. An object `x`
is *live* if:

* `x` is pointed to by a variable,
  * on the stack (e.g., in an activation record)
  * that is global (i.e. resides in the program's data segment)

* there is a register (containing a temporary or intermediate value) that points to `x`, or

* there is another object on the heap (e.g., `y`) that is live and points to `x`

All live objects in the heap can be found by a graph traversal:
* Start at the *roots*: local variables on the stack, global variables, registers
* Any object not reachable from the roots is *dead* and can be reclaimed.

#### Mark/Sweep GC

Mark/sweep is a simple garbage collection algorithm that works as follows:

* Each object has an extra bit called the *mark bit*

* *Mark phase*: the collector traverses the heap and sets the mark bit
  of each live object encountered

* *Sweep phase*: each object whose mark bit is not set goes on the free list

```
GC() 
  for each root pointer p do
    mark(p);
  sweep();

mark(p)
  if p->mark != 1 then
    p->mark = 1;
    for each pointer field p->x do
      mark(p->x);

sweep()
  for each object x in heap do
    if x.mark == 0 then insert(x, free_list);
                   else x.mark = 0;
```

#### Copying GC

A more sophisticated GC algorithm that is commonly used in practice is
based on copying live objects. The basic algorithm works as follows:

* Heap is split into 2 parts: *FROM* space, and *TO* space
* Objects allocated in *FROM* space
* When *FROM* space is full, garbage collection begins
* During traversal, each encountered object is copied to *TO* space
* When traversal is done, all live objects are in *TO* space
* Now we flip the spaces -- *FROM* space becomes *TO* space and vice-versa
* Since we are moving objects, any pointers to them must be updated:
  this is done by leaving a *forwarding address*

```
GC()
  for each root pointer p do
    p = traverse(p);
    
traverse(p)
  if *p contains forwarding address then
    p = *p;  // follow forwarding address
    return p;
  else 
    new_p = copy (p, TO_SPACE);
    *p = new_p; // write forwarding address
    for each pointer field p->x do
      new_p->x = traverse(p->x);
    return new_p;
```

A variant of Copying GC is *Generational* GC.

*Observation:* the older an object gets, the longer it is expected to
stay around. This is because

* Many objects are very short-lived (e.g., intermediate values)
* Objects that live for a long time tend to make up central data
  structures in the program, and will probably be live until the end
  of the program.

*Idea:* instead of 2 heaps, use many heaps, one for each "generation".

* Younger generations collected more frequently than older generations
  (because younger generations will have more garbage to collect)
* When a generation is traversed, live objects are copied to the next-older generation
* When a generation fills up, we garbage collect it.


### Reference Counting

One issue with garbage collection is that the points when the garbage
collector are invoked are usually not under the control of the
programmer. While the garbage collector is running, the program will
be either halted (in the case of a sequential GC algorithm) or at the
very least slowed down (in the case of a concurrent GC
algorithm). This makes garbage collected languages often unusable for
applications that need to satisfy strict timing guarantees
(e.g. real-time applications such as multi-media apps, video games,
embedded controllers, etc.).

An alternative approach to automated memory management is *reference
counting*. Here, the idea is to keep track of how many references
point to an object and free it when there are no more. That is:

* Set reference count to 1 for newly created objects.

* Increment reference count whenever we copy the pointer to the object.

* Decrement count when a point to the object goes
  out of scope or stops pointing to the object.

* When the count gets to 0, we can deallocate the object

**Advantages**:

  * More predictable performance: Memory can be reclaimed as soon as
    no longer needed. Rather than spending a lot of time on executing
    individual GC cycles, the time spent on memory management is more
    evenly distributed throughout program execution, which increases
    responsiveness.

  * Simplicity: Can be done by the programmer for languages not
    supporting GC.

**Disadvantages**:

  * Additional space is needed for keeping track of the reference count.

  * We still have some run-time overhead to perform the necessary
    bookkeeping for creating/deleting/updating the counters.

  * Will not reclaim circular reference structures (e.g. a cyclic list
    on the heap).

#### Implementing Reference Counting with Smart Pointers

A common way to implement reference counting is in the form of a
*smart pointer* library. We demonstrate how to do this in C++, which
allows us to implement smart pointers in a way that mimics a regular
pointer in syntax and semantics, making it convenient to use for
clients.

* With smart pointers we can do some of what a garbage collector does
  for us (i.e. automatically deallocate heap memory that is no longer
  needed).

* Our smart pointer will do reference counting, as discussed above.

* An implementation of this is included in this repository. The
  library is implemented as a template class `Ptr<T>` that represents
  a smart pointer to some value of type `T`.

* One core question, though: if we want to track the reference count
  of every object, where in memory do we store that information?

  * Do we store the count as a field inside each `Ptr` instance
    (internal containment)?

  * That has problems...

    * When two instances of `Ptr` point to the same underlying object
      and one of them goes out of the scope, both counters in the two
      `Ptr`s need to be decremented.

    * Clunky and difficult to access internal state of separate
      instance.

  * How about inside the data layout of the referenced values `T`. 
  
    * We don't want to implement this either because we want our code
      to support pointers to values of primitive types like `int` and
      also null pointers, neither of which have a data layout.

  *  How about we store the counter as a separate object on the heap
     with a reference to it in each `Ptr`? (external
     containment)

     * With this approach the counter can be shared across all
       `Ptr`s that reference the same object.

     * Whenever we do an assignment or a copy of a `Ptr` we increment the
       shared counter (respectively, decrement the counter to the previously
       referenced object).
  
Here is the basic skeleton of our smart pointer class:  
  
```c++
template<typename T>
class Ptr {
  T* addr; // pointer to referenced object
  size_t* counter; // pointer to counter shared by all Ptrs that point to addr.
public:
  ...
};
```

The notation, `template<typename T>` is C++'s way of defining a
generic class or function. Here, we define a generic class `Ptr` that
is parameterized by a type `T` (the type of values to which we
point)`.

Each instance of `Ptr<T>` have two fields:

* a pointer `addr` to a value of type `T`. This is the *raw pointer*
  that is managed by the smart pointer.
  
* a pointer `counter` that points to the counter (of type `size_t`)
  used to keep track of how many references we have to the value
  pointed to by `addr`.

Before we go into the details of the implementation, we need to
briefly discuss how memory management works in C++.

#### Explicit Memory Management in C++

In C++, heap allocated objects need to be managed explicitly. When a
heap allocated object is no longer needed, it must be deallocated
explicitly by the program. Otherwise, the program leaks memory.

There are three important methods related to memory management that
every C++ class `A` needs to implement:

**Copy constructor**: this constructor is used whenever the compiler
automatically creates a copy of an `A` instance. This happens e.g.
when an instance is passed by value to a function or when one instance
is used to initialize another:

```C++
A x; // allocate an A instance on the stack (calls default constructor of A)
A y = x; // calls copy constructor of A with x as argument to initialize y
```

**Assignment operator**: this method is called whenever we assign
another instance of the class to the current instance of the class:

```C++
A x; 
A y;
y = x; // calls assignment operator of `A` on y with x as argument.
```

**Destructor**: intuitively, you can think of a destructor as the
opposite of a constructor. A destructor is a method which is
automatically invoked when the object is destroyed, i.e., when the
memory of the instance is to be freed. Its main purpose is to free the
resources (memory allocations, open files etc.) which were acquired by
the instance along its life cycle. Destructors are necessary because
C++ does not have fully automatic memory management. The destructor of
a class `A` is denoted by `~A`.
    
* If an instance of a class `A` is allocated on the stack, the
  destructor `~A` is called automatically when the instance goes out
  of scope.
    
* If an instance of a class `A` is allocated on the heap (via `new
  A(...);`), the destructor `~A` is called when the instance is
  explicitly deallocated with a `delete` statement.
    
  Unlike Scala, C++ does not automatically reclaim heap-allocated
  memory when it is no longer reachable from any global
  or stack variable. It is the program's responsibility to delete
  heap-allocated instances via explicit `delete` calls.

If the programmer does not define these three methods explicitly, then
the C++ compiler will add them automatically. The semantics of the
default implementations are:

* Destructor - Call the destructors of all the object's class-type
  members (note that for pointer-typed members, the destructors of
  the referenced external objects are **not** called by the default
  destructors).

* Copy constructor - Construct all the object's members from the
  corresponding members of the copy constructor's argument (shallow copy).

* Assignment operator - Assign all the object's members
  from the corresponding members of the assignment operator's
  argument (shallow copy).

If one of the three methods has to be defined with a different
semantics than the compiler-generated version, it means that the
compiler-generated versions of the other two methods also likely do
not fit the needs of the class. This is known as the *Rule of Three*.

Typically, the way to go about memory management in C++ is that the
references to heap allocated objects are encapsulated in other objects
which are stored as values on the stack. The copy constructors,
assignment operators, and destructors of the stack-allocated objects
take care of managing the heap allocated objects and freeing them when
they are no longer needed. Since destructors of stack-allocated
objects are called automatically when the objects are popped from the
stack, this approach yields a semi-automatic memory management
strategy.
  
Where things get more complicated is when heap allocated objects are
shared between several stack allocated objects. So in this case, it is
in general unclear which object is responsible for *cleaning up*. This
is where smart pointers come into play.
  
#### Back to our smart pointer class

We discuss the methods of our smart pointer class `Ptr` in turn:

* Standard constructor

  ```c++
  Ptr(T* _addr = 0) : addr(_addr), counter(new size_t(1)) {}
  ```
  
  This constructor is called when we create a `Ptr` to wrap around a
  raw pointer `_addr` to a new heap allocated object of type `T`. We
  use `_addr` to initialize the field `addr` of the `Ptr` instance and
  create a new counter on the heap. The counter is initialized to 1
  since this `Ptr` holds the only pointer to `*addr`.
  
* Copy constructor

  ```c++
  Ptr(const Ptr<T>& other) : addr(other.addr), counter(other.counter) {
    ++(*counter);
  }
  ```

  When copying a `Ptr` instance `other`, we initialize the copy with a
  pointer to the same counter on the heap and increment the counter.


* Destructor 

  ```c++
  ~Ptr() {
    if (0 == --(*counter)) {
      delete addr;
      delete counter;
    }
  }
  ```
  
  When the current `Ptr` is destroyed, its destructor is called. The
  destructor first decrements the reference counter, since we now have
  one fewer reference to `addr`. When the counter becomes 0, i.e., we
  are destructing the last reference to `addr`. In this case, both the
  counter value on the heap, as well as the referenced object are
  deleted.
    
  Note that we do not need to safeguard against deleting `addr` when it is
  a `NULL` pointer because `delete` has no effect when its operand is `NULL`.

* Assignment Operator

  ```c++
  Ptr& operator=(const Ptr& right) {
    if (addr != right.addr) {
      if (0 == --(*counter)) {
        delete addr;
        delete counter;
      }
      addr = right.addr;
      counter = right.counter;
      ++(*counter);
    }
    return *this;
  }
  ```
  
  Whenever we do an assignment between two `Ptr` instances, we first
  do a self-assignment check: if we try to assign a `Ptr` instance to
  itself, then there is nothing to be done and we simply return.
  
  Otherwise, we decrement the counter for the object that `addr`
  currently points to. If that counter becomes 0, the current `Ptr`
  was the last one pointing to it. So we can delete both the counter
  and the object.

  Then we copy over the values of `addr` and `counter` from the other
  instance `right` and increment the new counter since we now hold one
  more reference to the object pointed to by `right`.

  Note that the self-assignment check is critical for the correctness
  of the implementation. If the current instance is the only instance
  holding a reference to `addr` when the assignment operator is
  called, then without the self-assignment check we would delete the
  object and counter, which would lead to use after free error when we
  subsequently try to increment the deleted counter.

Let's see this in action:

```c++
1: {
2:   Ptr<int> p = new int(3);
3:   Ptr<int> q = p;
4:   cout << *q << endl;
5: } 
```

1. In line 2, we allocate an `int` on the heap which is initialized
   to 3. Then we call the constructor `Ptr(int*)` to create `p` on the
   stack, passing to the constructor the address of the `int`
   value 3 on the heap.
   
2. In line 3, we call the copy constructor `Ptr(const &Ptr<int>)` with
   `p` as argument to create `q` on the stack.

3. In line 4, we call `Ptr::operator*` which returns the value pointed
   to by the field `addr` of `p`, which is 3, and then pass it to
   `cout` which prints 3.

4. In line 5, both `p` and `q` go out of scope. Hence, they are popped
   from the stack and the destructor `~Ptr` is called for each of
   them. The first destructor call will decrement the value of the
   counter associated with the `int` value on the heap to 1. The
   second destructor call will decrement it to 0 and then delete both
   the counter as well as the `int` value.

Here is another example

```c++
1: struct Node {
2:   Ptr<Node> next;
3: };
4:
5: {
6:   Ptr<Node> p = new Node(); 
7:   p->next = p;
8: }
```

When line 8 is executed, we set the smart pointer inside of the `Node`
object on the heap that is pointed to by `p` back to itself, creating
a cyclic pointer structure. I.e. at this point, we have one counter
associated with the `Node` object created on line 6 and its value is 2
because we have two references to the object: one from `p` and one
from `p->next`.

When `p` goes out of scope on line `8`, then `p` is popped from the
stack and its destructor is called. The destructor will decrement the
counter associated with the `Node` object to 1. Since the counter is
not 0 (we still have the cyclic reference from `p->next` back to `p`),
the `Node` object and its counter are not deleted. That is, here we
leak memory despite the usage of the smart pointers. Thus, when using
smart pointers, you have to be careful not to create cylic structures
with them.

You can find the smart pointer implementation in the file
[`src/ptr.h`](src/ptr.h) and a test program in
[`src/ptr_test.cpp`](src/ptr_test.cpp).

To compile the test program you can use the `cmake` build tool. If you
have `cmake` installed, then run the following command once at the
root of the repo:

```bash
cmake .
```

Then you can run

```bash
make
./ptr_test
```

whenever you want to compile and run the test program.

### Ownership Types

While reference counting is a relatively light-weight approach to
realize automated memory management, it still has the run-time
overhead for the booking of the counters.

An alternative approach is to move all the reasoning involved in
determining which references are alive at what time from run-time to
compile-time. That is, can we get automated memory management without
any run-time overhead? The answer is, perhaps surprisingly "yes".

One way to realize this by implementing a more sophisticated static
type system in the compiler that keeps track of which pointer *owns*
which heap-allocated object and is, hence, responsible for deleting
the object when it is no longer needed. An example of language with
such an *ownership type system* is [Rust](https://www.rust-lang.org/).

See [here](https://doc.rust-lang.org/book/2018-edition/ch04-00-understanding-ownership.html) for a
tutorial explaining the basics of how memory management works in Rust.
