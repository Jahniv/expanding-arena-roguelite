#pragma once

#include <SDL3/SDL.h>
#include "ear/Enemy.hpp"
#include "ear/Player.hpp"

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
    void render_hp_bar();
    void render_game_over_overlay();
    void restart_game();

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool running_ = false;
    bool game_over_ = false;

    Player player_;
    Enemy enemy_;

    static constexpr int window_width_ = 1280;
    static constexpr int window_height_ = 720;
};

} // namespace ear
