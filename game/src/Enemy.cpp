#include "ear/Enemy.hpp"

#include <cmath>

namespace ear {

Enemy::Enemy(float x, float y, EnemyType type)
    : type_(type), x_(x), y_(y) {
    if (type_ == EnemyType::Brute) {
        size_ = 62.0f;
        base_speed_ = 72.0f;
    } else {
        size_ = 44.0f;
        base_speed_ = 105.0f;
    }
}

void Enemy::update(float target_x, float target_y, float dt_seconds, float speed_multiplier) {
    const float my_center_x = x_ + size_ * 0.5f;
    const float my_center_y = y_ + size_ * 0.5f;

    float dx = target_x - my_center_x;
    float dy = target_y - my_center_y;

    const float length = std::sqrt(dx * dx + dy * dy);

    if (length > 0.001f) {
        dx /= length;
        dy /= length;

        x_ += dx * base_speed_ * speed_multiplier * dt_seconds;
        y_ += dy * base_speed_ * speed_multiplier * dt_seconds;
    }
}

void Enemy::render(SDL_Renderer* renderer) const {
    const SDL_FRect enemy_rect = bounds();

    if (type_ == EnemyType::Brute) {
        SDL_SetRenderDrawColor(renderer, 180, 90, 230, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 220, 70, 70, 255);
    }

    SDL_RenderFillRect(renderer, &enemy_rect);
}

SDL_FRect Enemy::bounds() const {
    return SDL_FRect{ x_, y_, size_, size_ };
}

void Enemy::set_position(float x, float y) {
    x_ = x;
    y_ = y;
}

EnemyType Enemy::type() const {
    return type_;
}

int Enemy::score_value() const {
    return type_ == EnemyType::Brute ? 160 : 100;
}

} // namespace ear
