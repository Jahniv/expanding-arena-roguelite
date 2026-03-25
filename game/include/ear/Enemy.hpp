#pragma once

#include <SDL3/SDL.h>

namespace ear
{

    enum class EnemyType
    {
        Chaser,
        Brute,
        Dasher
    };

    class Enemy
    {
    public:
        Enemy(float x = 920.0f, float y = 180.0f, EnemyType type = EnemyType::Chaser);

        void update(float target_x, float target_y, float dt_seconds, float speed_multiplier = 1.0f);
        void render(SDL_Renderer *renderer) const;

        SDL_FRect bounds() const;
        void start_spawn_telegraph(float duration_seconds);
        bool has_spawn_telegraph() const { return spawn_telegraph_timer_ > 0.0f; }
        bool is_spawn_protected() const { return spawn_telegraph_timer_ > 0.0f; }
        float spawn_telegraph_progress() const
        {
            if (spawn_telegraph_duration_ <= 0.0f)
            {
                return 1.0f;
            }

            return 1.0f - (spawn_telegraph_timer_ / spawn_telegraph_duration_);
        }

        void set_position(float x, float y);

        bool take_damage(int amount, float dir_x, float dir_y, float knockback_strength);
        void apply_knockback(float dir_x, float dir_y, float strength, float duration = 0.12f);

        bool is_dead() const;
        bool is_invulnerable() const;

        EnemyType type() const;
        int score_value() const;
        int hp() const;
        int max_hp() const;

    private:
        EnemyType type_ = EnemyType::Chaser;

        float x_ = 920.0f;
        float y_ = 180.0f;
        float size_ = 44.0f;
        float base_speed_ = 105.0f;

        float hit_invulnerability_timer_ = 0.0f;
        float hit_invulnerability_duration_ = 0.20f;

        float knockback_timer_ = 0.0f;
        float knockback_velocity_x_ = 0.0f;
        float knockback_velocity_y_ = 0.0f;
        float spawn_telegraph_timer_ = 0.0f;
        float spawn_telegraph_duration_ = 0.0f;

        float dash_cooldown_timer_ = 0.0f;
        float dash_windup_timer_ = 0.0f;
        float dash_duration_timer_ = 0.0f;
        bool dash_pending_ = false;
        float dash_direction_x_ = 0.0f;
        float dash_direction_y_ = 0.0f;

        int hp_ = 2;
        int max_hp_ = 2;
    };

} // namespace ear
