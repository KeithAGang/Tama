#pragma once

#include <string>
#include <optional>
#include "../Db/ledger.hpp"

// Forward declaration (avoids including <sqlite3.h> here)
struct sqlite3;

class Migrator {
public:
    // Constructor now establishes the DB connection
    Migrator(std::string migrationPath, std::string dbConnStr, std::string dbEngine);
    
    // Destructor closes the DB
    ~Migrator();

    // 1. Create a new file (init command)
    void generate_migration(std::string_view name);

    // 2. Scan and print files (The new requirement)
    void scan_and_print_migrations();

private:
    std::string migration_path;
    std::string db_conn_str;
    std::string db_engine;

    // DB Resources
    sqlite3* db = nullptr; // Migrator owns this
    std::optional<Ledger> ledger;         // Migrator owns the instance (which borrows the ptr)

    const std::string migration_file_template = R"(-- +tama up
SELECT 'up SQL query';

-- +tama down
SELECT 'down SQL query';)";
};