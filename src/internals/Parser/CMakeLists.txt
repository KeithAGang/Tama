add_library(Parser STATIC
        parser.hpp
        parser.cpp
)

target_include_directories(Parser PUBLIC ${CMAKE_CURRENT_LIST_DIR})
