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

UpgradeChoice make_choice(UpgradeType type) {
    switch (type) {
        case UpgradeType::MoveSpeed:
            return UpgradeChoice{ type, "Move Speed +40", SDL_Color{ 70, 170, 255, 255 } };
        case UpgradeType::MaxHp:
            return UpgradeChoice{ type, "Max HP +1", SDL_Color{ 220, 80, 80, 255 } };
        case UpgradeType::DashCooldown:
            return UpgradeChoice{ type, "Dash Cooldown -15%", SDL_Color{ 170, 120, 255, 255 } };
        case UpgradeType::DashSpeed:
            return UpgradeChoice{ type, "Dash Speed +120", SDL_Color{ 200, 150, 255, 255 } };
        case UpgradeType::AttackSize:
            return UpgradeChoice{ type, "Attack Size +10", SDL_Color{ 235, 205, 90, 255 } };
        case UpgradeType::ScoreBonus:
            return UpgradeChoice{ type, "Score Bonus +10%", SDL_Color{ 80, 220, 170, 255 } };
    }

    return UpgradeChoice{};
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
            } else if (choosing_upgrade_) {
                if (!event.key.repeat) {
                    if (event.key.key == SDLK_1) {
                        apply_upgrade_by_index(0);
                    } else if (event.key.key == SDLK_2) {
                        apply_upgrade_by_index(1);
                    } else if (event.key.key == SDLK_3) {
                        apply_upgrade_by_index(2);
                    }
                }
            } else {
                player_.on_key_down(event.key.key, event.key.repeat);
            }
        }

        if (event.type == SDL_EVENT_KEY_UP && !game_over_ && !choosing_upgrade_) {
            player_.on_key_up(event.key.key);
        }
    }
}

