#include <iostream>
#include "ptr.h"

using namespace std;

struct Node {
  Ptr<Node> next;
};

int main(void) {
  TRACE("declaration of p");          Ptr<int> p = new int(5);
  TRACE("declaration of q");          Ptr<int> q;
  TRACE("assignment of q");           q = p;
  TRACE("dereferencing q");           cout << *q << endl;

  TRACE("declaration of n");          Ptr<Node> n = new Node();
  TRACE("assignment of n->next");     n->next = n; // creates cyclic reference that will cause memory leakage
  TRACE("return");                    return 0;
}
