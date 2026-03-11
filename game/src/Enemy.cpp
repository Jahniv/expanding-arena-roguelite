#include "ear/Enemy.hpp"

#include <cmath>

namespace ear {

Enemy::Enemy(float spawn_x, float spawn_y)
    : spawn_x_(spawn_x),
      spawn_y_(spawn_y),
      x_(spawn_x),
      y_(spawn_y) {
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
    SDL_SetRenderDrawColor(renderer, 220, 70, 70, 255);
    SDL_RenderFillRect(renderer, &enemy_rect);
}

SDL_FRect Enemy::bounds() const {
    return SDL_FRect{ x_, y_, size_, size_ };
}

void Enemy::respawn() {
    x_ = spawn_x_;
    y_ = spawn_y_;
}

} // namespace ear
