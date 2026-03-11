#include "ear/App.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

namespace ear {
namespace {

bool rects_overlap(const SDL_FRect& a, const SDL_FRect& b) {
    return a.x < (b.x + b.w) &&
           (a.x + a.w) > b.x &&
           a.y < (b.y + b.h) &&
           (a.y + a.h) > b.y;
}

} // namespace

bool App::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return false;
    }

    window_ = SDL_CreateWindow(
        "Expanding Arena Roguelite",
        window_width_,
        window_height_,
        0
    );

    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);

    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    initialize_enemies();
    update_window_title();

    running_ = true;

    fmt::print("Game initialized successfully.\n");
    return true;
}

int App::run() {
    using clock = std::chrono::steady_clock;

    auto previous = clock::now();

    while (running_) {
        const auto current = clock::now();
        const std::chrono::duration<float> delta = current - previous;
        previous = current;

        handle_events();
        update(delta.count());
        render();

        SDL_Delay(1);
    }

    return 0;
}

void App::shutdown() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

void App::handle_events() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running_ = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE && !event.key.repeat) {
                running_ = false;
            }

            if (game_over_) {
                if (event.key.key == SDLK_R && !event.key.repeat) {
                    restart_game();
                }
            } else {
                player_.on_key_down(event.key.key, event.key.repeat);
            }
        }

        if (event.type == SDL_EVENT_KEY_UP && !game_over_) {
            player_.on_key_up(event.key.key);
        }
    }
}

void App::update(float dt_seconds) {
    if (game_over_) {
        return;
    }

    player_.update(dt_seconds, window_width_, window_height_);

    const float enemy_speed_multiplier = 1.0f + 0.12f * static_cast<float>(wave_ - 1);

    for (int i = 0; i < active_enemy_count_; ++i) {
        enemies_[i].update(player_.center_x(), player_.center_y(), dt_seconds, enemy_speed_multiplier);
    }

    for (int i = 0; i < active_enemy_count_; ++i) {
        if (player_.attack_hits(enemies_[i].bounds())) {
            enemies_[i].respawn();

            ++kills_total_;
            ++kills_in_wave_;
            score_ += 100;

            fmt::print("Kill {} | Score {} | Wave {}\n", kills_total_, score_, wave_);

            if (kills_in_wave_ >= kills_per_wave_) {
                ++wave_;
                kills_in_wave_ = 0;

                if (active_enemy_count_ < static_cast<int>(enemies_.size())) {
                    ++active_enemy_count_;
                }

                fmt::print("Wave {} started. Active enemies: {}\n", wave_, active_enemy_count_);
            }

            update_window_title();
        }
    }

    for (int i = 0; i < active_enemy_count_; ++i) {
        if (rects_overlap(player_.bounds(), enemies_[i].bounds())) {
            if (player_.try_take_damage(1)) {
                fmt::print("Player took damage. HP: {}\n", player_.hp());
                enemies_[i].respawn();

                if (player_.is_dead()) {
                    game_over_ = true;
                    fmt::print("Game Over. Press R to restart.\n");
                }

                update_window_title();
                break;
            }
        }
    }
}

void App::render() {
    SDL_SetRenderDrawColor(renderer_, 20, 24, 32, 255);
    SDL_RenderClear(renderer_);

    for (int i = 0; i < active_enemy_count_; ++i) {
        enemies_[i].render(renderer_);
    }

    player_.render(renderer_);
    render_hp_bar();
    render_status_bars();

    if (game_over_) {
        render_game_over_overlay();
    }

    SDL_RenderPresent(renderer_);
}

