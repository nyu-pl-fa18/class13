#pragma once

#include <cstring>

#if 1
#include <iostream>
#define TRACE(s) \
  std::cout << __FUNCTION__ << ":" << __LINE__ << ":" << s << std::endl
#else
#define TRACE(s)
#endif

template<typename T>
class Ptr {
  T* addr;
  size_t* counter;

 public:
  // constructor to wrap raw pointer (and default constructor)
  Ptr(T* addr = 0) : addr(addr), counter(new size_t(1)) {
    TRACE(addr);
  }

  // copy constructor
  Ptr(const Ptr<T>& other) : addr(other.addr), counter(other.counter) {
    TRACE(addr);
    ++(*counter);
  }

  // destructor
  ~Ptr() {
    TRACE(addr);
    if(0 == --(*counter)) {
      if (0 != addr) {
        delete addr;
      }
      delete counter;
    }
  }

  // assignment operator
  Ptr& operator=(const Ptr& right) {
    TRACE(addr);
    if (addr != right.addr) {
      if (0 == --(*counter)) {
        if (addr != 0) delete addr;
        delete counter;
      }
      addr = right.addr;
      counter = right.counter;
      ++(*counter);
    }
    return *this;
  }

    
  T& operator*()  const { TRACE(addr); return *addr; }
  T* operator->() const { TRACE(addr); return addr;  }
  
  bool operator==(const Ptr<T>& other) const {
    return addr == other.addr;
  }
    
  bool operator!=(const Ptr<T>& other) const {
    return addr != other.addr;
  }

};

