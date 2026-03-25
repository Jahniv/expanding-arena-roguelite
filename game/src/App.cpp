#include "ear/App.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <random>

namespace ear
{
    namespace
    {

        bool rects_overlap(const SDL_FRect &a, const SDL_FRect &b)
        {
            return a.x < (b.x + b.w) &&
                   (a.x + a.w) > b.x &&
                   a.y < (b.y + b.h) &&
                   (a.y + a.h) > b.y;
        }

        UpgradeChoice make_choice(UpgradeType type)
        {
            switch (type)
            {
            case UpgradeType::MoveSpeed:
                return UpgradeChoice{type, "Move Speed +40", SDL_Color{70, 170, 255, 255}};
            case UpgradeType::MaxHp:
                return UpgradeChoice{type, "Max HP +1", SDL_Color{220, 80, 80, 255}};
            case UpgradeType::DashCooldown:
                return UpgradeChoice{type, "Dash Cooldown -15%", SDL_Color{170, 120, 255, 255}};
            case UpgradeType::DashSpeed:
                return UpgradeChoice{type, "Dash Speed +120", SDL_Color{200, 150, 255, 255}};
            case UpgradeType::AttackSize:
                return UpgradeChoice{type, "Attack Size +10", SDL_Color{235, 205, 90, 255}};
            case UpgradeType::ScoreBonus:
                return UpgradeChoice{type, "Score Bonus +10%", SDL_Color{80, 220, 170, 255}};
            }

            return UpgradeChoice{};
        }

        float telegraph_size_for(EnemyType type)
        {
            return type == EnemyType::Brute ? 78.0f : 58.0f;
        }

    } // namespace

