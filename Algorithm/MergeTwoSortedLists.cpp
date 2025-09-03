

struct ListNode {
  int val;
  ListNode *next;
  ListNode() : val(0), next(nullptr) {}
  ListNode(int x) : val(x), next(nullptr) {}
  ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class MergeTwoSortedLists {
public:
  ListNode *run(ListNode *list1, ListNode *list2) {
    ListNode dummy{0};
    ListNode *current = &dummy;
    while (list1 && list2) {
      if (list1->val < list2->val) {
        current->next = list1;
        list1 = list1->next;
      } else {
        current->next = list2;
        list2 = list2->next;
      }
      current = current->next;
    }
    if (list1) {
      current->next = list1;
    } else {
      current->next = list2;
    }
    return dummy.next;
  }
};

/*

Time Complexity: O(n + m)
- n: len(list1)
- m: len(list2)
Auxiliary Space Complexity: O(1)

#towPointers

*/