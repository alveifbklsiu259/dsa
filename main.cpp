#include "./DataStructure/DynamicArray.hpp"
#include "./DataStructure/LinkedList/SinglyLinkedList.hpp"
#include "./DataStructure/StaticArray.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
  LinkedList::SinglyLinkedList<int> list;
  list.pushFront(1);
  list.pushFront(2);
  list.pushFront(3);

  LinkedList::SinglyLinkedList<int> list2 = list;

  for (const auto &i : list2) {
    std::cout << i << std::endl;
  }

  return 0;
};
