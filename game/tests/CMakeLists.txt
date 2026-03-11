find_package(Catch2 3 CONFIG REQUIRED)

add_executable(ear_tests
    smoke_test.cpp
)

target_link_libraries(ear_tests
    PRIVATE
        ear_game_lib
        Catch2::Catch2WithMain
)

add_test(NAME ear_tests COMMAND ear_tests)
