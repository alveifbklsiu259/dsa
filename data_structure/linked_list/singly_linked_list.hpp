#pragma once

#include "./linked_list_base.hpp"
#include "./node.hpp"

namespace linkedlist {
template <typename T> class SinglyLinkedList : public LinkedListBase<SinglyLinkNode<T>> {};
} // namespace linkedlist
