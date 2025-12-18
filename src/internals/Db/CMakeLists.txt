add_library(Db STATIC
        ledger.hpp
        ledger.cpp
)

target_include_directories(Db PUBLIC ${CMAKE_CURRENT_LIST_DIR})
find_package(SQLite3 REQUIRED)
target_link_libraries(Db PRIVATE SQLite::SQLite3)

# target_link_libraries(DB PRIVATE Config Migrator)
