#pragma once

#include <optional>
#include <stdexcept>
#include <string>

namespace linkedlist {
class EmptyListException : public std::out_of_range {
public:
  static constexpr const char* message = "List is empty";

  EmptyListException(std::optional<std::string> context = std::nullopt);
};
} // namespace linkedlist
