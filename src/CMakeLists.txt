find_package(SQLite3 REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Migrator)
target_link_libraries(${PROJECT_NAME} PRIVATE Config)
# target_link_libraries(${PROJECT_NAME} PRIVATE Db)
target_link_libraries(${PROJECT_NAME} PRIVATE SQLite::SQLite3)