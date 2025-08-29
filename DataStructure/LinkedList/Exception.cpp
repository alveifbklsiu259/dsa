#include "./Exception.hpp"
#include <optional>
#include <stdexcept>
#include <string>

LinkedList::EmptyListException::EmptyListException(
    std::optional<std::string> context)
    : std::out_of_range(context.has_value()
                            ? std::string(message) + ": " + std::move(*context)
                            : message) {}
