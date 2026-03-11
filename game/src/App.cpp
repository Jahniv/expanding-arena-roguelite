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

        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_ESCAPE && !event.key.repeat) {
                running_ = false;
            }

            player_.on_key_down(event.key.key, event.key.repeat);
        }

        if (event.type == SDL_EVENT_KEY_UP) {
            player_.on_key_up(event.key.key);
        }
    }
}

void App::update(float dt_seconds) {
    player_.update(dt_seconds, window_width_, window_height_);
    enemy_.update(player_.center_x(), player_.center_y(), dt_seconds);

    if (player_.attack_hits(enemy_.bounds())) {
        enemy_.respawn();
    }
}

void App::render() {
    SDL_SetRenderDrawColor(renderer_, 20, 24, 32, 255);
    SDL_RenderClear(renderer_);

    enemy_.render(renderer_);
    player_.render(renderer_);

    SDL_RenderPresent(renderer_);
}

} // namespace ear
