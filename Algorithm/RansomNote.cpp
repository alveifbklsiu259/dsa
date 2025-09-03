#include <string>
#include <unordered_map>

class RansomNote {
public:
  bool canConstruct(std::string ransomNote, std::string magazine) {
    if (magazine.length() < ransomNote.length())
      return false;
    std::unordered_map<char, int> magazineLetters;

    for (const char &c : magazine) {
      magazineLetters[c]++;
    }

    for (const char &c : ransomNote) {
      if (magazineLetters[c] == 0)
        return false;
      magazineLetters[c]--;
    }
    return true;
  }
};

/*

Time Complexity: O(n + m)
- n = characters of magazine
- m = characters of ransomNote

Auxiliary Space Complexity: O(k * log(n)) -> O(log(n))
- k = unique character (which is 26 in the case if lowercase alphabets)

#resourcePoolConsumption

*/