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
            if (attack_timer_ <= 0.0f &&
                attack_cooldown_timer_ <= 0.0f &&
                !is_dead() &&
                !is_dashing() &&
                !is_knocked_back()) {
                attack_timer_ = attack_duration_;
                attack_cooldown_timer_ = attack_cooldown_duration_;
            }
            break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT: {
            if (dash_timer_ > 0.0f || dash_cooldown_timer_ > 0.0f || is_dead() || is_knocked_back()) {
                break;
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

            if (dx == 0.0f && dy == 0.0f) {
                dx = facing_x_;
                dy = facing_y_;
            }

            const float length = std::sqrt(dx * dx + dy * dy);

            if (length > 0.001f) {
                dash_dir_x_ = dx / length;
                dash_dir_y_ = dy / length;
                dash_timer_ = dash_duration_;
                dash_cooldown_timer_ = dash_cooldown_duration_;
                post_dash_invulnerability_timer_ = 0.0f;
            }
            break;
        }
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
    attack_cooldown_timer_ = std::max(0.0f, attack_cooldown_timer_ - dt_seconds);
    ranged_cooldown_timer_ = std::max(0.0f, ranged_cooldown_timer_ - dt_seconds);

    post_dash_invulnerability_timer_ =
        std::max(0.0f, post_dash_invulnerability_timer_ - dt_seconds);

    const float previous_dash_timer = dash_timer_;
    dash_timer_ = std::max(0.0f, dash_timer_ - dt_seconds);
    dash_cooldown_timer_ = std::max(0.0f, dash_cooldown_timer_ - dt_seconds);

    if (previous_dash_timer > 0.0f && dash_timer_ <= 0.0f) {
        post_dash_invulnerability_timer_ = post_dash_invulnerability_duration_;
    }

    knockback_timer_ = std::max(0.0f, knockback_timer_ - dt_seconds);

    damage_invulnerability_timer_ =
        std::max(0.0f, damage_invulnerability_timer_ - dt_seconds);

    if (is_dead()) {
        return;
    }

    if (is_knocked_back()) {
        x_ += knockback_velocity_x_ * dt_seconds;
        y_ += knockback_velocity_y_ * dt_seconds;

        x_ = std::clamp(x_, 0.0f, static_cast<float>(window_width) - size_);
        y_ = std::clamp(y_, 0.0f, static_cast<float>(window_height) - size_);
        return;
    }

    if (is_dashing()) {
        x_ += dash_dir_x_ * dash_speed_ * dt_seconds;
        y_ += dash_dir_y_ * dash_speed_ * dt_seconds;

        x_ = std::clamp(x_, 0.0f, static_cast<float>(window_width) - size_);
        y_ = std::clamp(y_, 0.0f, static_cast<float>(window_height) - size_);
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

    if (is_dashing()) {
        SDL_SetRenderDrawColor(renderer, 210, 160, 255, 255);
    } else if (is_knocked_back()) {
        SDL_SetRenderDrawColor(renderer, 255, 180, 120, 255);
    } else if (post_dash_invulnerability_timer_ > 0.0f) {
        SDL_SetRenderDrawColor(renderer, 200, 180, 255, 255);
    } else if (damage_invulnerability_timer_ > 0.0f) {
        SDL_SetRenderDrawColor(renderer, 120, 220, 255, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, body_color_.r, body_color_.g, body_color_.b, body_color_.a);
    }

    SDL_RenderFillRect(renderer, &player_rect);

    if (is_attacking()) {
        const SDL_FRect attack_rect = attack_bounds();
        SDL_SetRenderDrawColor(renderer, attack_color_.r, attack_color_.g, attack_color_.b, attack_color_.a);
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

    const float padding = 8.0f;
    const bool horizontal = std::abs(facing_x_) >= std::abs(facing_y_);

    if (horizontal) {
        const float width = attack_main_size_;
        const float height = attack_cross_size_;

        if (facing_x_ >= 0.0f) {
            return SDL_FRect{
                x_ + size_ + padding,
                y_ + (size_ - height) * 0.5f,
                width,
                height
            };
        }

        return SDL_FRect{
            x_ - width - padding,
            y_ + (size_ - height) * 0.5f,
            width,
            height
        };
    }

    const float width = attack_cross_size_;
    const float height = attack_main_size_;

    if (facing_y_ >= 0.0f) {
        return SDL_FRect{
            x_ + (size_ - width) * 0.5f,
            y_ + size_ + padding,
            width,
            height
        };
    }

    return SDL_FRect{
        x_ + (size_ - width) * 0.5f,
        y_ - height - padding,
        width,
        height
    };
}

bool Player::is_attacking() const {
    return attack_timer_ > 0.0f;
}

bool Player::is_dashing() const {
    return dash_timer_ > 0.0f;
}

bool Player::is_knocked_back() const {
    return knockback_timer_ > 0.0f;
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
    damage_invulnerability_timer_ = damage_invulnerability_duration_;
    return true;
}

void Player::apply_knockback(float dir_x, float dir_y, float strength, float duration) {
    const float length = std::sqrt(dir_x * dir_x + dir_y * dir_y);
    if (length <= 0.001f) {
        return;
    }

    knockback_velocity_x_ = (dir_x / length) * strength;
    knockback_velocity_y_ = (dir_y / length) * strength;
    knockback_timer_ = duration;
}

bool Player::is_dead() const {
    return hp_ <= 0;
}

bool Player::is_invulnerable() const {
    return damage_invulnerability_timer_ > 0.0f ||
           dash_timer_ > 0.0f ||
           post_dash_invulnerability_timer_ > 0.0f;
}

int Player::hp() const {
    return hp_;
}

int Player::max_hp() const {
    return max_hp_;
}

float Player::attack_cooldown_ratio() const {
    if (attack_cooldown_duration_ <= 0.0f) {
        return 0.0f;
    }

    return std::clamp(attack_cooldown_timer_ / attack_cooldown_duration_, 0.0f, 1.0f);
}

float Player::dash_cooldown_ratio() const {
    if (dash_cooldown_duration_ <= 0.0f) {
        return 0.0f;
    }

    return std::clamp(dash_cooldown_timer_ / dash_cooldown_duration_, 0.0f, 1.0f);
}

float Player::ranged_cooldown_ratio() const {
    if (ranged_cooldown_duration_ <= 0.0f) {
        return 0.0f;
    }

    return std::clamp(ranged_cooldown_timer_ / ranged_cooldown_duration_, 0.0f, 1.0f);
}

float Player::move_speed() const {
    return speed_;
}

float Player::dash_speed() const {
    return dash_speed_;
}

float Player::dash_cooldown_seconds() const {
    return dash_cooldown_duration_;
}

float Player::attack_size() const {
    return attack_main_size_;
}

bool Player::try_begin_ranged_attack() {
    if (ranged_cooldown_timer_ > 0.0f || is_dead() || is_dashing() || is_knocked_back()) {
        return false;
    }

    ranged_cooldown_timer_ = ranged_cooldown_duration_;
    return true;
}

float Player::projectile_speed() const {
    return projectile_speed_;
}

float Player::projectile_lifetime() const {
    return projectile_lifetime_;
}

float Player::projectile_width() const {
    return projectile_width_;
}

float Player::projectile_height() const {
    return projectile_height_;
}

int Player::projectile_damage() const {
    return projectile_damage_;
}

float Player::projectile_knockback() const {
    return projectile_knockback_;
}

float Player::projectile_spawn_x() const {
    return center_x() - projectile_width_ * 0.5f;
}

float Player::projectile_spawn_y() const {
    return center_y() - projectile_height_ * 0.5f;
}

float Player::facing_x() const {
    return facing_x_;
}

float Player::facing_y() const {
    return facing_y_;
}

SDL_Color Player::body_color() const {
    return body_color_;
}

SDL_Color Player::attack_color() const {
    return attack_color_;
}

SDL_Color Player::projectile_color() const {
    return projectile_color_;
}

CharacterType Player::character_type() const {
    return character_type_;
}

const char* Player::character_name() const {
    switch (character_type_) {
        case CharacterType::Hammer:
            return "Hammer";
        case CharacterType::Bow:
            return "Bow";
        case CharacterType::Spear:
            return "Spear";
        default:
            return "None";
    }
}

void Player::select_character(CharacterType type) {
    character_type_ = type;

    switch (type) {
        case CharacterType::Hammer:
            body_color_ = SDL_Color{150, 150, 150, 255};
            attack_color_ = SDL_Color{235, 190, 70, 255};
            projectile_color_ = SDL_Color{160, 160, 160, 255};

            attack_duration_ = 0.14f;
            attack_cooldown_duration_ = 0.42f;
            attack_main_size_ = 58.0f;
            attack_cross_size_ = 58.0f;

            ranged_cooldown_duration_ = 0.75f;
            projectile_speed_ = 430.0f;
            projectile_lifetime_ = 0.85f;
            projectile_width_ = 24.0f;
            projectile_height_ = 24.0f;
            projectile_damage_ = 1;
            projectile_knockback_ = 420.0f;
            break;

        case CharacterType::Bow:
            body_color_ = SDL_Color{90, 180, 255, 255};
            attack_color_ = SDL_Color{220, 220, 220, 255};
            projectile_color_ = SDL_Color{120, 240, 240, 255};

            attack_duration_ = 0.10f;
            attack_cooldown_duration_ = 0.18f;
            attack_main_size_ = 24.0f;
            attack_cross_size_ = 24.0f;

            ranged_cooldown_duration_ = 0.26f;
            projectile_speed_ = 930.0f;
            projectile_lifetime_ = 1.35f;
            projectile_width_ = 24.0f;
            projectile_height_ = 8.0f;
            projectile_damage_ = 1;
            projectile_knockback_ = 180.0f;
            break;

        case CharacterType::Spear:
            body_color_ = SDL_Color{120, 230, 170, 255};
            attack_color_ = SDL_Color{250, 235, 140, 255};
            projectile_color_ = SDL_Color{250, 235, 140, 255};

            attack_duration_ = 0.11f;
            attack_cooldown_duration_ = 0.28f;
            attack_main_size_ = 68.0f;
            attack_cross_size_ = 18.0f;

            ranged_cooldown_duration_ = 0.42f;
            projectile_speed_ = 700.0f;
            projectile_lifetime_ = 1.10f;
            projectile_width_ = 34.0f;
            projectile_height_ = 10.0f;
            projectile_damage_ = 1;
            projectile_knockback_ = 300.0f;
            break;

        default:
            body_color_ = SDL_Color{80, 200, 120, 255};
            attack_color_ = SDL_Color{240, 220, 90, 255};
            projectile_color_ = SDL_Color{140, 230, 230, 255};

            attack_duration_ = 0.12f;
            attack_cooldown_duration_ = 0.25f;
            attack_main_size_ = 36.0f;
            attack_cross_size_ = 36.0f;

            ranged_cooldown_duration_ = 0.55f;
            projectile_speed_ = 620.0f;
            projectile_lifetime_ = 0.95f;
            projectile_width_ = 18.0f;
            projectile_height_ = 18.0f;
            projectile_damage_ = 1;
            projectile_knockback_ = 260.0f;
            break;
    }

    ranged_cooldown_timer_ = 0.0f;
    attack_cooldown_timer_ = 0.0f;
    attack_timer_ = 0.0f;
}

void Player::increase_move_speed(float amount) {
    speed_ = std::max(80.0f, speed_ + amount);
}

void Player::increase_max_hp(int amount) {
    if (amount <= 0) {
        return;
    }

    max_hp_ += amount;
    hp_ += amount;
}

void Player::reduce_dash_cooldown_multiplier(float multiplier) {
    dash_cooldown_duration_ =
        std::clamp(dash_cooldown_duration_ * multiplier, 0.20f, 5.0f);
}

void Player::increase_dash_speed(float amount) {
    dash_speed_ = std::max(120.0f, dash_speed_ + amount);
}

void Player::increase_attack_size(float amount) {
    attack_main_size_ = std::clamp(attack_main_size_ + amount, 16.0f, 140.0f);
    attack_cross_size_ = std::clamp(attack_cross_size_ + amount * 0.6f, 8.0f, 120.0f);
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
    attack_cooldown_timer_ = 0.0f;
    attack_duration_ = 0.12f;
    attack_cooldown_duration_ = 0.25f;
    attack_main_size_ = 36.0f;
    attack_cross_size_ = 36.0f;

    ranged_cooldown_timer_ = 0.0f;
    ranged_cooldown_duration_ = 0.55f;
    projectile_speed_ = 620.0f;
    projectile_lifetime_ = 0.95f;
    projectile_width_ = 18.0f;
    projectile_height_ = 18.0f;
    projectile_damage_ = 1;
    projectile_knockback_ = 260.0f;

    dash_timer_ = 0.0f;
    dash_cooldown_timer_ = 0.0f;
    dash_duration_ = 0.18f;
    dash_cooldown_duration_ = 0.85f;
    dash_speed_ = 860.0f;
    dash_dir_x_ = 1.0f;
    dash_dir_y_ = 0.0f;

    knockback_timer_ = 0.0f;
    knockback_velocity_x_ = 0.0f;
    knockback_velocity_y_ = 0.0f;

    post_dash_invulnerability_timer_ = 0.0f;
    damage_invulnerability_timer_ = 0.0f;
    damage_invulnerability_duration_ = 1.00f;

    hp_ = max_hp_ = 5;

    character_type_ = CharacterType::None;
    body_color_ = SDL_Color{80, 200, 120, 255};
    attack_color_ = SDL_Color{240, 220, 90, 255};
    projectile_color_ = SDL_Color{140, 230, 230, 255};
}

float Player::center_x() const {
    return x_ + size_ * 0.5f;
}

float Player::center_y() const {
    return y_ + size_ * 0.5f;
}

} // namespace ear
