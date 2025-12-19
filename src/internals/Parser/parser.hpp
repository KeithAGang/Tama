#pragma once
#include <string>
#include <string_view>

struct ParsedMigration {
    std::string up_sql;
    std::string down_sql;
};

class Parser {
public:
    static ParsedMigration parse(std::string_view raw_content);
};