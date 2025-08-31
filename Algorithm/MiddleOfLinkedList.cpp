struct ListNode {
  int val;
  ListNode *next;
  ListNode() : val(0), next(nullptr) {}
  ListNode(int x) : val(x), next(nullptr) {}
  ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class MiddleOfLinkedList {
public:
  ListNode *run(ListNode *head) {
    if (!head || !head->next)
      return nullptr;

    ListNode *slow = head;
    ListNode *fast = head;

    while (fast && fast->next) {
      slow = slow->next;
      fast = fast->next->next;
    }
    return slow;
  }
};

/*

Time Complexity: O(n)
Auxiliary Space Complexity: O(1)

#towPointers

*/