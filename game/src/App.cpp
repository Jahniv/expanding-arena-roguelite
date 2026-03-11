#include "ear/App.hpp"

#include <fmt/core.h>
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
        1280,
        720,
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

        SDL_Delay(16);
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
    }
}

void App::update(float dt_seconds) {
    player_x_ += static_cast<float>(direction_) * player_speed_ * dt_seconds;

    if (player_x_ <= 40.0f) {
        player_x_ = 40.0f;
        direction_ = 1;
    }

    if (player_x_ >= 1190.0f) {
        player_x_ = 1190.0f;
        direction_ = -1;
    }
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