    bool App::initialize()
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
            return false;
        }

        window_ = SDL_CreateWindow(
            "Expanding Arena Roguelite",
            window_width_,
            window_height_,
            0);

        if (!window_)
        {
            std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
            SDL_Quit();
            return false;
        }

        renderer_ = SDL_CreateRenderer(window_, nullptr);

        if (!renderer_)
        {
            std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            SDL_Quit();
            return false;
        }

        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

        restart_game();

        running_ = true;

        fmt::print("Game initialized successfully.\n");
        return true;
    }

    int App::run()
    {
        using clock = std::chrono::steady_clock;

        auto previous = clock::now();

        while (running_)
        {
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

    void App::shutdown()
    {
        if (renderer_)
        {
            SDL_DestroyRenderer(renderer_);
            renderer_ = nullptr;
        }

        if (window_)
        {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }

        SDL_Quit();
    }

    void App::handle_events()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running_ = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_ESCAPE && !event.key.repeat)
                {
                    running_ = false;
                }

                if (game_over_)
                {
                    if (event.key.key == SDLK_R && !event.key.repeat)
                    {
                        restart_game();
                    }
                }
                else if (choosing_character_)
                {
                    if (!event.key.repeat)
                    {
                        if (event.key.key == SDLK_1)
                        {
                            select_character(CharacterType::Hammer);
                        }
                        else if (event.key.key == SDLK_2)
                        {
                            select_character(CharacterType::Bow);
                        }
                        else if (event.key.key == SDLK_3)
                        {
                            select_character(CharacterType::Spear);
                        }
                    }
                }
                else if (choosing_upgrade_)
                {
                    if (!event.key.repeat)
                    {
                        if (event.key.key == SDLK_1)
                        {
                            apply_upgrade_by_index(0);
                        }
                        else if (event.key.key == SDLK_2)
                        {
                            apply_upgrade_by_index(1);
                        }
                        else if (event.key.key == SDLK_3)
                        {
                            apply_upgrade_by_index(2);
                        }
                    }
                }
                else
                {
                    if (event.key.key == SDLK_E && !event.key.repeat)
                    {
                        spawn_player_projectile();
                    }
                    else
                    {
                        player_.on_key_down(event.key.key, event.key.repeat);
                    }
                }
            }

            if (event.type == SDL_EVENT_KEY_UP &&
                !game_over_ &&
                !choosing_upgrade_ &&
                !choosing_character_)
            {
                player_.on_key_up(event.key.key);
            }
        }
    }

    void App::update(float dt_seconds)
    {
        if (game_over_ || choosing_upgrade_ || choosing_character_)
        {
            return;
        }

        player_.update(dt_seconds, window_width_, window_height_);

        spawn_enemies_if_needed(dt_seconds);

        for (std::size_t i = 0; i < projectiles_.size();)
        {
            projectiles_[i].update(dt_seconds, window_width_, window_height_);

            if (!projectiles_[i].is_alive())
            {
                projectiles_.erase(projectiles_.begin() + static_cast<long>(i));
                continue;
            }

            bool projectile_consumed = false;

            for (std::size_t j = 0; j < enemies_.size(); ++j)
            {
                if (enemies_[j].is_spawn_protected())
                {
                    continue;
                }

                if (!rects_overlap(projectiles_[i].bounds(), enemies_[j].bounds()))
                {
                    continue;
                }

                const SDL_FRect enemy_rect = enemies_[j].bounds();
                const SDL_FRect projectile_rect = projectiles_[i].bounds();

                const float enemy_center_x = enemy_rect.x + enemy_rect.w * 0.5f;
                const float enemy_center_y = enemy_rect.y + enemy_rect.h * 0.5f;
                const float projectile_center_x = projectile_rect.x + projectile_rect.w * 0.5f;
                const float projectile_center_y = projectile_rect.y + projectile_rect.h * 0.5f;

                const float dir_x = enemy_center_x - projectile_center_x;
                const float dir_y = enemy_center_y - projectile_center_y;

                if (enemies_[j].take_damage(
                        projectiles_[i].damage(),
                        dir_x,
                        dir_y,
                        projectiles_[i].knockback_strength()))
                {
                    projectiles_[i].deactivate();
                    projectile_consumed = true;

                    if (enemies_[j].is_dead())
                    {
                        const int base_score = enemies_[j].score_value();
                        const int gained_score =
                            base_score + (base_score * score_bonus_percent_) / 100;

                        enemies_.erase(enemies_.begin() + static_cast<long>(j));

                        ++kills_total_;
                        ++enemies_killed_in_wave_;
                        score_ += gained_score;

                        fmt::print(
                            "Projectile kill {} | +{} score | Total {} | Wave {} | Wave progress {}/{}\n",
                            kills_total_,
                            gained_score,
                            score_,
                            wave_,
                            enemies_killed_in_wave_,
                            enemies_to_spawn_in_wave_);

                        if (enemies_killed_in_wave_ >= enemies_to_spawn_in_wave_)
                        {
                            start_upgrade_selection();
                            update_window_title();
                            return;
                        }

                        update_window_title();
                    }

                    break;
                }
            }

            if (projectile_consumed || !projectiles_[i].is_alive())
            {
                projectiles_.erase(projectiles_.begin() + static_cast<long>(i));
                continue;
            }

            ++i;
        }

        const float enemy_speed_multiplier =
            std::min(1.0f + 0.03f * static_cast<float>(wave_ - 1), 1.18f);

        for (auto &enemy : enemies_)
        {
            enemy.update(player_.center_x(), player_.center_y(), dt_seconds, enemy_speed_multiplier);
        }

        for (std::size_t i = 0; i < enemies_.size();)
        {
            SDL_FRect enemy_rect = enemies_[i].bounds();

            if (enemies_[i].is_spawn_protected())
            {
                ++i;
                continue;
            }

            if (player_.attack_hits(enemy_rect))
            {
                const float enemy_center_x = enemy_rect.x + enemy_rect.w * 0.5f;
                const float enemy_center_y = enemy_rect.y + enemy_rect.h * 0.5f;
                const float dir_x = enemy_center_x - player_.center_x();
                const float dir_y = enemy_center_y - player_.center_y();
                const float knockback_strength =
                    enemies_[i].type() == EnemyType::Brute ? 260.0f : 360.0f;

                if (enemies_[i].take_damage(1, dir_x, dir_y, knockback_strength))
                {
                    if (enemies_[i].is_dead())
                    {
                        const int base_score = enemies_[i].score_value();
                        const int gained_score =
                            base_score + (base_score * score_bonus_percent_) / 100;

                        enemies_.erase(enemies_.begin() + static_cast<long>(i));

                        ++kills_total_;
                        ++enemies_killed_in_wave_;
                        score_ += gained_score;

                        fmt::print(
                            "Kill {} | +{} score | Total {} | Wave {} | Wave progress {}/{}\n",
                            kills_total_,
                            gained_score,
                            score_,
                            wave_,
                            enemies_killed_in_wave_,
                            enemies_to_spawn_in_wave_);

                        if (enemies_killed_in_wave_ >= enemies_to_spawn_in_wave_)
                        {
                            start_upgrade_selection();
                            update_window_title();
                            return;
                        }

                        update_window_title();
                        continue;
                    }
                }
            }

            ++i;
        }

        for (auto &enemy : enemies_)
        {
            if (enemy.is_spawn_protected())
            {
                continue;
            }

            if (rects_overlap(player_.bounds(), enemy.bounds()))
            {
                if (player_.try_take_damage(1))
                {
                    const SDL_FRect enemy_rect = enemy.bounds();
                    const float enemy_center_x = enemy_rect.x + enemy_rect.w * 0.5f;
                    const float enemy_center_y = enemy_rect.y + enemy_rect.h * 0.5f;

                    const float player_dir_x = player_.center_x() - enemy_center_x;
                    const float player_dir_y = player_.center_y() - enemy_center_y;

                    player_.apply_knockback(player_dir_x, player_dir_y, 480.0f, 0.18f);
                    enemy.apply_knockback(-player_dir_x, -player_dir_y, 220.0f, 0.10f);

                    fmt::print("Player took damage. HP: {}\n", player_.hp());

                    if (player_.is_dead())
                    {
                        game_over_ = true;
                        fmt::print("Game Over. Press R to restart.\n");
                    }

                    update_window_title();
                    break;
                }
            }
        }
        update_spawn_telegraphs(dt_seconds);
    }

    void App::render()
    {
        SDL_SetRenderDrawColor(renderer_, 20, 24, 32, 255);
        SDL_RenderClear(renderer_);
        render_spawn_telegraphs();

        for (const auto &enemy : enemies_)
        {
            enemy.render(renderer_);
        }

        for (const auto &projectile : projectiles_)
        {
            projectile.render(renderer_);
        }

        player_.render(renderer_);
        render_hp_bar();
        render_status_bars();

        if (choosing_character_)
        {
            render_character_overlay();
        }

        if (choosing_upgrade_)
        {
            render_upgrade_overlay();
        }

        if (game_over_)
        {
            render_game_over_overlay();
        }

        SDL_RenderPresent(renderer_);
    }

    void App::render_hp_bar()
    {
        const float start_x = 24.0f;
        const float start_y = 24.0f;
        const float box_width = 34.0f;
        const float box_height = 18.0f;
        const float gap = 8.0f;

        for (int i = 0; i < player_.max_hp(); ++i)
        {
            const SDL_FRect rect{
                start_x + i * (box_width + gap),
                start_y,
                box_width,
                box_height};

            if (i < player_.hp())
            {
                SDL_SetRenderDrawColor(renderer_, 210, 70, 70, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer_, 70, 70, 70, 255);
            }

            SDL_RenderFillRect(renderer_, &rect);
        }
    }

    void App::render_status_bars()
    {
        const float bar_x = 24.0f;
        const float attack_bar_y = 56.0f;
        const float dash_bar_y = 78.0f;
        const float ranged_bar_y = 100.0f;
        const float bar_width = 180.0f;
        const float bar_height = 12.0f;

        const SDL_FRect attack_bg{bar_x, attack_bar_y, bar_width, bar_height};
        const SDL_FRect dash_bg{bar_x, dash_bar_y, bar_width, bar_height};
        const SDL_FRect ranged_bg{bar_x, ranged_bar_y, bar_width, bar_height};

        SDL_SetRenderDrawColor(renderer_, 55, 55, 55, 255);
        SDL_RenderFillRect(renderer_, &attack_bg);
        SDL_RenderFillRect(renderer_, &dash_bg);
        SDL_RenderFillRect(renderer_, &ranged_bg);

        const float attack_ready_ratio = 1.0f - player_.attack_cooldown_ratio();
        const float dash_ready_ratio = 1.0f - player_.dash_cooldown_ratio();
        const float ranged_ready_ratio = 1.0f - player_.ranged_cooldown_ratio();

        const SDL_FRect attack_fill{
            bar_x,
            attack_bar_y,
            bar_width * std::clamp(attack_ready_ratio, 0.0f, 1.0f),
            bar_height};

        const SDL_FRect dash_fill{
            bar_x,
            dash_bar_y,
            bar_width * std::clamp(dash_ready_ratio, 0.0f, 1.0f),
            bar_height};

        const SDL_FRect ranged_fill{
            bar_x,
            ranged_bar_y,
            bar_width * std::clamp(ranged_ready_ratio, 0.0f, 1.0f),
            bar_height};

        SDL_SetRenderDrawColor(renderer_, 230, 200, 70, 255);
        SDL_RenderFillRect(renderer_, &attack_fill);

        SDL_SetRenderDrawColor(renderer_, 170, 120, 255, 255);
        SDL_RenderFillRect(renderer_, &dash_fill);

        SDL_SetRenderDrawColor(renderer_, 120, 220, 220, 255);
        SDL_RenderFillRect(renderer_, &ranged_fill);
    }

    void App::render_spawn_telegraphs()
    {
        for (const auto &enemy : enemies_)
        {
            if (!enemy.has_spawn_telegraph())
            {
                continue;
            }

            const SDL_FRect enemy_rect = enemy.bounds();
            const float progress = enemy.spawn_telegraph_progress();

            const SDL_FRect outer{
                enemy_rect.x - 10.0f,
                enemy_rect.y - 10.0f,
                enemy_rect.w + 20.0f,
                enemy_rect.h + 20.0f};

            const SDL_FRect inner{
                enemy_rect.x,
                enemy_rect.y,
                enemy_rect.w,
                enemy_rect.h};

            if (enemy.type() == EnemyType::Brute)
            {
                SDL_SetRenderDrawColor(
                    renderer_,
                    180,
                    90,
                    230,
                    static_cast<Uint8>(50 + progress * 90.0f));
                SDL_RenderFillRect(renderer_, &outer);

                SDL_SetRenderDrawColor(
                    renderer_,
                    230,
                    180,
                    255,
                    static_cast<Uint8>(90 + progress * 120.0f));
                SDL_RenderFillRect(renderer_, &inner);
            }
            else
            {
                SDL_SetRenderDrawColor(
                    renderer_,
                    220,
                    70,
                    70,
                    static_cast<Uint8>(50 + progress * 90.0f));
                SDL_RenderFillRect(renderer_, &outer);

                SDL_SetRenderDrawColor(
                    renderer_,
                    255,
                    170,
                    170,
                    static_cast<Uint8>(90 + progress * 120.0f));
                SDL_RenderFillRect(renderer_, &inner);
            }
        }
    }

    void App::render_upgrade_overlay()
    {
        const SDL_FRect full_screen{
            0.0f,
            0.0f,
            static_cast<float>(window_width_),
            static_cast<float>(window_height_)};

        SDL_SetRenderDrawColor(renderer_, 15, 15, 15, 150);
        SDL_RenderFillRect(renderer_, &full_screen);

        const float panel_w = 260.0f;
        const float panel_h = 180.0f;
        const float gap = 40.0f;
        const float total_w = panel_w * 3.0f + gap * 2.0f;
        const float start_x = (window_width_ - total_w) * 0.5f;
        const float y = 250.0f;

        for (int i = 0; i < 3; ++i)
        {
            const float x = start_x + i * (panel_w + gap);
            const SDL_FRect panel{x, y, panel_w, panel_h};

            const SDL_Color c = current_upgrade_choices_[i].color;
            SDL_SetRenderDrawColor(renderer_, c.r, c.g, c.b, 220);
            SDL_RenderFillRect(renderer_, &panel);

            const SDL_FRect inner{
                x + 8.0f,
                y + 8.0f,
                panel_w - 16.0f,
                panel_h - 16.0f};

            SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 120);
            SDL_RenderFillRect(renderer_, &inner);
        }
    }

    void App::render_character_overlay()
    {
        const SDL_FRect full_screen{
            0.0f,
            0.0f,
            static_cast<float>(window_width_),
            static_cast<float>(window_height_)};

        SDL_SetRenderDrawColor(renderer_, 10, 10, 10, 170);
        SDL_RenderFillRect(renderer_, &full_screen);

        const float panel_w = 260.0f;
        const float panel_h = 220.0f;
        const float gap = 40.0f;
        const float total_w = panel_w * 3.0f + gap * 2.0f;
        const float start_x = (window_width_ - total_w) * 0.5f;
        const float y = 220.0f;

        const SDL_Color colors[3] = {
            SDL_Color{180, 180, 180, 255},
            SDL_Color{90, 180, 255, 255},
            SDL_Color{120, 230, 170, 255}};

        for (int i = 0; i < 3; ++i)
        {
            const float x = start_x + i * (panel_w + gap);
            const SDL_FRect panel{x, y, panel_w, panel_h};

            SDL_SetRenderDrawColor(renderer_, colors[i].r, colors[i].g, colors[i].b, 230);
            SDL_RenderFillRect(renderer_, &panel);

            const SDL_FRect inner{
                x + 10.0f,
                y + 10.0f,
                panel_w - 20.0f,
                panel_h - 20.0f};

            SDL_SetRenderDrawColor(renderer_, 25, 25, 25, 140);
            SDL_RenderFillRect(renderer_, &inner);
        }
    }

    void App::render_game_over_overlay()
    {
        const SDL_FRect full_screen{
            0.0f,
            0.0f,
            static_cast<float>(window_width_),
            static_cast<float>(window_height_)};

        SDL_SetRenderDrawColor(renderer_, 120, 10, 10, 120);
        SDL_RenderFillRect(renderer_, &full_screen);

        const SDL_FRect center_panel{
            360.0f,
            260.0f,
            560.0f,
            200.0f};

        SDL_SetRenderDrawColor(renderer_, 25, 25, 25, 220);
        SDL_RenderFillRect(renderer_, &center_panel);
    }

    void App::restart_game()
    {
        player_.reset();
        projectiles_.clear();
        enemies_.clear();
        spawn_telegraphs_.clear();

        kills_total_ = 0;
        score_ = 0;
        score_bonus_percent_ = 0;
        upgrade_roll_counter_ = 0;
        choosing_upgrade_ = false;
        choosing_character_ = true;
        game_over_ = false;

        wave_ = 1;
        enemies_to_spawn_in_wave_ = 0;
        enemies_spawned_in_wave_ = 0;
        enemies_killed_in_wave_ = 0;
        spawn_timer_ = 0.0f;

        update_window_title();
        fmt::print("Game restarted. Choose a character.\n");
    }

    void App::update_window_title()
    {
        std::string title;

        if (game_over_)
        {
            title = fmt::format(
                "Expanding Arena Roguelite | GAME OVER | Wave {} | Kills {} | Score {} | Press R to Restart",
                wave_,
                kills_total_,
                score_);
        }
        else if (choosing_character_)
        {
            title = "Choose Character: [1] Hammer | [2] Bow | [3] Spear";
        }
        else if (choosing_upgrade_)
        {
            title = fmt::format(
                "Choose Upgrade: [1] {} | [2] {} | [3] {}",
                current_upgrade_choices_[0].name,
                current_upgrade_choices_[1].name,
                current_upgrade_choices_[2].name);
        }
        else
        {
            const int remaining_to_kill = enemies_to_spawn_in_wave_ - enemies_killed_in_wave_;

            title = fmt::format(
                "{} | HP {}/{} | Wave {} | Kills {} | Score {} | Alive {} | Remaining {} | Bonus {}%",
                player_.character_name(),
                player_.hp(),
                player_.max_hp(),
                wave_,
                kills_total_,
                score_,
                static_cast<int>(enemies_.size()),
                remaining_to_kill,
                score_bonus_percent_);
        }

        SDL_SetWindowTitle(window_, title.c_str());
    }

    void App::start_upgrade_selection()
    {
        choosing_upgrade_ = true;
        enemies_.clear();
        projectiles_.clear();
        spawn_telegraphs_.clear();
        roll_upgrade_choices();

        fmt::print("\nWave {} cleared. Choose an upgrade:\n", wave_);
        fmt::print("  [1] {}\n", upgrade_description(current_upgrade_choices_[0]));
        fmt::print("  [2] {}\n", upgrade_description(current_upgrade_choices_[1]));
        fmt::print("  [3] {}\n\n", upgrade_description(current_upgrade_choices_[2]));
    }

    void App::apply_upgrade_by_index(int index)
    {
        if (index < 0 || index >= 3 || !choosing_upgrade_)
        {
            return;
        }

        apply_upgrade(current_upgrade_choices_[index]);
        choosing_upgrade_ = false;
        begin_wave(wave_ + 1);
    }

    void App::roll_upgrade_choices()
    {
        static constexpr std::array<UpgradeType, 6> pool = {
            UpgradeType::MoveSpeed,
            UpgradeType::MaxHp,
            UpgradeType::DashCooldown,
            UpgradeType::DashSpeed,
            UpgradeType::AttackSize,
            UpgradeType::ScoreBonus};

        const int pool_size = static_cast<int>(pool.size());

        for (int i = 0; i < 3; ++i)
        {
            const int idx = (upgrade_roll_counter_ + i) % pool_size;
            current_upgrade_choices_[i] = make_choice(pool[idx]);
        }

        upgrade_roll_counter_ = (upgrade_roll_counter_ + 3) % pool_size;
    }

    void App::apply_upgrade(const UpgradeChoice &choice)
    {
        switch (choice.type)
        {
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

    std::string App::upgrade_description(const UpgradeChoice &choice) const
    {
        switch (choice.type)
        {
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

    void App::begin_wave(int wave_number)
    {
        wave_ = wave_number;
        enemies_.clear();
        projectiles_.clear();
        spawn_telegraphs_.clear();

        enemies_to_spawn_in_wave_ = total_enemies_for_wave(wave_);
        enemies_spawned_in_wave_ = 0;
        enemies_killed_in_wave_ = 0;
        spawn_timer_ = 0.0f;

        const int initial_spawn_target = concurrent_enemies_for_wave(wave_);
        while (static_cast<int>(enemies_.size()) < initial_spawn_target &&
               enemies_spawned_in_wave_ < enemies_to_spawn_in_wave_)
        {
            spawn_one_enemy();
        }

        fmt::print(
            "Wave {} started for {}. Total enemies: {} | Concurrent cap: {}\n",
            wave_,
            player_.character_name(),
            enemies_to_spawn_in_wave_,
            concurrent_enemies_for_wave(wave_));

        update_window_title();
    }

    void App::spawn_enemies_if_needed(float dt_seconds)
    {
        if (enemies_spawned_in_wave_ >= enemies_to_spawn_in_wave_)
        {
            return;
        }

        const int concurrent_cap = concurrent_enemies_for_wave(wave_);

        if (static_cast<int>(enemies_.size()) >= concurrent_cap)
        {
            return;
        }

        spawn_timer_ -= dt_seconds;
        if (spawn_timer_ > 0.0f)
        {
            return;
        }

        spawn_one_enemy();
        spawn_timer_ = spawn_interval_seconds_;
        update_window_title();
    }

    void App::update_spawn_telegraphs(float dt_seconds)
    {
        (void)dt_seconds;
    }

    void App::spawn_one_enemy()
    {
        auto [x, y] = random_spawn_position();
        const EnemyType type = next_enemy_type_for_wave();

        enemies_.emplace_back(x, y, type);
        enemies_.back().start_spawn_telegraph(spawn_telegraph_duration_seconds_);

        ++enemies_spawned_in_wave_;
    }

    std::pair<float, float> App::random_spawn_position()
    {
        std::uniform_int_distribution<int> side_dist(0, 3);
        std::uniform_real_distribution<float> x_dist(40.0f, window_width_ - 100.0f);
        std::uniform_real_distribution<float> y_dist(40.0f, window_height_ - 100.0f);

        const float min_distance = 240.0f;
        const float player_x = player_.center_x();
        const float player_y = player_.center_y();

        for (int attempt = 0; attempt < 32; ++attempt)
        {
            const int side = side_dist(rng_);
            float x = 0.0f;
            float y = 0.0f;

            switch (side)
            {
            case 0:
                x = x_dist(rng_);
                y = 40.0f;
                break;
            case 1:
                x = x_dist(rng_);
                y = window_height_ - 100.0f;
                break;
            case 2:
                x = 40.0f;
                y = y_dist(rng_);
                break;
            default:
                x = window_width_ - 100.0f;
                y = y_dist(rng_);
                break;
            }

            const float dx = x - player_x;
            const float dy = y - player_y;
            if (std::sqrt(dx * dx + dy * dy) >= min_distance)
            {
                return {x, y};
            }
        }

        return {40.0f, 40.0f};
    }

    int App::total_enemies_for_wave(int wave_number) const
    {
        return 2 * wave_number + 2;
    }

    int App::concurrent_enemies_for_wave(int wave_number) const
    {
        return std::min(2 + wave_number / 2, 5);
    }

    EnemyType App::next_enemy_type_for_wave() const
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> roll_dist(0, 99);

        if (wave_ >= 6)
        {
            const int roll = roll_dist(rng);

            if (roll < 18)
            {
                return EnemyType::Brute;
            }
            if (roll < 42)
            {
                return EnemyType::Dasher;
            }
            return EnemyType::Chaser;
        }

        if (wave_ >= 4)
        {
            const int roll = roll_dist(rng);

            if (roll < 18)
            {
                return EnemyType::Dasher;
            }
            if (roll < 30)
            {
                return EnemyType::Brute;
            }
            return EnemyType::Chaser;
        }

        return EnemyType::Chaser;
    }

    void App::spawn_player_projectile()
    {
        if (!player_.try_begin_ranged_attack())
        {
            return;
        }

        projectiles_.emplace_back(
            player_.projectile_spawn_x(),
            player_.projectile_spawn_y(),
            player_.facing_x(),
            player_.facing_y(),
            player_.projectile_speed(),
            player_.projectile_lifetime(),
            player_.projectile_width(),
            player_.projectile_height(),
            player_.projectile_damage(),
            player_.projectile_knockback(),
            player_.projectile_color());

        update_window_title();
    }

    void App::select_character(CharacterType type)
    {
        player_.select_character(type);
        choosing_character_ = false;
        projectiles_.clear();
        begin_wave(1);

        fmt::print("Selected character: {}\n", player_.character_name());
    }

} // namespace ear
