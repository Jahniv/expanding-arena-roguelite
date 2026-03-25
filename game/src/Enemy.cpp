#include "ear/Enemy.hpp"

#include <cmath>
#include <algorithm>

namespace ear
{

    Enemy::Enemy(float x, float y, EnemyType type)
        : type_(type), x_(x), y_(y)
    {
        if (type_ == EnemyType::Brute)
        {
            size_ = 58.0f;
            base_speed_ = 82.0f;
            hp_ = 4;
        }
        else if (type_ == EnemyType::Dasher)
        {
            size_ = 34.0f;
            base_speed_ = 118.0f;
            hp_ = 2;
            dash_cooldown_timer_ = 1.10f;
        }
        else
        {
            size_ = 34.0f;
            base_speed_ = 104.0f;
            hp_ = 1;
        }
    }

    void Enemy::start_spawn_telegraph(float duration_seconds)
    {
        spawn_telegraph_duration_ = duration_seconds;
        spawn_telegraph_timer_ = duration_seconds;
    }

    void Enemy::update(float target_x, float target_y, float dt_seconds, float speed_multiplier)
    {
        hit_invulnerability_timer_ = std::max(0.0f, hit_invulnerability_timer_ - dt_seconds);
        knockback_timer_ = std::max(0.0f, knockback_timer_ - dt_seconds);
        spawn_telegraph_timer_ = std::max(0.0f, spawn_telegraph_timer_ - dt_seconds);
        dash_cooldown_timer_ = std::max(0.0f, dash_cooldown_timer_ - dt_seconds);
        dash_windup_timer_ = std::max(0.0f, dash_windup_timer_ - dt_seconds);
        dash_duration_timer_ = std::max(0.0f, dash_duration_timer_ - dt_seconds);

        if (is_dead())
        {
            return;
        }

        if (knockback_timer_ > 0.0f)
        {
            x_ += knockback_velocity_x_ * dt_seconds;
            y_ += knockback_velocity_y_ * dt_seconds;
            return;
        }

        const float my_center_x = x_ + size_ * 0.5f;
        const float my_center_y = y_ + size_ * 0.5f;

        float dx = target_x - my_center_x;
        float dy = target_y - my_center_y;

        const float length = std::sqrt(dx * dx + dy * dy);

        if (type_ == EnemyType::Dasher)
        {
            if (dash_pending_)
            {
                if (dash_windup_timer_ > 0.0f)
                {
                    return;
                }

                dash_pending_ = false;
                dash_duration_timer_ = 0.34f;
            }

            if (dash_duration_timer_ > 0.0f)
            {
                const float dash_speed = 520.0f * speed_multiplier;
                x_ += dash_direction_x_ * dash_speed * dt_seconds;
                y_ += dash_direction_y_ * dash_speed * dt_seconds;
                return;
            }

            if (length > 0.001f &&
                dash_cooldown_timer_ <= 0.0f &&
                length < 260.0f &&
                length > 95.0f)
            {
                dash_direction_x_ = dx / length;
                dash_direction_y_ = dy / length;
                dash_windup_timer_ = 0.50f;
                dash_cooldown_timer_ = 2.10f;
                dash_pending_ = true;
                return;
            }
        }

        if (length > 0.001f)
        {
            dx /= length;
            dy /= length;

            x_ += dx * base_speed_ * speed_multiplier * dt_seconds;
            y_ += dy * base_speed_ * speed_multiplier * dt_seconds;
        }
    }

    void Enemy::render(SDL_Renderer *renderer) const
    {
        const SDL_FRect enemy_rect = bounds();

        if (type_ == EnemyType::Brute)
        {
            if (is_invulnerable())
            {
                SDL_SetRenderDrawColor(renderer, 220, 170, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 180, 90, 230, 255);
            }
        }
        else if (type_ == EnemyType::Dasher)
        {
            if (dash_windup_timer_ > 0.0f)
            {
                SDL_SetRenderDrawColor(renderer, 255, 230, 90, 255);
            }
            else if (is_invulnerable())
            {
                SDL_SetRenderDrawColor(renderer, 255, 210, 140, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 150, 40, 255);
            }
        }
        else
        {
            if (is_invulnerable())
            {
                SDL_SetRenderDrawColor(renderer, 255, 150, 150, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 220, 70, 70, 255);
            }
        }

        SDL_RenderFillRect(renderer, &enemy_rect);
    }

    SDL_FRect Enemy::bounds() const
    {
        return SDL_FRect{x_, y_, size_, size_};
    }

    void Enemy::set_position(float x, float y)
    {
        x_ = x;
        y_ = y;
    }

    bool Enemy::take_damage(int amount, float dir_x, float dir_y, float knockback_strength)
    {
        if (amount <= 0 || is_dead() || is_invulnerable())
        {
            return false;
        }

        hp_ = std::max(0, hp_ - amount);
        hit_invulnerability_timer_ = hit_invulnerability_duration_;
        apply_knockback(dir_x, dir_y, knockback_strength);
        return true;
    }

    void Enemy::apply_knockback(float dir_x, float dir_y, float strength, float duration)
    {
        const float length = std::sqrt(dir_x * dir_x + dir_y * dir_y);
        if (length <= 0.001f)
        {
            return;
        }

        knockback_velocity_x_ = (dir_x / length) * strength;
        knockback_velocity_y_ = (dir_y / length) * strength;
        knockback_timer_ = duration;
    }

    bool Enemy::is_dead() const
    {
        return hp_ <= 0;
    }

    bool Enemy::is_invulnerable() const
    {
        return hit_invulnerability_timer_ > 0.0f;
    }

    EnemyType Enemy::type() const
    {
        return type_;
    }

    int Enemy::score_value() const
    {
        return type_ == EnemyType::Brute ? 160 : 100;
    }

    int Enemy::hp() const
    {
        return hp_;
    }

    int Enemy::max_hp() const
    {
        return max_hp_;
    }

} // namespace ear
