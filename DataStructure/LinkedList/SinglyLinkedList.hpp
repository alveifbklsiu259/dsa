#pragma once

#include "./LinkedListBase.hpp"
#include "./Node.hpp"

namespace linkedlist {
template <typename T> class SinglyLinkedList : public LinkedListBase<SinglyLinkNode<T>> {};
} // namespace linkedlist
