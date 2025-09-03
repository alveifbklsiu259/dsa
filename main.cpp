#include "./DataStructure/LinkedList/DoublyLinkedList.hpp"
#include "./DataStructure/LinkedList/SinglyLinkedList.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

int main() {

  LinkedList::DoublyLinkedList<int> d;
  d.pushFront(1);
  d.pushFront(2);
  d.pushFront(3);
  d.pushFront(4);
  d.print();
  d.reverse();
  d.print();
  return 0;
};

// smart ptr
// does C++ have built-in linked list?

/*

LinkedListBase still takes two generics

### 7. **Insert/Delete at Arbitrary Position**
With both `prev` and `next`, you can insert or delete in O(1) time if you have a
pointer to the node. This is useful for:

- `insertAfter(DoublyLinkedListNode<T>*, const T&)`
- `remove(DoublyLinkedListNode<T>*)`


*/

/*

class Head {
public:
  int value;
  Head *next = nullptr;
  Head() = default;
};

we can't have virtual constructor, but we can have virtual destructor, why



*/