#include "ear/App.hpp"

#include <fmt/core.h>
#include <algorithm>
#include <chrono>
#include <iostream>

namespace ear {

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

        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            switch (event.key.key) {
                case SDLK_ESCAPE:
                    running_ = false;
                    break;
                case SDLK_W:
                case SDLK_UP:
                    move_up_ = true;
                    break;
                case SDLK_S:
                case SDLK_DOWN:
                    move_down_ = true;
                    break;
                case SDLK_A:
                case SDLK_LEFT:
                    move_left_ = true;
                    break;
                case SDLK_D:
                case SDLK_RIGHT:
                    move_right_ = true;
                    break;
                default:
                    break;
            }
        }

        if (event.type == SDL_EVENT_KEY_UP) {
            switch (event.key.key) {
                case SDLK_W:
                case SDLK_UP:
                    move_up_ = false;
                    break;
                case SDLK_S:
                case SDLK_DOWN:
                    move_down_ = false;
                    break;
                case SDLK_A:
                case SDLK_LEFT:
                    move_left_ = false;
                    break;
                case SDLK_D:
                case SDLK_RIGHT:
                    move_right_ = false;
                    break;
                default:
                    break;
            }
        }
    }
}

void App::update(float dt_seconds) {
    float dx = 0.0f;
    float dy = 0.0f;

    if (move_up_) {
        dy -= 1.0f;
    }
    if (move_down_) {
        dy += 1.0f;
    }
    if (move_left_) {
        dx -= 1.0f;
    }
    if (move_right_) {
        dx += 1.0f;
    }

    player_x_ += dx * player_speed_ * dt_seconds;
    player_y_ += dy * player_speed_ * dt_seconds;

    player_x_ = std::clamp(player_x_, 0.0f, static_cast<float>(window_width_) - player_size_);
    player_y_ = std::clamp(player_y_, 0.0f, static_cast<float>(window_height_) - player_size_);
}

void App::render() {
    SDL_SetRenderDrawColor(renderer_, 20, 24, 32, 255);
    SDL_RenderClear(renderer_);

    const SDL_FRect player_rect{
        player_x_,
        player_y_,
        player_size_,
        player_size_
    };

    SDL_SetRenderDrawColor(renderer_, 80, 200, 120, 255);
    SDL_RenderFillRect(renderer_, &player_rect);

    SDL_RenderPresent(renderer_);
}

} // namespace ear
