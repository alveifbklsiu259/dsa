#pragma once

#include <optional>
#include <stdexcept>
#include <string>

namespace LinkedList {
class EmptyListException : public std::out_of_range {
public:
  static inline constexpr const char *message = "List is empty";

  EmptyListException(std::optional<std::string> context = std::nullopt);
};
} // namespace LinkedList
