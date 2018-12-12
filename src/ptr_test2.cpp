#include <iostream>
#include "ptr.h"

struct A;

struct B {
  Ptr<A> a;
  B(Ptr<A>& _a) : a(_a) { TRACE("B(Ptr<A>&)"); }
  B(A* _a) : a(_a) { TRACE("B(A*)"); }
  ~B() { TRACE("~B"); }
};

struct A {
  Ptr<B> b;
  A(Ptr<B>& _b) : b(_b) { TRACE("A(Ptr<B>&)");}
  ~A() { TRACE("~A"); }
};

int main(void) {
  Ptr<B> pn = 0;

  Ptr<B> pb = new B(0);

  Ptr<A> pa = new A(pb);

  pb->a = pa;

  pa->b = pn;

  return 0;
}
