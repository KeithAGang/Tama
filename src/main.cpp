#include "handlers.hpp"
#include <map>
#include <functional>
#include <print>
#include <cstdlib>
#include <span>
#include <string_view>
#include <vector>
#include "internals/Commands/handlers.hpp"

int main(int argc, char *argv[]) {
    // 1. Wrap the raw C-array in a std::span
    // This creates a safe "view" over the arguments
    std::span<char*> args_view{argv, static_cast<size_t>(argc)};

    // 2. Convert to a vector of string_views for easier handling (Optional but nice)
    // We skip the first argument (args_view[0]) because it's just the program name.
    std::vector<std::string_view> args;
    if (args_view.size() > 1) {
        for (auto arg : args_view.subspan(1)) {
            args.emplace_back(arg);
        }
    }

    // 3. Simple Router Logic
    if (args.empty()) {
        std::println("Error: No command provided.");
        commands::handle_help({});
        return 1;
    }

    // 3. The Dispatch Table
    // Maps a string ("init") to a function (commands::handle_init)
    // using std::string_view as the key is efficient (no copies)
    const std::map<std::string_view, std::function<void(std::span<std::string_view>)>> dispatch_table {
        { "init", commands::handle_init },
        { "help", commands::handle_help },
        // { "up",   commands::handle_up }, // TODO Add this later
    };

    // 4. Router Logic
    std::string_view command = args[0];

    if (dispatch_table.contains(command)) {
        // Pass the *rest* of the args (args[1] onwards) to the handler
        // we use std::span to create a view of the vector without copying
        std::span<std::string_view> cmd_args(args.begin() + 1, args.end());
        
        // Execute!
        dispatch_table.at(command)(cmd_args);
    } else {
        std::println("Unknown command: '{}'", command);
        commands::handle_help({});
        return 1;
    }

    return 0;
}