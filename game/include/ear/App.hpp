#pragma once

#include <SDL3/SDL.h>
#include <array>
#include <string>
#include <vector>

#include "ear/Enemy.hpp"
#include "ear/Player.hpp"

namespace ear {

enum class UpgradeType {
    MoveSpeed,
    MaxHp,
    DashCooldown,
    DashSpeed,
    AttackSize,
    ScoreBonus
};

struct UpgradeChoice {
    UpgradeType type{};
    const char* name = "";
    SDL_Color color{ 255, 255, 255, 255 };
};

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
    void render_upgrade_overlay();
    void restart_game();
    void initialize_enemies();
    void update_window_title();

    void start_upgrade_selection();
    void apply_upgrade_by_index(int index);
    void roll_upgrade_choices();
    void apply_upgrade(const UpgradeChoice& choice);
    std::string upgrade_description(const UpgradeChoice& choice) const;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool running_ = false;
    bool game_over_ = false;
    bool choosing_upgrade_ = false;

    Player player_;
    std::vector<Enemy> enemies_;

    std::array<UpgradeChoice, 3> current_upgrade_choices_{};

    int wave_ = 1;
    int kills_total_ = 0;
    int kills_in_wave_ = 0;
    int score_ = 0;
    int active_enemy_count_ = 1;
    int score_bonus_percent_ = 0;
    int upgrade_roll_counter_ = 0;

    static constexpr int window_width_ = 1280;
    static constexpr int window_height_ = 720;
    static constexpr int kills_per_wave_ = 3;
};

} // namespace ear
