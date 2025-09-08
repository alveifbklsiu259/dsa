#include "./DataStructure/Array/DynamicArray.hpp"
#include "./DataStructure/Array/StaticArray.hpp"
#include "./DataStructure/LinkedList/DoublyLinkedList.hpp"
#include "./DataStructure/LinkedList/SinglyLinkedList.hpp"
#include "./DataStructure/Stack/Stack.hpp"
#include <iostream>
#include <unordered_set>

int main() {
  Stack::Stack<std::string, 5> s;

  // std::cout << s.getCapacity() << std::endl;
  // std::cout << s.getSize() << std::endl;

  s.push("1");
  s.push("2");
  s.push("34");

  // Stack::Stack<int, 8> s2 = s;
  Stack::Stack<std::string, 8> s2;
  s2 = std::move(s);
  std::cout << s2.pop() << std::endl;

  std::cout << s2.getCapacity() << std::endl;
  std::cout << s2.getSize() << std::endl;

  // ---

  // std::cout << s.pop() << std::endl;
  // std::cout << s.pop() << std::endl;
  // std::cout << s.pop() << std::endl;
  // std::cout << std::boolalpha << s.isEmpty() << std::endl;
  // s.peek();

  return 0;
};
