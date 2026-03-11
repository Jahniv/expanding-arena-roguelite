#include "ear/Player.hpp"

#include <algorithm>
#include <cmath>

namespace ear {
namespace {

bool rects_overlap(const SDL_FRect& a, const SDL_FRect& b) {
    return a.x < (b.x + b.w) &&
           (a.x + a.w) > b.x &&
           a.y < (b.y + b.h) &&
           (a.y + a.h) > b.y;
}

} // namespace

void Player::on_key_down(SDL_Keycode key, bool repeat) {
    if (repeat) {
        return;
    }

    switch (key) {
        case SDLK_W:
        case SDLK_UP:
            move_up_ = true;
            break;
        case SDLK_S:
        case SDLK_DOWN:
            move_down_ = true;
            break;
        case SDLK_A:
        case SDLK_LEFT:
            move_left_ = true;
            break;
        case SDLK_D:
        case SDLK_RIGHT:
            move_right_ = true;
            break;
        case SDLK_SPACE:
            if (attack_timer_ <= 0.0f && attack_cooldown_ <= 0.0f && !is_dead()) {
                attack_timer_ = 0.12f;
                attack_cooldown_ = 0.25f;
            }
            break;
        default:
            break;
    }
}

void Player::on_key_up(SDL_Keycode key) {
    switch (key) {
        case SDLK_W:
        case SDLK_UP:
            move_up_ = false;
            break;
        case SDLK_S:
        case SDLK_DOWN:
            move_down_ = false;
            break;
        case SDLK_A:
        case SDLK_LEFT:
            move_left_ = false;
            break;
        case SDLK_D:
        case SDLK_RIGHT:
            move_right_ = false;
            break;
        default:
            break;
    }
}

void Player::update(float dt_seconds, int window_width, int window_height) {
    attack_timer_ = std::max(0.0f, attack_timer_ - dt_seconds);
    attack_cooldown_ = std::max(0.0f, attack_cooldown_ - dt_seconds);
    damage_invulnerability_timer_ = std::max(0.0f, damage_invulnerability_timer_ - dt_seconds);

    if (is_dead()) {
        return;
    }

    float dx = 0.0f;
    float dy = 0.0f;

    if (move_up_) {
        dy -= 1.0f;
    }
    if (move_down_) {
        dy += 1.0f;
    }
    if (move_left_) {
        dx -= 1.0f;
    }
    if (move_right_) {
        dx += 1.0f;
    }

    if (dx != 0.0f || dy != 0.0f) {
        const float length = std::sqrt(dx * dx + dy * dy);
        dx /= length;
        dy /= length;

        facing_x_ = dx;
        facing_y_ = dy;

        x_ += dx * speed_ * dt_seconds;
        y_ += dy * speed_ * dt_seconds;
    }

    x_ = std::clamp(x_, 0.0f, static_cast<float>(window_width) - size_);
    y_ = std::clamp(y_, 0.0f, static_cast<float>(window_height) - size_);
}

void Player::render(SDL_Renderer* renderer) const {
    const SDL_FRect player_rect = bounds();

    if (is_invulnerable()) {
        SDL_SetRenderDrawColor(renderer, 120, 220, 255, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 80, 200, 120, 255);
    }

    SDL_RenderFillRect(renderer, &player_rect);

    if (is_attacking()) {
        const SDL_FRect attack_rect = attack_bounds();
        SDL_SetRenderDrawColor(renderer, 240, 220, 90, 255);
        SDL_RenderFillRect(renderer, &attack_rect);
    }
}

SDL_FRect Player::bounds() const {
    return SDL_FRect{ x_, y_, size_, size_ };
}

SDL_FRect Player::attack_bounds() const {
    if (!is_attacking()) {
        return SDL_FRect{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    const float attack_size = 36.0f;
    const float padding = 8.0f;

    if (std::abs(facing_x_) >= std::abs(facing_y_)) {
        if (facing_x_ >= 0.0f) {
            return SDL_FRect{
                x_ + size_ + padding,
                y_ + (size_ - attack_size) * 0.5f,
                attack_size,
                attack_size
            };
        }

        return SDL_FRect{
            x_ - attack_size - padding,
            y_ + (size_ - attack_size) * 0.5f,
            attack_size,
            attack_size
        };
    }

    if (facing_y_ >= 0.0f) {
        return SDL_FRect{
            x_ + (size_ - attack_size) * 0.5f,
            y_ + size_ + padding,
            attack_size,
            attack_size
        };
    }

    return SDL_FRect{
        x_ + (size_ - attack_size) * 0.5f,
        y_ - attack_size - padding,
        attack_size,
        attack_size
    };
}

bool Player::is_attacking() const {
    return attack_timer_ > 0.0f;
}

bool Player::attack_hits(const SDL_FRect& target) const {
    if (!is_attacking()) {
        return false;
    }

    return rects_overlap(attack_bounds(), target);
}

bool Player::try_take_damage(int amount) {
    if (amount <= 0 || is_dead() || is_invulnerable()) {
        return false;
    }

    hp_ = std::max(0, hp_ - amount);
    damage_invulnerability_timer_ = 0.70f;
    return true;
}

bool Player::is_dead() const {
    return hp_ <= 0;
}

bool Player::is_invulnerable() const {
    return damage_invulnerability_timer_ > 0.0f;
}

int Player::hp() const {
    return hp_;
}

int Player::max_hp() const {
    return max_hp_;
}

void Player::reset() {
    x_ = 120.0f;
    y_ = 320.0f;
    size_ = 50.0f;
    speed_ = 300.0f;

    move_up_ = false;
    move_down_ = false;
    move_left_ = false;
    move_right_ = false;

    facing_x_ = 1.0f;
    facing_y_ = 0.0f;

    attack_timer_ = 0.0f;
    attack_cooldown_ = 0.0f;
    damage_invulnerability_timer_ = 0.0f;

    hp_ = max_hp_;
}

float Player::center_x() const {
    return x_ + size_ * 0.5f;
}

float Player::center_y() const {
    return y_ + size_ * 0.5f;
}

} // namespace ear
