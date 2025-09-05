#pragma once

#include "./Iterator.hpp"
#include "./LinkedListBase.hpp"
#include "./Node.hpp"

namespace LinkedList {
template <typename T>
class SinglyLinkedList : public LinkedListBase<SinglyLinkNode<T>> {};
} // namespace LinkedList
