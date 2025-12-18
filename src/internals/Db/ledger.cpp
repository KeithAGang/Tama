#include "ledger.hpp"

// The only place we include the heavy C-API
#include <sqlite3.h>

#include <print>
#include <format>

// Constructor
Ledger::Ledger(sqlite3* db_conn) : db(std::move(db_conn)) {
    if (!db) {
        // Safety check: The program usually crashes if we use a null db pointer
        std::println(stderr, "Critical Error: Ledger initialized with null DB connection!");
    } else {
        ensure_ledger_table_exists();
    }
}

// READ: Get Applied Versions
std::set<std::string> Ledger::get_applied_versions() {
    std::set<std::string> versions;
    const char* sql = "SELECT version FROM tama_schema_history;";
    sqlite3_stmt* stmt = nullptr;

    // 1. Prepare (Compile SQL)
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::println(stderr, "Ledger Read Error: {}", sqlite3_errmsg(db));
        return {}; // Return empty set on failure
    }

    // 2. Step (Execute Row-by-Row)
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Column 0 is 'version'. sqlite3_column_text returns unsigned char*, so we cast.
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text) {
            versions.insert(reinterpret_cast<const char*>(text));
        }
    }

    // 3. Finalize (Cleanup Statement memory)
    sqlite3_finalize(stmt);
    
    return versions;
}

// UPDATE: Mark Version as Applied
void Ledger::mark_version_as_applied(std::string_view version) {
    // We use ? placeholders to be safe, even internally
    const char* sql = "INSERT INTO tama_schema_history (version, applied_at) VALUES (?, datetime('now'))";
    sqlite3_stmt* stmt = nullptr;

    // 1. Prepare
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::println(stderr, "Ledger Insert Error: {}", sqlite3_errmsg(db));
        return;
    }

    // 2. Bind Parameters (Replace '?' with the version string)
    // Index 1 is the first '?'
    // SQLITE_STATIC tells SQLite: "I promise this string exists until you are done using it"
    sqlite3_bind_text(stmt, 1, version.data(), -1, SQLITE_STATIC);

    // 3. Step (Run the Insert)
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::println(stderr, "Failed to record version {}: {}", version, sqlite3_errmsg(db));
    }

    // 4. Finalize
    sqlite3_finalize(stmt);
}

/* 
   CREATE: Initialize Table
   
*/
void Ledger::ensure_ledger_table_exists() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS tama_schema_history (
            version TEXT PRIMARY KEY,
            applied_at TEXT
        );
    )";

    char* errMsg = nullptr;
    // sqlite3_exec is a shortcut for Prepare -> Step -> Finalize when you don't need results
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::println(stderr, "Ledger Init Failed: {}", errMsg ? errMsg : "Unknown error");
        sqlite3_free(errMsg); // We must manually free the error message memory
    }
}