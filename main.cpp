#include "./DataStructure/Array/DynamicArray.hpp"
#include "./DataStructure/Array/StaticArray.hpp"
#include "./DataStructure/LinkedList/DoublyLinkedList.hpp"
#include "./DataStructure/LinkedList/SinglyLinkedList.hpp"
#include "./DataStructure/Queue/Queue.hpp"
#include "./DataStructure/Stack/Stack.hpp"
#include <iostream>
#include <unordered_set>

#include <vector>

int main() {
  Queue::Queue<int, 5> q;
  q.enqueue(1);
  q.enqueue(2);
  q.enqueue(3);
  q.enqueue(4);
  q.enqueue(5);
  Queue::Queue<int, 8> q2 = std::move(q);
  q2.enqueue(6);
  q2.enqueue(7);
  q2.enqueue(8);

  std::cout << q2.getSize() << std::endl;
  while (!q2.isEmpty()) {
    std::cout << q2.dequeue() << std::endl;
  }

  /*  */
  return 0;
}

// what is CTAD?
