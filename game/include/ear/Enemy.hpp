#pragma once

#include <SDL3/SDL.h>

namespace ear {

class Enemy {
public:
    void update(float target_x, float target_y, float dt_seconds);
    void render(SDL_Renderer* renderer) const;

    SDL_FRect bounds() const;
    void respawn();

private:
    float x_ = 920.0f;
    float y_ = 180.0f;
    float size_ = 44.0f;
    float speed_ = 120.0f;
};

} // namespace ear
