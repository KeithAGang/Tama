#include "config.hpp"
#include <expected>
#include <fstream>
#include <string>
#include <string_view>

namespace config {

    // internal helper for string trimming (kept private to this file)
    static std::string_view trim(std::string_view str) {
        auto start = str.find_first_not_of(" \t\r");
        if (start == std::string_view::npos) return {};
        auto end = str.find_last_not_of(" \t\r");
        return str.substr(start, end - start + 1);
    }

    std::expected<std::map<std::string, std::string>, ParseError> load_env(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return std::unexpected(ParseError::FileNotFound);
        }

        std::map<std::string, std::string> env_map;
        std::string line;

        while (std::getline(file, line)) {
            std::string_view sv = line;
            sv = trim(sv);
            if (sv.empty() || sv.starts_with('#')) continue;

            auto pos = sv.find('=');
            if (pos == std::string_view::npos) continue;

            auto key = trim(sv.substr(0, pos));
            auto value = trim(sv.substr(pos + 1));

            if (!key.empty()) {
                env_map[std::string(key)] = std::string(value);
            }
        }

        return env_map;;
    }
}