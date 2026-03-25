#pragma once

#include <SDL3/SDL.h>

namespace ear {

enum class CharacterType {
    None,
    Hammer,
    Bow,
    Spear
};

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
    bool is_knocked_back() const;
    bool attack_hits(const SDL_FRect& target) const;

    bool try_take_damage(int amount);
    void apply_knockback(float dir_x, float dir_y, float strength, float duration = 0.16f);

    bool is_dead() const;
    bool is_invulnerable() const;

    int hp() const;
    int max_hp() const;

    float attack_cooldown_ratio() const;
    float dash_cooldown_ratio() const;
    float ranged_cooldown_ratio() const;

    float move_speed() const;
    float dash_speed() const;
    float dash_cooldown_seconds() const;
    float attack_size() const;

    bool try_begin_ranged_attack();
    float projectile_speed() const;
    float projectile_lifetime() const;
    float projectile_width() const;
    float projectile_height() const;
    int projectile_damage() const;
    float projectile_knockback() const;
    float projectile_spawn_x() const;
    float projectile_spawn_y() const;
    float facing_x() const;
    float facing_y() const;

    SDL_Color body_color() const;
    SDL_Color attack_color() const;
    SDL_Color projectile_color() const;

    CharacterType character_type() const;
    const char* character_name() const;
    void select_character(CharacterType type);

    void increase_move_speed(float amount);
    void increase_max_hp(int amount);
    void reduce_dash_cooldown_multiplier(float multiplier);
    void increase_dash_speed(float amount);
    void increase_attack_size(float amount);

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
    float attack_main_size_ = 36.0f;
    float attack_cross_size_ = 36.0f;

    float ranged_cooldown_timer_ = 0.0f;
    float ranged_cooldown_duration_ = 0.55f;
    float projectile_speed_ = 620.0f;
    float projectile_lifetime_ = 0.95f;
    float projectile_width_ = 18.0f;
    float projectile_height_ = 18.0f;
    int projectile_damage_ = 1;
    float projectile_knockback_ = 260.0f;

    float dash_timer_ = 0.0f;
    float dash_cooldown_timer_ = 0.0f;
    float dash_duration_ = 0.18f;
    float dash_cooldown_duration_ = 0.85f;
    float dash_speed_ = 860.0f;
    float dash_dir_x_ = 1.0f;
    float dash_dir_y_ = 0.0f;

    float knockback_timer_ = 0.0f;
    float knockback_velocity_x_ = 0.0f;
    float knockback_velocity_y_ = 0.0f;

    float post_dash_invulnerability_timer_ = 0.0f;
    float post_dash_invulnerability_duration_ = 0.10f;

    float damage_invulnerability_timer_ = 0.0f;
    float damage_invulnerability_duration_ = 1.00f;

    int hp_ = 5;
    int max_hp_ = 5;

    CharacterType character_type_ = CharacterType::None;
    SDL_Color body_color_{80, 200, 120, 255};
    SDL_Color attack_color_{240, 220, 90, 255};
    SDL_Color projectile_color_{140, 230, 230, 255};
};

} // namespace ear
