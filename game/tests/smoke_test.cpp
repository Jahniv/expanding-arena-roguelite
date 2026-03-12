#include <catch2/catch_test_macros.hpp>
#include "ear/Player.hpp"
#include "ear/Projectile.hpp"

TEST_CASE("player starts with full HP") {
    ear::Player player;
    REQUIRE(player.hp() == player.max_hp());
}

TEST_CASE("player takes damage once and becomes invulnerable") {
    ear::Player player;

    REQUIRE(player.try_take_damage(1));
    REQUIRE(player.hp() == player.max_hp() - 1);
    REQUIRE(player.is_invulnerable());

    REQUIRE_FALSE(player.try_take_damage(1));
    REQUIRE(player.hp() == player.max_hp() - 1);
}

TEST_CASE("dash makes player temporarily invulnerable") {
    ear::Player player;

    player.on_key_down(SDLK_LSHIFT, false);

    REQUIRE(player.is_dashing());
    REQUIRE(player.is_invulnerable());
    REQUIRE_FALSE(player.try_take_damage(1));
    REQUIRE(player.hp() == player.max_hp());
}

TEST_CASE("post dash grace keeps player invulnerable briefly") {
    ear::Player player;

    player.on_key_down(SDLK_LSHIFT, false);
    player.update(0.19f, 1280, 720);

    REQUIRE_FALSE(player.is_dashing());
    REQUIRE(player.is_invulnerable());
}

TEST_CASE("player stat upgrades modify values") {
    ear::Player player;

    const float base_move_speed = player.move_speed();
    const float base_dash_speed = player.dash_speed();
    const float base_dash_cd = player.dash_cooldown_seconds();
    const float base_attack_size = player.attack_size();
    const int base_max_hp = player.max_hp();

    player.increase_move_speed(40.0f);
    player.increase_dash_speed(120.0f);
    player.reduce_dash_cooldown_multiplier(0.85f);
    player.increase_attack_size(10.0f);
    player.increase_max_hp(1);

    REQUIRE(player.move_speed() > base_move_speed);
    REQUIRE(player.dash_speed() > base_dash_speed);
    REQUIRE(player.dash_cooldown_seconds() < base_dash_cd);
    REQUIRE(player.attack_size() > base_attack_size);
    REQUIRE(player.max_hp() == base_max_hp + 1);
    REQUIRE(player.hp() == player.max_hp());
}

TEST_CASE("player can enter knockback state") {
    ear::Player player;

    player.apply_knockback(1.0f, 0.0f, 200.0f, 0.20f);

    REQUIRE(player.is_knocked_back());

    player.update(0.25f, 1280, 720);

    REQUIRE_FALSE(player.is_knocked_back());
}

TEST_CASE("player ranged attack enters cooldown") {
    ear::Player player;

    REQUIRE(player.try_begin_ranged_attack());
    REQUIRE(player.ranged_cooldown_ratio() > 0.0f);
    REQUIRE_FALSE(player.try_begin_ranged_attack());

    player.update(1.0f, 1280, 720);

    REQUIRE(player.try_begin_ranged_attack());
}

TEST_CASE("projectile expires after lifetime") {
    ear::Projectile projectile(
        0.0f, 0.0f,
        1.0f, 0.0f,
        100.0f,
        0.10f,
        10.0f,
        1,
        100.0f);

    REQUIRE(projectile.is_alive());

    projectile.update(0.20f, 1280, 720);

    REQUIRE_FALSE(projectile.is_alive());
}
