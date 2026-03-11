cmake_minimum_required(VERSION 3.25)

project(ExpandingArenaRoguelite VERSION 0.1.0 LANGUAGES CXX)

option(EAR_BUILD_TESTS "Build tests" ON)

add_subdirectory(game)

if(EAR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(game/tests)
endif()
