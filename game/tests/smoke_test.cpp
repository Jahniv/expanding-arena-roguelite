#include <catch2/catch_test_macros.hpp>
#include "ear/Player.hpp"

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
