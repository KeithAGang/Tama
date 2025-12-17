#pragma once

#include <string>

class Migrator {
    public:
        Migrator(std::string migrationPath, std::string dbEngine, std::string migrationFileName);
        ~Migrator();

        void generate_migration();

    private:
        std::string migration_path;
        std::string db_engine;
        std::string migration_file_name;


        const std::string migration_file_template = R"(-- +tama up
SELECT 'up SQL query';

-- +tama down
SELECT 'down SQL query';)";
};