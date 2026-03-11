#pragma once

#include <SDL3/SDL.h>

namespace ear {

class App {
public:
    bool initialize();
    int run();
    void shutdown();

private:
    void handle_events();
    void update(float dt_seconds);
    void render();

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool running_ = false;

    float player_x_ = 120.0f;
    float player_y_ = 320.0f;
    float player_size_ = 50.0f;
    float player_speed_ = 220.0f;
    int direction_ = 1;
};

} // namespace ear
