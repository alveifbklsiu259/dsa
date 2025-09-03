
struct ListNode {
  int val;
  ListNode *next;
  ListNode() : val(0), next(nullptr) {}
  ListNode(int x) : val(x), next(nullptr) {}
  ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class AddTwoNumbers {
public:
  ListNode *run(ListNode *l1, ListNode *l2) {
    ListNode *l1Current = l1;
    ListNode *l2Current = l2;
    ListNode *dummy = new ListNode(0);
    ListNode *current = dummy;

    int carry = 0;
    while (l1Current || l2Current || carry) {
      int sum = carry;

      if (l1Current) {
        sum += l1Current->val;
        l1Current = l1Current->next;
      }

      if (l2Current) {
        sum += l2Current->val;
        l2Current = l2Current->next;
      }

      carry = sum / 10;
      int remainder = sum % 10;

      ListNode *newNode = new ListNode(remainder);
      current->next = newNode;
      current = newNode;
    }
    return dummy->next;
  }
};

/*

Time Complexity: O(n)
- n = max(len(l1), len(l2))
Auxiliary Space Complexity: O(1)

#AdditionWithCarry

*/