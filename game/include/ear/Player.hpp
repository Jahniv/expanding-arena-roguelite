#pragma once

#include <SDL3/SDL.h>

namespace ear {

class Player {
public:
    void on_key_down(SDL_Keycode key, bool repeat);
    void on_key_up(SDL_Keycode key);

    void update(float dt_seconds, int window_width, int window_height);
    void render(SDL_Renderer* renderer) const;

    SDL_FRect bounds() const;
    SDL_FRect attack_bounds() const;

    bool is_attacking() const;
    bool is_dashing() const;
    bool attack_hits(const SDL_FRect& target) const;

    bool try_take_damage(int amount);
    bool is_dead() const;
    bool is_invulnerable() const;

    int hp() const;
    int max_hp() const;

    float attack_cooldown_ratio() const;
    float dash_cooldown_ratio() const;

    void reset();

    float center_x() const;
    float center_y() const;

private:
    float x_ = 120.0f;
    float y_ = 320.0f;
    float size_ = 50.0f;
    float speed_ = 300.0f;

    bool move_up_ = false;
    bool move_down_ = false;
    bool move_left_ = false;
    bool move_right_ = false;

    float facing_x_ = 1.0f;
    float facing_y_ = 0.0f;

    float attack_timer_ = 0.0f;
    float attack_cooldown_timer_ = 0.0f;
    float attack_duration_ = 0.12f;
    float attack_cooldown_duration_ = 0.25f;

    float dash_timer_ = 0.0f;
    float dash_cooldown_timer_ = 0.0f;
    float dash_duration_ = 0.14f;
    float dash_cooldown_duration_ = 0.85f;
    float dash_speed_ = 780.0f;
    float dash_dir_x_ = 1.0f;
    float dash_dir_y_ = 0.0f;

    float damage_invulnerability_timer_ = 0.0f;

    int hp_ = 5;
    int max_hp_ = 5;
};

} // namespace ear
