#include <print>
#include <cstdlib>
#include <span>
#include <string_view>
#include <vector>
#include "internals/Config/config.hpp"
#include "internals/Migrator/migrator.hpp"

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
        std::println("Usage: tama init migration <migration_name>");
        return 1;
    }
    std::string migration_name = "";
    if (args[0] == "init") {
        if (args.size() < 3) {
            std::println("Error: Missing arguments");
        } else {
            migration_name = args[2];

            auto result = config::load_env(".env");

            if (result) {
                const auto env = *result;

                if (env.contains("TAMA_DB_MIGRATION_DIR") && env.contains("TAMA_DB_ENGINE")) {
                    Migrator migrator(env.at("TAMA_DB_MIGRATION_DIR"), env.at("TAMA_DB_ENGINE"), migration_name);
                    migrator.generate_migration();
                }
            } else {
                std::println("Failed to load environment variables");
            }
        }
    }

    
    
}