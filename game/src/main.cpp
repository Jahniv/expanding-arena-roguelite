#include <SDL3/SDL_main.h>
#include "ear/App.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    ear::App app;

    if (!app.initialize()) {
        return 1;
    }

    const int result = app.run();
    app.shutdown();

    return result;
}
