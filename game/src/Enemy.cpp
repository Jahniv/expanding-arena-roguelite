#include "ear/Enemy.hpp"

#include <cmath>

namespace ear {

void Enemy::update(float target_x, float target_y, float dt_seconds) {
    const float my_center_x = x_ + size_ * 0.5f;
    const float my_center_y = y_ + size_ * 0.5f;

    float dx = target_x - my_center_x;
    float dy = target_y - my_center_y;

    const float length = std::sqrt(dx * dx + dy * dy);

    if (length > 0.001f) {
        dx /= length;
        dy /= length;

        x_ += dx * speed_ * dt_seconds;
        y_ += dy * speed_ * dt_seconds;
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
    x_ = 920.0f;
    y_ = 180.0f;
}

} // namespace ear
