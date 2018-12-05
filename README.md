# Class 13

## Memory Management



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
    p->mark := 1;
    for each pointer field p->x do
      mark(p->x);

sweep()
  for each object x in heap do
    if x.mark = 0 then insert(x, free_list);
                  else x.mark := 0;
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
    p := traverse(p);
    
traverse(p)
  if *p contains forwarding address then
    p := *p;  // follow forwarding address
    return p;
  else 
    new_p := copy (p, TO_SPACE);
    *p := new_p; // write forwarding address
    for each pointer field p->x do
      new_p->x := traverse(p->x);
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
collector are invoked are typically outside of the control of the
programmer. While the garbage collector is running, the program will
be either halted (in the case of a sequential GC) or at the very least
slowed down (in the case of a concurrent GC). This makes garbage
collected languages often unusable for applications that need to
satisfy strict timing guarantees (e.g. real-time applications such as
multi-media apps, video games, embedded controllers, etc.).

An alternative approach to automated memory management is *reference
counting*. Here, the idea is to keep track of how many references
point to an object and free it when there are no more:

* Set reference count to 1 for newly created objects.

* Increment reference count whenever we copy the pointer to the object.

* Decrement count when a point to the object goes
  out of scope or stops pointing to the object.

* When the count gets to 0, we can free the memory

* Advantages:

  * More predictable performance: Memory can be reclaimed as soon as
    no longer needed. Rather than spending a lot of time on executing
    individual GC cycles, the time spent on memory management is more
    evenly distributed throughout program execution, which increases
    responsiveness.

  * Simplicity: Can be done by the programmer for languages not
    supporting GC.

* Disadvantages:

  * Additional space is needed for keeping track of the reference count.

  * We still have some run-time overhead to perform the necessary
    bookkeeping.

  * Will not reclaim circular reference structures (e.g. a cyclic list
    on the heap).

#### Implementing Reference Counting with Smart Pointers

A common way to implement reference counting is in the form of a
*smart pointer* library. We demonstrate how to do this in C++, which
allows us to implement smart pointers in a way that mimics a regular
pointer in syntax and semantics, so that it is convenient to use for
clients.

* With smart pointers we can do some of what a garbage collector does
  for us (i.e. automatically deallocate heap memory that is no longer
  needed).

* Our smart pointer will do reference counting, as discussed above.

* An implementation of this is included in this repository. It is
  implemented as a template class `Ptr<T>` that represents a smart
  pointer to some value of type `T`.

* One core question, though, if we want to track the reference count
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
      to support pointers to values of primitive types like `int` as
      well as null pointers neither of which have a data layout.

  *  How about we store the counter as a separate object on the heap
     with a reference to it in each `Ptr`? (external
     containment)

     * With this approach the counter can be shared across all
       `Ptr`s that reference the same object instance.

     * Whenever we do an assignment or a copy of `Ptr` we increment the
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

*Copy constructor*: this constructor is used whenever the compiler
automatically creates a copy of an `A` instance. This happens e.g.
when an instance is passed by value to a function or when one instance
is used to initialize another:

```C++
A x; // allocate an A instance on the stack (calls default constructor of A)
A y = x; // calls copy constructor of A with x as argument to initialize y
```

*Assignment operator*: this operator is called whenever we assign
another instance of the class to the current instance of the class:

```C++
A x; 
A y;
y = x; // calls assignment operator of `A` on y with x as argument.
```

*Destructor*: intuitively, you can think of a destructor as the
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
      if (0 != addr) delete addr;
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
    
  Note that we need to safeguard against deleting `addr` when it is
  a `NULL` pointer.

* Assignment Operator

  ```c++
  Ptr& operator=(const Ptr& right) {
    if (addr != right.addr) {
      if (0 == --(*counter)) {
        if (0 != addr) delete addr;
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
  object and counter, which would lead to dangling pointers in `addr`
  and `counter`.


### Ownership Types
