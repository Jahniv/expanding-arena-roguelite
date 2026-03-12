#pragma once

#include <SDL3/SDL.h>

namespace ear {

class Projectile {
public:
    Projectile(
        float x,
        float y,
        float dir_x,
        float dir_y,
        float speed,
        float lifetime_seconds,
        float size,
        int damage,
        float knockback_strength,
        SDL_Color color = SDL_Color{220, 220, 220, 255});

    void update(float dt_seconds, int window_width, int window_height);
    void render(SDL_Renderer* renderer) const;

    SDL_FRect bounds() const;

    bool is_alive() const;
    void deactivate();

    int damage() const;
    float knockback_strength() const;

private:
    float x_ = 0.0f;
    float y_ = 0.0f;
    float velocity_x_ = 0.0f;
    float velocity_y_ = 0.0f;
    float size_ = 16.0f;
    float lifetime_seconds_ = 1.0f;
    int damage_ = 1;
    float knockback_strength_ = 220.0f;
    SDL_Color color_{220, 220, 220, 255};
    bool alive_ = true;
};

} // namespace ear
