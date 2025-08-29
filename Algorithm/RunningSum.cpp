#include <vector>

class RunningSum {
public:
  std::vector<int> run(const std::vector<int> &nums) {
    std::vector<int> result;
    result.reserve(nums.size());
    int sum = 0;
    for (const int &num : nums) {
      sum += num;
      result.emplace_back(sum);
    }
    return result;
  }
};

/*

Time Complexity: O(n)
Auxiliary Space Complexity: O(1)

#prefixSum #RunningTotal

*/