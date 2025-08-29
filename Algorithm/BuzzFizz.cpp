#include <string>
#include <vector>

class BuzzFeed {
public:
  std::vector<std::string> run(int n) {
    std::vector<std::string> result;
    result.reserve(n);
    for (int i = 1; i <= n; i++) {
      if (i % 15 == 0) {
        result.push_back("FizzBuzz");
      } else if (i % 3 == 0) {
        result.emplace_back("Fizz");
      } else if (i % 5 == 0) {
        result.emplace_back("Buzz");
      } else {
        result.emplace_back(std::to_string(i));
      };
    };
    return result;
  }
};

/*

Time Complexity: O(n)
Auxiliary Space Complexity: O(1)

#naive

*/