void App::render_hp_bar() {
    const float start_x = 24.0f;
    const float start_y = 24.0f;
    const float box_width = 34.0f;
    const float box_height = 18.0f;
    const float gap = 8.0f;

    for (int i = 0; i < player_.max_hp(); ++i) {
        const SDL_FRect rect{
            start_x + i * (box_width + gap),
            start_y,
            box_width,
            box_height
        };

        if (i < player_.hp()) {
            SDL_SetRenderDrawColor(renderer_, 210, 70, 70, 255);
        } else {
            SDL_SetRenderDrawColor(renderer_, 70, 70, 70, 255);
        }

        SDL_RenderFillRect(renderer_, &rect);
    }
}

void App::render_status_bars() {
    const float bar_x = 24.0f;
    const float attack_bar_y = 56.0f;
    const float dash_bar_y = 78.0f;
    const float bar_width = 180.0f;
    const float bar_height = 12.0f;

    const SDL_FRect attack_bg{ bar_x, attack_bar_y, bar_width, bar_height };
    const SDL_FRect dash_bg{ bar_x, dash_bar_y, bar_width, bar_height };

    SDL_SetRenderDrawColor(renderer_, 55, 55, 55, 255);
    SDL_RenderFillRect(renderer_, &attack_bg);
    SDL_RenderFillRect(renderer_, &dash_bg);

    const float attack_ready_ratio = 1.0f - player_.attack_cooldown_ratio();
    const float dash_ready_ratio = 1.0f - player_.dash_cooldown_ratio();

    const SDL_FRect attack_fill{
        bar_x,
        attack_bar_y,
        bar_width * std::clamp(attack_ready_ratio, 0.0f, 1.0f),
        bar_height
    };

    const SDL_FRect dash_fill{
        bar_x,
        dash_bar_y,
        bar_width * std::clamp(dash_ready_ratio, 0.0f, 1.0f),
        bar_height
    };

    SDL_SetRenderDrawColor(renderer_, 230, 200, 70, 255);
    SDL_RenderFillRect(renderer_, &attack_fill);

    SDL_SetRenderDrawColor(renderer_, 170, 120, 255, 255);
    SDL_RenderFillRect(renderer_, &dash_fill);
}

void App::render_game_over_overlay() {
    const SDL_FRect full_screen{
        0.0f,
        0.0f,
        static_cast<float>(window_width_),
        static_cast<float>(window_height_)
    };

    SDL_SetRenderDrawColor(renderer_, 120, 10, 10, 120);
    SDL_RenderFillRect(renderer_, &full_screen);

    const SDL_FRect center_panel{
        360.0f,
        260.0f,
        560.0f,
        200.0f
    };

    SDL_SetRenderDrawColor(renderer_, 25, 25, 25, 220);
    SDL_RenderFillRect(renderer_, &center_panel);
}

void App::restart_game() {
    player_.reset();

    wave_ = 1;
    kills_total_ = 0;
    kills_in_wave_ = 0;
    score_ = 0;
    active_enemy_count_ = 1;
    game_over_ = false;

    for (auto& enemy : enemies_) {
        enemy.respawn();
    }

    update_window_title();
    fmt::print("Game restarted.\n");
}

void App::initialize_enemies() {
    enemies_.clear();

    enemies_.emplace_back(920.0f, 180.0f);
    enemies_.emplace_back(1080.0f, 540.0f);
    enemies_.emplace_back(160.0f, 120.0f);
    enemies_.emplace_back(180.0f, 560.0f);
    enemies_.emplace_back(620.0f, 80.0f);
}

void App::update_window_title() {
    std::string title;

    if (game_over_) {
        title = fmt::format(
            "Expanding Arena Roguelite | GAME OVER | Wave {} | Kills {} | Score {} | Press R to Restart",
            wave_,
            kills_total_,
            score_
        );
    } else {
        title = fmt::format(
            "Expanding Arena Roguelite | HP {}/{} | Wave {} | Kills {} | Score {} | Enemies {}",
            player_.hp(),
            player_.max_hp(),
            wave_,
            kills_total_,
            score_,
            active_enemy_count_
        );
    }

    SDL_SetWindowTitle(window_, title.c_str());
}

} // namespace ear
