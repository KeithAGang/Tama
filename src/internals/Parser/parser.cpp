#include "parser.hpp"
#include <cstddef>
#include <string_view>
#include <vector>
#include <algorithm>

ParsedMigration Parser::parse(std::string_view raw_content) {
    ParsedMigration result;

    // Define the markers we are looking for
    constexpr std::string_view up_marker = "-- +tama up";
    constexpr std::string_view down_marker = "-- +tama down";

    // 1. Find the UP section
    size_t up_pos = raw_content.find(up_marker);
    size_t down_pos = raw_content.find(down_marker);

    // If no UP marker found, return empty result
    if (up_pos == std::string_view::npos) {
        return result;
    }

    // Start reading *after* the UP marker
    size_t up_start = up_pos + up_marker.size();

    // The UP section ends at the DOWN marker 
    size_t up_end = (down_pos == std::string_view::npos) ? raw_content.size() : down_pos;

    // Extract UP block
    result.up_sql = raw_content.substr(up_start, up_end - up_start);

    // 2. Find the DOWN section (if it exists)
    if (down_pos != std::string_view::npos) {
        size_t down_start = down_pos + down_marker.size();
        result.down_sql = raw_content.substr(down_start);
    }

    return result;
}