void App::update(float dt_seconds) {
    if (game_over_ || choosing_upgrade_) {
        return;
    }

    player_.update(dt_seconds, window_width_, window_height_);

    const float enemy_speed_multiplier = std::min(1.0f + 0.06f * static_cast<float>(wave_ - 1), 1.45f);

    for (int i = 0; i < active_enemy_count_; ++i) {
        enemies_[i].update(player_.center_x(), player_.center_y(), dt_seconds, enemy_speed_multiplier);
    }

    for (int i = 0; i < active_enemy_count_; ++i) {
        if (player_.attack_hits(enemies_[i].bounds())) {
            const int base_score = enemies_[i].score_value();
            const int gained_score = base_score + (base_score * score_bonus_percent_) / 100;
            enemies_[i].respawn();

            ++kills_total_;
            ++kills_in_wave_;
            score_ += gained_score;

            fmt::print("Kill {} | +{} score | Total {} | Wave {}\n", kills_total_, gained_score, score_, wave_);

            if (kills_in_wave_ >= kills_per_wave_) {
                start_upgrade_selection();
                update_window_title();
                return;
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

    if (choosing_upgrade_) {
        render_upgrade_overlay();
    }

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

void App::render_upgrade_overlay() {
    const SDL_FRect full_screen{
        0.0f,
        0.0f,
        static_cast<float>(window_width_),
        static_cast<float>(window_height_)
    };

    SDL_SetRenderDrawColor(renderer_, 15, 15, 15, 150);
    SDL_RenderFillRect(renderer_, &full_screen);

    const float panel_w = 260.0f;
    const float panel_h = 180.0f;
    const float gap = 40.0f;
    const float total_w = panel_w * 3.0f + gap * 2.0f;
    const float start_x = (window_width_ - total_w) * 0.5f;
    const float y = 250.0f;

    for (int i = 0; i < 3; ++i) {
        const float x = start_x + i * (panel_w + gap);
        const SDL_FRect panel{ x, y, panel_w, panel_h };

        const SDL_Color c = current_upgrade_choices_[i].color;
        SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, 220);
        SDL_RenderFillRect(renderer_, &panel);

        const SDL_FRect inner{
            x + 8.0f,
            y + 8.0f,
            panel_w - 16.0f,
            panel_h - 16.0f
        };

        SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 120);
        SDL_RenderFillRect(renderer_, &inner);
    }
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
    score_bonus_percent_ = 0;
    upgrade_roll_counter_ = 0;
    choosing_upgrade_ = false;
    game_over_ = false;

    for (auto& enemy : enemies_) {
        enemy.respawn();
    }

    update_window_title();
    fmt::print("Game restarted.\n");
}

void App::initialize_enemies() {
    enemies_.clear();

    enemies_.emplace_back(920.0f, 180.0f, EnemyType::Chaser);
    enemies_.emplace_back(1080.0f, 540.0f, EnemyType::Chaser);
    enemies_.emplace_back(160.0f, 120.0f, EnemyType::Brute);
    enemies_.emplace_back(180.0f, 560.0f, EnemyType::Chaser);
    enemies_.emplace_back(620.0f, 80.0f, EnemyType::Brute);
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
    } else if (choosing_upgrade_) {
        title = fmt::format(
            "Choose Upgrade: [1] {} | [2] {} | [3] {}",
            current_upgrade_choices_[0].name,
            current_upgrade_choices_[1].name,
            current_upgrade_choices_[2].name
        );
    } else {
        title = fmt::format(
            "Expanding Arena Roguelite | HP {}/{} | Wave {} | Kills {} | Score {} | Enemies {} | Bonus {}%",
            player_.hp(),
            player_.max_hp(),
            wave_,
            kills_total_,
            score_,
            active_enemy_count_,
            score_bonus_percent_
        );
    }

    SDL_SetWindowTitle(window_, title.c_str());
}

void App::start_upgrade_selection() {
    choosing_upgrade_ = true;
    roll_upgrade_choices();

    fmt::print("\nWave {} cleared. Choose an upgrade:\n", wave_);
    fmt::print("  [1] {}\n", upgrade_description(current_upgrade_choices_[0]));
    fmt::print("  [2] {}\n", upgrade_description(current_upgrade_choices_[1]));
    fmt::print("  [3] {}\n\n", upgrade_description(current_upgrade_choices_[2]));
}

void App::apply_upgrade_by_index(int index) {
    if (index < 0 || index >= 3 || !choosing_upgrade_) {
        return;
    }

    apply_upgrade(current_upgrade_choices_[index]);

    ++wave_;
    kills_in_wave_ = 0;

    if (active_enemy_count_ < static_cast<int>(enemies_.size())) {
        ++active_enemy_count_;
    }

    choosing_upgrade_ = false;

    fmt::print("Wave {} started. Active enemies: {}\n", wave_, active_enemy_count_);
    update_window_title();
}

void App::roll_upgrade_choices() {
    static constexpr std::array<UpgradeType, 6> pool = {
        UpgradeType::MoveSpeed,
        UpgradeType::MaxHp,
        UpgradeType::DashCooldown,
        UpgradeType::DashSpeed,
        UpgradeType::AttackSize,
        UpgradeType::ScoreBonus
    };

    const int pool_size = static_cast<int>(pool.size());

    for (int i = 0; i < 3; ++i) {
        const int idx = (upgrade_roll_counter_ + i) % pool_size;
        current_upgrade_choices_[i] = make_choice(pool[idx]);
    }

    upgrade_roll_counter_ = (upgrade_roll_counter_ + 3) % pool_size;
}

void App::apply_upgrade(const UpgradeChoice& choice) {
    switch (choice.type) {
        case UpgradeType::MoveSpeed:
            player_.increase_move_speed(40.0f);
            break;
        case UpgradeType::MaxHp:
            player_.increase_max_hp(1);
            break;
        case UpgradeType::DashCooldown:
            player_.reduce_dash_cooldown_multiplier(0.85f);
            break;
        case UpgradeType::DashSpeed:
            player_.increase_dash_speed(120.0f);
            break;
        case UpgradeType::AttackSize:
            player_.increase_attack_size(10.0f);
            break;
        case UpgradeType::ScoreBonus:
            score_bonus_percent_ += 10;
            break;
    }

    fmt::print("Applied upgrade: {}\n", upgrade_description(choice));
}

std::string App::upgrade_description(const UpgradeChoice& choice) const {
    switch (choice.type) {
        case UpgradeType::MoveSpeed:
            return fmt::format("{} (current move speed: {:.0f})", choice.name, player_.move_speed());
        case UpgradeType::MaxHp:
            return fmt::format("{} (current HP: {}/{})", choice.name, player_.hp(), player_.max_hp());
        case UpgradeType::DashCooldown:
            return fmt::format("{} (current dash cooldown: {:.2f})", choice.name, player_.dash_cooldown_seconds());
        case UpgradeType::DashSpeed:
            return fmt::format("{} (current dash speed: {:.0f})", choice.name, player_.dash_speed());
        case UpgradeType::AttackSize:
            return fmt::format("{} (current attack size: {:.0f})", choice.name, player_.attack_size());
        case UpgradeType::ScoreBonus:
            return fmt::format("{} (current score bonus: {}%)", choice.name, score_bonus_percent_);
    }

    return "Unknown upgrade";
}

} // namespace ear
