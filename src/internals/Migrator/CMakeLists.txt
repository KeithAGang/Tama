add_library(Migrator STATIC
        migrator.hpp
        migrator.cpp
)

target_include_directories(Migrator PUBLIC ${CMAKE_CURRENT_LIST_DIR})