#include "migrator.hpp"
#include "ledger.hpp"
#include <sqlite3.h>
#include <print>
#include <utility>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

Migrator::Migrator(std::string migrationPath, std::string dbConnStr, std::string dbEngine)
    : migration_path(std::move(migrationPath)),
      db_conn_str(std::move(dbConnStr)),
      db_engine(std::move(dbEngine))
{
    // 1. Open SQLite Database
    std::println("DEBUG: Attempting to create DB at: [{}]", dbConnStr);
    std::string db_file = dbConnStr; 
    
    if (sqlite3_open(db_file.c_str(), &db) != SQLITE_OK) {
        // In C++23, it is safe to throw or handle error gracefully
        std::string err = db ? sqlite3_errmsg(db) : "Memory allocation failed";
        throw std::runtime_error("Failed to open DB: " + err);
    }

    // 2. Connect the Ledger
    // Now that 'db' is valid, we reconstruct the ledger with it.
    // The Ledger constructor automatically runs "ensure_table_exists()"
    ledger.emplace(db);
}

Migrator::~Migrator() {
    std::println("Migrator destructor called");
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

void Migrator::scan_and_print_migrations() {
    std::println("Scanning directory: {}", migration_path);

    // 1. Check if directory exists
    if (!fs::exists(migration_path)) {
        fs::create_directories(migration_path); // Be helpful and create it
    }

    // 2. Collect files
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(migration_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sql") {
            files.push_back(entry.path().filename().string());
        }
    }

    // 3. Sort (Crucial for migrations!)
    // We sort lexicographically (by string). Since we use timestamps 
    // (YYYYMMDD...), they will sort chronologically automatically.
    std::ranges::sort(files);

    // 4. Print
    if (files.empty()) {
        std::println("No migration files found.");
    } else {
        std::println("Found {} migrations:", files.size());
        for (const auto& f : files) {
            std::println(" - {}", f);
        }
    }
}

void Migrator::generate_migration(std::string_view name) {
    // C++23 Timestamp Logic (from previous steps)
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    std::string timestamp = std::format("{:%Y%m%d%H%M%S}", now_sec);

    // Construct filename: 20251218120000_name.sql
    std::string filename = std::format("{}/{}_{}.sql", migration_path, timestamp, name);

    // Write file
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << migration_file_template;
        outfile.close();
        std::println("Created migration: {}", filename);
    } else {
        std::println(stderr, "Error: Could not create file at {}", filename);
    }
}