class NumberOfSteps {
public:
  int run(int num) {
    int steps = 0;
    while (num != 0) {
      if (num % 2 == 0) {
        num /= 2;
      } else {
        num -= 1;
      }
      steps += 1;
    }
    return steps;
  }
};

/*

Time Complexity: O(n)
Auxiliary Space Complexity: O(1)

#dynamic

*/