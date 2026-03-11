#pragma once

#include <SDL3/SDL.h>

namespace ear {

enum class EnemyType {
    Chaser,
    Brute
};

class Enemy {
public:
    Enemy(float spawn_x = 920.0f, float spawn_y = 180.0f, EnemyType type = EnemyType::Chaser);

    void update(float target_x, float target_y, float dt_seconds, float speed_multiplier = 1.0f);
    void render(SDL_Renderer* renderer) const;

    SDL_FRect bounds() const;
    void respawn();

    EnemyType type() const;
    int score_value() const;

private:
    EnemyType type_ = EnemyType::Chaser;

    float spawn_x_ = 920.0f;
    float spawn_y_ = 180.0f;

    float x_ = 920.0f;
    float y_ = 180.0f;
    float size_ = 44.0f;
    float base_speed_ = 120.0f;
};

} // namespace ear
