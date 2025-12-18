#include "handlers.hpp"
#include "../Config/config.hpp"
#include "../Migrator/migrator.hpp"
#include <print>
#include <cstdlib>
#include <string_view>

namespace {
    auto loadEnvHelper(const std::string &filename) {
        auto result = config::load_env(filename);

        if (result) {
            return *result;
        } else {
            std::println("Failed to load environment variables");
            std::exit(EXIT_FAILURE);
        }
    }
}

namespace commands {
    void handle_init(std::span<std::string_view> args) {
    // 1. Validate specific args for this command
    // We expect: [migration_name]
        if (args.empty()) {
            std::println("Error: 'init' requires a migration name.");
            std::println("Usage: tama init <migration_name>");
            std::exit(EXIT_FAILURE);
        }

        std::string_view migration_name = args[0];
        std::println("Creating sql migration: {}", migration_name);

    // 2. Load Env
        const auto& env = loadEnvHelper(".env");
        if (env.contains("TAMA_DB_MIGRATION_DIR") && env.contains("TAMA_DB_ENGINE")) {
            // Construct the Migrator
            // Note: converting string_view to string for the constructor if needed
            Migrator migrator(env.at("TAMA_DB_MIGRATION_DIR"), env.at("TAMA_DB_ENGINE"), std::string(migration_name));
            migrator.generate_migration();
        } else {
            std::println("Error: .env missing TAMA_DB_MIGRATION_DIR or TAMA_DB_ENGINE");
        }
    }

    void handle_help(std::span<std::string_view> args) {
    std::println("Available commands:");
    std::println("  init <migration_name>   Create a new migration");
    std::println("  up            Run pending migrations");
    }
}