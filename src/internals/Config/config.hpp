#pragma once

#include <expected>
#include <map>
#include <string>

namespace config {
    enum class ParseError { FileNotFound, InvalidFormat };

    // Function declaration: The "Promise"
    // Returns a map on success or a ParseError on failure
    std::expected<std::map<std::string, std::string>, ParseError>
    load_env(const std::string &filename);
} // namespace config