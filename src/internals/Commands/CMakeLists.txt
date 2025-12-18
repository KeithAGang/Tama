add_library(Commands STATIC
        handlers.hpp
        handlers.cpp
)

target_include_directories(Commands PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Commands PRIVATE Config Migrator)
