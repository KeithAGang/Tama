#pragma once

#include <string>
#include <string_view>
#include <set>

// FORWARD DECLARATION
// We tell the compiler "A struct named sqlite3 exists, trust me."
// This lets us use 'sqlite3*' pointers without including the heavy library here.
struct sqlite3;

class Ledger {
    private:
        sqlite3* db; // We hold a reference, but we do NOT own/close it (Migrator does).

    public:
        // Constructor takes an already open connection
        Ledger(sqlite3* database);

        // READ: returns a set of all version IDs found in the Db
        [[nodiscard]] std::set<std::string> get_applied_versions();

        // UPDATE: Inserts a new migration record
        void mark_version_as_applied(std::string_view version);

    private:
        // CREATE: Internal helper to make sure the table exists on startup
        void ensure_ledger_table_exists();
};