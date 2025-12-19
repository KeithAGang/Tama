find_package(SQLite3 REQUIRED)
add_library(Migrator STATIC
        migrator.hpp
        migrator.cpp
)

target_include_directories(Migrator PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Migrator PRIVATE Db)
target_link_libraries(Migrator PRIVATE Parser)
target_link_libraries(Migrator PRIVATE SQLite::SQLite3)