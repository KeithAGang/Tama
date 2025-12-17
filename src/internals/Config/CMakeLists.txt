add_library(Config STATIC
        config.hpp
        config.cpp
)

target_include_directories(Config PUBLIC ${CMAKE_CURRENT_LIST_DIR})