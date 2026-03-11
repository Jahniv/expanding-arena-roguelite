#pragma once

#include <SDL3/SDL.h>
#include <vector>

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
    void render_status_bars();
    void render_game_over_overlay();
    void restart_game();
    void initialize_enemies();
    void update_window_title();

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool running_ = false;
    bool game_over_ = false;

    Player player_;
    std::vector<Enemy> enemies_;

    int wave_ = 1;
    int kills_total_ = 0;
    int kills_in_wave_ = 0;
    int score_ = 0;
    int active_enemy_count_ = 1;

    static constexpr int window_width_ = 1280;
    static constexpr int window_height_ = 720;
    static constexpr int kills_per_wave_ = 3;
};

} // namespace ear
