#include "migrator.hpp"
#include "parser.hpp"
#include <sqlite3.h>
#include <print>
#include <sstream>
#include <utility>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <ranges>

namespace fs = std::filesystem;

Migrator::Migrator(std::string migrationPath, std::string dbConnStr, std::string dbEngine)
    : migration_path(std::move(migrationPath)),
      db_conn_str(std::move(dbConnStr)),
      db_engine(std::move(dbEngine))
{
    // 1. Open SQLite Database
    std::println("DEBUG: Attempting to create DB at: [{}]", db_conn_str);
    std::string db_file = db_conn_str; 
    
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

// Helper: Read File
std::string Migrator::read_file_content(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (!in) {
        std::println(stderr, "Error: Could not read file {}", filepath);
        return "";
    }

    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

// Helper: Execute
bool Migrator::execute_sql(std::string_view sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.data(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::println(stderr, "SQL Error: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// The UP LOGIC
void Migrator::up() {
    std::println("Checking for pending migrations...");

    // A. Get history from Ledger
    // Note: ledger is std::optional, so use '->'
    auto applied_versions = ledger->get_applied_versions();

    // B. Scan files
    std::vector<std::string> files;
    for (const auto& entry: fs::directory_iterator(migration_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sql") {
            files.push_back(entry.path().filename().string());
        }
    }

    std::ranges::sort(files); // ensure chronological order

    // C. Iterate and apply
    int count = 0;
    for (const auto& filename : files) {
        // Extract Version (assumes format: 20251218xxxxx_name.sql)
        // We take the substring before the first '_'
        std::string version = filename.substr(0, filename.find('_'));

        // SKIP if already applied
        if (applied_versions.find(version) != applied_versions.end()) {
            continue;
        }

        std::println("Applying: {}", filename);

        // 1. Read & Parse
        std::string full_path = migration_path + "/" + filename;
        std::string content = read_file_content(full_path);
        ParsedMigration parsed = Parser::parse(content);

        if (parsed.up_sql.empty()) {
            std::println("Warning: No UP block found in {}", filename);
            continue;
        }

        // 2. BEGIN TRANSACTION
        // This is crucial. If the script fails halfway, we want to undo it.
        execute_sql("BEGIN TRANSACTION;");

        // 3. Run the user's SQL
        if (!execute_sql(parsed.up_sql)) {
            std::println(stderr, "Migration failed! Rolling back...");
            execute_sql("ROLLBACK;");
            return; // Stop everything
        }

        // 4. Update Ledger
        ledger->mark_version_as_applied(version);

        // 5. COMMIT
        // If we got here, both the SQL and the Ledger update are pending.
        // This saves them both to disk at the exact same time.
        if (execute_sql("COMMIT;")) {
            std::println("Success: {}", filename);
            count++;
        } else {
             std::println(stderr, "Commit failed! Rolling back...");
             execute_sql("ROLLBACK;");
             return;
        }
    }

    if (count == 0) {
        std::println("Database is up to date.");
    } else {
        std::println("Applied {} migrations.", count);
    }
    
}

// The DROP LOGIC
void Migrator::down(int steps) {

    if (steps == -1) {
        std::println("Reverting ALL migrations (Reset)...");
    } else {
        std::println("Reverting last {} migration(s)...", steps);
    }

    // A. Get history from Ledger
    // Note: ledger is std::optional, so use '->'
    auto applied_versions = ledger->get_applied_versions();

    // B. Scan files
    std::vector<std::string> files;
    for (const auto& entry: fs::directory_iterator(migration_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sql") {
            files.push_back(entry.path().filename().string());
        }
    }

    std::ranges::sort(files); // ensure chronological order

    // C. Iterate and apply
    int count = 0;
    for (const auto& filename : files | std::views::reverse) {

        // 1. CHECK LIMIT
        // If we aren't in "Reset Mode" (-1) and we hit our limit, STOP.
        if (steps != -1 && count >= steps) {
            break;
        }
        
        // Extract Version (assumes format: 20251218xxxxx_name.sql)
        // We take the substring before the first '_'
        std::string version = filename.substr(0, filename.find('_'));

        // SKIP if not in ledger
        if (applied_versions.find(version) == applied_versions.end()) {
            continue;
        }

        std::println("Dropping: {}", filename);

        // 1. Read & Parse
        std::string full_path = migration_path + "/" + filename;
        std::string content = read_file_content(full_path);
        ParsedMigration parsed = Parser::parse(content);

        if (parsed.down_sql.empty()) {
            std::println("Warning: No DOWN block found in {}", filename);
            continue;
        }

        std::println("Executing DOWN SQL: {}", parsed.down_sql);

        // 2. BEGIN TRANSACTION
        // This is crucial. If the script fails halfway, we want to undo it.
        execute_sql("BEGIN TRANSACTION;");

        // 3. Run the user's SQL
        if (!execute_sql(parsed.down_sql)) {
            std::println(stderr, "Migration Drop failed! Rolling back...");
            execute_sql("ROLLBACK;");
            return; // Stop everything
        }

        // 4. Update Ledger
        ledger->remove_version(version);

        // 5. COMMIT
        // If we got here, both the SQL and the Ledger update are pending.
        // This saves them both to disk at the exact same time.
        if (execute_sql("COMMIT;")) {
            std::println("Success: {}", filename);
            count++;
        } else {
             std::println(stderr, "Commit failed! Rolling back...");
             execute_sql("ROLLBACK;");
             return;
        }
    }

    if (count == 0) {
        std::println("Database is up to date.");
    } else {
        std::println("Dropped {} migrations.", count);
    }
    
}