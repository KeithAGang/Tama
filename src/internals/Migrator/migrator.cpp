#include "migrator.hpp"
#include <print>
#include <utility>
#include <chrono>
#include <fstream>

Migrator::Migrator(std::string migrationPath, std::string dbEngine, std::string migrationFileName)
    : migration_path(std::move(migrationPath)), db_engine(std::move(dbEngine)), migration_file_name(std::move(migrationFileName)) {}

Migrator::~Migrator() {
    std::println("Migrator destructor called");
}

void Migrator::generate_migration() {
    // 1. Get current time
    auto now = std::chrono::system_clock::now();

    // 2. Round it down to seconds (removes .123456 ms noise)
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);

    // 3. Format strictly as YYYYMMDDHHMMSS
    // The "{:%Y%m%d%H%M%S}" syntax is standard C++20/23
    std::string timestamp_str =std::format("{:%Y%m%d%H%M%S}", now_sec);

    {
        std::ofstream migration_file(migration_path + "/" + timestamp_str + "_" + migration_file_name +".sql");
        if (migration_file.is_open()) {
            migration_file << migration_file_template;
            migration_file.close();
        } else {
            std::println("Failed to create migration file");
        }
    }
}