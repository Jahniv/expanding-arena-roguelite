include(FetchContent)

find_package(fmt CONFIG REQUIRED)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.2
)

FetchContent_MakeAvailable(SDL3)

add_library(ear_game_lib STATIC
    src/App.cpp
    src/Player.cpp
    src/Enemy.cpp
)

target_include_directories(ear_game_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(ear_game_lib PUBLIC cxx_std_20)

target_link_libraries(ear_game_lib
    PUBLIC
        SDL3::SDL3
        fmt::fmt
)

add_executable(ear_game
    src/main.cpp
)

target_link_libraries(ear_game
    PRIVATE
        ear_game_lib
)
