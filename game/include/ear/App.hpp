#pragma once
// s
#include <SDL3/SDL.h>
#include <array>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "ear/Enemy.hpp"
#include "ear/Player.hpp"
#include "ear/Projectile.hpp"

namespace ear
{

    enum class UpgradeType
    {
        MoveSpeed,
        MaxHp,
        DashCooldown,
        DashSpeed,
        AttackSize,
        ScoreBonus
    };

    struct UpgradeChoice
    {
        UpgradeType type{};
        const char *name = "";
        SDL_Color color{255, 255, 255, 255};
    };

    class App
    {
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
        void render_spawn_telegraphs();
        void render_game_over_overlay();
        void render_upgrade_overlay();
        void render_character_overlay();

        void restart_game();
        void update_window_title();

        void start_upgrade_selection();
        void apply_upgrade_by_index(int index);
        void roll_upgrade_choices();
        void apply_upgrade(const UpgradeChoice &choice);
        std::string upgrade_description(const UpgradeChoice &choice) const;

        void begin_wave(int wave_number);
        void spawn_enemies_if_needed(float dt_seconds);
        void update_spawn_telegraphs(float dt_seconds);
        void spawn_one_enemy();
        std::pair<float, float> random_spawn_position();

        int total_enemies_for_wave(int wave_number) const;
        int concurrent_enemies_for_wave(int wave_number) const;
        EnemyType next_enemy_type_for_wave() const;

        void spawn_player_projectile();
        void select_character(CharacterType type);

        SDL_Window *window_ = nullptr;
        SDL_Renderer *renderer_ = nullptr;
        bool running_ = false;
        bool game_over_ = false;
        bool choosing_upgrade_ = false;
        bool choosing_character_ = true;

        struct SpawnTelegraph
        {
            float x = 0.0f;
            float y = 0.0f;
            float timer = 0.0f;
            float duration = 0.0f;
            EnemyType type = EnemyType::Chaser;
        };

        Player player_;
        std::vector<Enemy> enemies_;
        std::vector<Projectile> projectiles_;
        std::vector<SpawnTelegraph> spawn_telegraphs_;
        std::array<UpgradeChoice, 3> current_upgrade_choices_{};

        int wave_ = 1;
        int kills_total_ = 0;
        int score_ = 0;
        int score_bonus_percent_ = 0;
        int upgrade_roll_counter_ = 0;

        int enemies_to_spawn_in_wave_ = 0;
        int enemies_spawned_in_wave_ = 0;
        int enemies_killed_in_wave_ = 0;

        float spawn_interval_seconds_ = 0.70f;
        float spawn_timer_ = 0.0f;
        float spawn_telegraph_duration_seconds_ = 0.95f;

        std::mt19937 rng_{std::random_device{}()};

        static constexpr int window_width_ = 1280;
        static constexpr int window_height_ = 720;
    };

} // namespace ear
