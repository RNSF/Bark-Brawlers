#pragma once

#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <array>

// glfw (OpenGL)
#define NOMINMAX

#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3

using glm::vec2;

// Simple utility functions to avoid mistyping directory name
#include "../ext/project_path.hpp"
inline std::string dataPath() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shaderPath(const std::string& name) { return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name; };
inline std::string texturesPath(const std::string& name) { return dataPath() + "/textures/" + std::string(name); };
inline std::string tilesetsPath(const std::string& name) { return dataPath() + "/textures/tilesets/" + std::string(name); };
inline std::string sfxPath(const std::string& name) { return dataPath() + "/sfx/" + std::string(name); };
inline std::string musicPath(const std::string& name) { return dataPath() + "/music/" + std::string(name); };
inline std::string tilemapsPath(const std::string& name) { return dataPath() + "/tilemaps/" + std::string(name); };
inline std::string arenasPath(const std::string& name) { return dataPath() + "/arenas/" + std::string(name); };

constexpr unsigned int MAX_ROUNDS = 3;

constexpr float DEFAULT_GRAVITY = 800.0;
constexpr float COLLISION_EPSILON = 0.01;

constexpr float FIXED_FPS = 60.0f;
constexpr float FIXED_FRAME_PERIOD = 1.0f / FIXED_FPS;

constexpr int MAX_PLAYER_COUNT = 2;

constexpr unsigned int WINDOW_W = 1920 / 2;
constexpr unsigned int WINDOW_H = 1080 / 2;

constexpr unsigned int TILEMAP_W = 30;
constexpr unsigned int TILEMAP_H = 17;
constexpr unsigned int TILE_SIZE = 16;
constexpr unsigned int SMOKE_PARTICLE_MAX = 32;

// Basic Feature: [25] Game balancing
// Balance levers
// CD stand for Cooldown

constexpr float ATK_DAMAGE_BASE = 1.0f;
constexpr float ATK_DAMAGE_CHARGED = 2.0f;

constexpr float ATK_CD_KNIGHT = 0.15f;
constexpr float ATK_CD_ARCHER = 0.30f;
constexpr float ATK_CD_MAGE = 0.45f;

constexpr float ATK_CHARGE_RATE_KNIGHT = 2.0f;
constexpr float ATK_CHARGE_RATE_ARCHER = 2.0f;
constexpr float ATK_CHARGE_RATE_MAGE = 1.5f;

constexpr float ATK_LEN_KNIGHT = 0.15f;
constexpr float ATK_LEN_ARCHER = 0.0f;
constexpr float ATK_LEN_MAGE = 0.1f;

constexpr float ARROW_SPEED_MIN = 200.0f;
constexpr float ARROW_SPEED_MAX = 700.0f;

constexpr float FIREBALL_SPEED_MIN = 200.0f;
constexpr float FIREBALL_SPEED_MAX = 250.0f;
constexpr float FIREBALL_TIME_MIN = 0.3f;
constexpr float FIREBALL_TIME_MAX = 1.8f;
constexpr float FIREBALL_BOUNCE_EXTEND = 0.15f;
constexpr int FIREBALL_BOUNCE_LIMIT = 7;

constexpr float SWORD_KNOCK_DIST_MIN = 200.0f;
constexpr float SWORD_KNOCK_DIST_MAX = 400.0f;
constexpr float SWORD_KNOCK_TIME_MIN = 0.2f;
constexpr float SWORD_KNOCK_TIME_MAX = 0.4f;
constexpr float SWORD_KNOCK_TIME_SPECIAL = 0.5f;

constexpr float ARROW_KNOCK_DIST_MULT = 0.25;
constexpr float ARROW_KNOCK_TIME_MIN = 0.1f;
constexpr float ARROW_KNOCK_TIME_MAX = 0.18f;

constexpr float FIREBALL_KNOCK_DIST_MIN = 50.0f;
constexpr float FIREBALL_KNOCK_DIST_MAX = 250.0f;
constexpr float FIREBALL_KNOCK_TIME_MIN = 0.1f;
constexpr float FIREBALL_KNOCK_TIME_MAX = 0.18f;

constexpr float EXPLOSION_KNOCK_DIST = 150.0f;
constexpr float EXPLOSION_KNOCK_TIME = 0.2f;
constexpr float EXPLOSION_OFFSET = 100.0f;

constexpr float KNIGHT_DASH_MAIN = 0.07f;
constexpr float KNIGHT_DASH_SLOW = 0.05f;
constexpr float KNIGHT_DASH_SPEED = 600.0f;

constexpr float ARCHER_DASH_MAIN = 0.15f;
constexpr float ARCHER_DASH_SLOW = 0.05f;
constexpr float ARCHER_DASH_SPEED = 300.0f;

constexpr float MAGE_DASH_MAIN = 0.02f;
constexpr float MAGE_DASH_SLOW = 0.05f;
constexpr float MAGE_DASH_SPEED = 800.0f;

constexpr float MAGE_SPECIAL_DASH_MAIN = 0.003f;
constexpr float MAGE_SPECIAL_DASH_SLOW = 0.002f;
constexpr float MAGE_SPECIAL_DASH_SPEED = 0.0f;