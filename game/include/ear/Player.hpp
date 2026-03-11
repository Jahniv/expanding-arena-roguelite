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
    bool attack_hits(const SDL_FRect& target) const;

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
    float attack_cooldown_ = 0.0f;
};

} // namespace ear
