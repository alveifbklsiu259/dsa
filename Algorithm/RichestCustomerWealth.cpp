#include <algorithm>
#include <numeric>
#include <vector>

class RichestCustomerWealth {
public:
  int run(const std::vector<std::vector<int>> &accounts) {
    int max = 0;
    for (const std::vector<int> &personAccounts : accounts) {
      // clang-format off
      int personWealth = std::accumulate(personAccounts.begin(), personAccounts.end(), 0);
      max = std::max(max, personWealth);
    }
    return max;
  }
};

/*

Time Complexity: O(n * m)
  - n = accounts.length
  - m = accounts[i].length
Auxiliary Space Complexity: O(1)

#reduction

*/