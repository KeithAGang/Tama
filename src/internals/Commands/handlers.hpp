#pragma once
#include <span>
#include <string_view>

namespace commands {
    // The signature for any CLI command function
    // It takes the list of args *after* the command name itself
    void handle_init(std::span<std::string_view> args);
    void handle_up(std::span<std::string_view> args);
    void handle_help(std::span<std::string_view> args);
}