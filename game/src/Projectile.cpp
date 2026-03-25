#include "ear/Projectile.hpp"

#include <cmath>

namespace ear {

Projectile::Projectile(
    float x,
    float y,
    float dir_x,
    float dir_y,
    float speed,
    float lifetime_seconds,
    float width,
    float height,
    int damage,
    float knockback_strength,
    SDL_Color color)
    : x_(x),
      y_(y),
      width_(width),
      height_(height),
      lifetime_seconds_(lifetime_seconds),
      damage_(damage),
      knockback_strength_(knockback_strength),
      color_(color) {
    const float length = std::sqrt(dir_x * dir_x + dir_y * dir_y);

    if (length > 0.001f) {
        velocity_x_ = (dir_x / length) * speed;
        velocity_y_ = (dir_y / length) * speed;
    } else {
        velocity_x_ = speed;
        velocity_y_ = 0.0f;
    }
}

void Projectile::update(float dt_seconds, int window_width, int window_height) {
    if (!alive_) {
        return;
    }

    lifetime_seconds_ -= dt_seconds;
    if (lifetime_seconds_ <= 0.0f) {
        alive_ = false;
        return;
    }

    x_ += velocity_x_ * dt_seconds;
    y_ += velocity_y_ * dt_seconds;

    if (x_ < -width_ || y_ < -height_ ||
        x_ > static_cast<float>(window_width) + width_ ||
        y_ > static_cast<float>(window_height) + height_) {
        alive_ = false;
    }
}

void Projectile::render(SDL_Renderer* renderer) const {
    if (!alive_) {
        return;
    }

    const SDL_FRect rect = bounds();
    SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);
    SDL_RenderFillRect(renderer, &rect);
}

SDL_FRect Projectile::bounds() const {
    const bool horizontal = std::abs(velocity_x_) >= std::abs(velocity_y_);

    if (horizontal) {
        return SDL_FRect{ x_, y_, width_, height_ };
    }

    return SDL_FRect{ x_, y_, height_, width_ };
}

bool Projectile::is_alive() const {
    return alive_;
}

void Projectile::deactivate() {
    alive_ = false;
}

int Projectile::damage() const {
    return damage_;
}

float Projectile::knockback_strength() const {
    return knockback_strength_;
}

} // namespace ear
