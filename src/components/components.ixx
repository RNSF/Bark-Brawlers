module;

#include "common.hpp"
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <unordered_set>

export module Components;

import Transform;
import Vector2;
import Rect;
import Entity;
import Polygon;
import Animation;
import Color;
import Curve;

export {

	enum class GAME_SCREEN {
		ARENA_1,
		ARENA_2,
		ARENA_3,
		ARENA_4,
		ARENA_5,
		ARENA_6,
		ARENA_7,
		ARENA_8,
		ARENA_9,
		ARENA_10,
		ARENA_11,
		ARENA_12,
		ARENA_13,
		ARENA_14,
		ARENA_15,
		// ROUND_END_SCREEN,
		GAME_END_SCREEN,
		TITLE_SCREEN,
		CHARACTER_SELECT_SCREEN,
		ARENA_SELECT_SCREEN,
		COUNT
	};

	enum class ARENA_THEME_ID {
		FOREST,
		TOWN,
		CAVE,
		COUNT
	};

	GAME_SCREEN arenaSets[(int)ARENA_THEME_ID::COUNT][5] = {
		{GAME_SCREEN::ARENA_1, GAME_SCREEN::ARENA_2, GAME_SCREEN::ARENA_3, GAME_SCREEN::ARENA_4, GAME_SCREEN::ARENA_5},
		{GAME_SCREEN::ARENA_6, GAME_SCREEN::ARENA_7, GAME_SCREEN::ARENA_8, GAME_SCREEN::ARENA_9, GAME_SCREEN::ARENA_10},
		{GAME_SCREEN::ARENA_11, GAME_SCREEN::ARENA_12, GAME_SCREEN::ARENA_13, GAME_SCREEN::ARENA_14, GAME_SCREEN::ARENA_15},
	};

	const std::vector<std::string> GAME_SCREEN_TO_NAME = {
		"1forest.txt",
		"2forest.txt",
		"3forest.txt",
		"4forest.txt",
		"5forest.txt",
		"6city.txt",
		"7city.txt",
		"8city.txt",
		"9city.txt",
		"10city.txt",
		"11cave.txt",
		"12cave.txt",
		"13cave.txt",
		"14cave.txt",
		"15cave.txt",
	};

	std::string getArenaFilenameForScene(GAME_SCREEN scene) {
		int index = static_cast<int>(scene);
		if (index < 0 || index >= GAME_SCREEN_TO_NAME.size()) {
			return "default_arena.txt";
		}
		return GAME_SCREEN_TO_NAME[index];
	}

	enum class TEXTURE_ASSET_ID {
		TEST_DRAW,
		FIGHER_TEST,
		DAWG,
		FOREST_TILESET,
		FOREST_BACKGROUND_OLD,
		PIXEL,
		WINNER,
		ROUND_OVER,
		CIRCLE_PARTICLE_10PX,
		

		// WEAPONS
		SWORD,
		ARROW,
		BOW,
		STAFF,

		// TILESETS 
		BLACK_TILESET,
		GRASS_TILESET, // 14
		GRASS_TILESET2,
		BLACK_BRICKS_TILESET, // 16
		BLACK_BRICKS_TILESET2,
		SANDSTONE_BRICKS_TILESET,
		WOODEN_PLANKS_TILESET,
		DRY_DIRT_TILESET,
		DARK_DIRT_TILESET,
		CAVE_TILESET,
		RED_BRICKS_TILESET,

		// BACKGROUNDS
		BACKGROUND_FOREST,
		BACKGROUND_CAVE,
		BACKGROUND_TOWN,

		BACKGROUND_CHAR_SELECT,
		BACKGROUND_ARENA_SELECT,
		BACKGROUND_TITLE_BANNER,
		BACKGROUND_GAME_OVER,

		// FONTS
		FONT_BORE_BLASTERS,
		FONT_SMILEY_5X5,

		// MISC
		HEART,
		STAR,
		CIRCLE_48PX,
		INPUTS,

		// CLASSES
		KNIGHT_A,
		KNIGHT_B,
		ARCHER_A,
		ARCHER_B,
		MAGE_A,
		MAGE_B,

		COUNT
	};


	enum class EFFECT_ASSET_ID {
		COLOURED,
		TEXTURED,
		TEXTURED_OUTLINED,
		TEXTURED_INSTANCED,
		TEXTURED_INSTANCED_OUTLINED,
		SCREEN,
		COUNT
	};


	enum class GEOMETRY_BUFFER_ID {
		SPRITE,
		SCREEN_TRIANGLE,
		SWORD,
		ARROW,
		SQUARE,
		OCTAGON,
		COUNT
	};

	enum class GAME_SCREEN_ID {
		PLAYING,		
		DRAWING_MAIN,
		DRAWING_BACKGROUND,
		TILE_SELECTOR,
		TITLE_SCREEN,
		CHARACTER_SELECT,
		COUNT
	};

	const ConvexPolygon POLYGONS[(int)GEOMETRY_BUFFER_ID::COUNT] = {
		ConvexPolygon({{0, 1}, {1, 1}, {1, 0}, {0, 0}}),
		ConvexPolygon({{1, -1}, {-1, -1}, {-1, 1}, {1, 1}}),
		ConvexPolygon({{0, 1 * 5}, {3 * 5, 1 * 5}, {5 * 5, 0}, {3 * 5, -1 * 5}, {0, -1 * 5}}),
		ConvexPolygon({{-10, 1 * 3}, {-5, 1 * 3}, {0, 0}, {-5, -1 * 3}, {-10, -1 * 3}}),
		ConvexPolygon({{-0.5f, 0.5f}, {0.5f, 0.5f}, {0.5f, -0.5f}, {-0.5f, -0.5f}}),
		ConvexPolygon({{-2, -1}, {-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}})
	};

	enum class SFX_ASSET_ID {
		FOOTSTEP_1,
		FOOTSTEP_2,
		FOOTSTEP_3,
		FOOTSTEP_4,
		FOOTSTEP_5,
		SWORD_SWIPE_CRITICAL,
		SWORD_SWIPE_1,
		SWORD_SWIPE_2,
		SWORD_SWIPE_3,
		BOW_CHARGE,
		BOW_SHOOT,
		STAFF_SHOOT,
		STAFF_CHARGE,
		FIREBALL_LAND,
		EXPLOSION,
		ARROW_HIT_WALL,
		DAMAGE_1,
		//DAMAGE_2,
		//DAMAGE_3,
		DAMAGE_CRITICAL_1,
		//DAMAGE_CRITICAL_2,
		//DAMAGE_CRITICAL_3,
		DASH,
		USE_SPECIAL,
		DEATH,
		COUNT
	};

	const std::string SFX_FILES[(int)SFX_ASSET_ID::COUNT]{
		"footstep_wood_000.wav",
		"footstep_wood_001.wav",
		"footstep_wood_002.wav",
		"footstep_wood_003.wav",
		"footstep_wood_004.wav",
		"sword_swipe_critical.wav",
		"sword_swipe_1.wav",
		"sword_swipe_2.wav",
		"sword_swipe_3.wav",
		"bow_load.wav",
		"bow_shoot.wav",
		"fireball_shoot.wav",
		"staff_charge.wav",
		"fire_hit.wav",
		"explosion.wav",
		"arrow_hit_wall.wav",
		"damage1.wav",
		"critical_damage1.wav",
		"dash.wav",
		"special_dash.wav",
		"dog_bark.wav"
	};

	enum class MUSIC_ASSET_ID {
		BATTLE_1,
		COUNT
	};

	const std::string MUSIC_FILES[(int)MUSIC_ASSET_ID::COUNT]{
		"battle1.wav"
	};



	struct RenderRequest {
		TEXTURE_ASSET_ID   used_texture = TEXTURE_ASSET_ID::COUNT;
		EFFECT_ASSET_ID    used_effect = EFFECT_ASSET_ID::COUNT;
		GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::COUNT;
	};


	struct Camera {
		Vector2 position = Vector2::zero();
		Vector2 size = { 1920.0f, 1080.0f };
		Vector2 anchor = Vector2::unitCenter();
		float zoom = 1.0f;
		float rotation = 0.0f;
		bool debug = false;

		Transform getTransform(Vector2 windowSize) {
			Vector2 dimensions = size * zoom;
			Vector2 translation = position - (dimensions * anchor).rotated(rotation);

			Transform transform;

			transform.translate(translation);
			transform.scale(dimensions / windowSize);
			transform.rotate(rotation);

			return transform;
		}
	};

	enum class INPUT_ID {
		LEFT = (1 << 0),
		UP = (1 << 1),
		RIGHT = (1 << 2),
		DOWN = (1 << 3),
		ATTACK = (1 << 4),
		JUMP = (1 << 5),
		DEBUG = (1 << 6),
		SPECIAL = (1 << 7),
		PAUSE = (1 << 8),
	};

	enum class WEAPON_ID {
		SWORD,
		BOW,
		STAFF,
		COUNT,
	};

	enum class WEAPON_ANIMATION_ID {
		RESTING,
		CHARGING,
		ATTACKING,
		DASH,
		SPECIAL,
	};
	

	enum class CLASS_ID {
		KNIGHT,
		ARCHER,
		MAGE,
		// THIEF,
		COUNT,
	};



	struct Player {
		unsigned int id = 0; // is this player 0 or player 1?
		bool isFacingRight = false;
		Entity weapon;
		CLASS_ID class_id = CLASS_ID::KNIGHT;
		WalkAnimation walkAnimation;
		IdleAnimation idleAnimation;
		FallAnimation fallAnimation;
		DeathAnimation deathAnimation;
		Animation* currentAnimation = &idleAnimation;

		float dashCooldown = 0.0f;
		bool hasTouchedGroundSinceLastDash = false;
		bool isSpecialCharged = false;
		bool isSpecialActive = false;
		bool isDead = false;

		float hitFlashTime = 0.0f;
		float immunityTime = 0.0f;

		bool canDash() {
			return dashCooldown <= 0.0f && hasTouchedGroundSinceLastDash;
		}

		bool isHitFlashed() {
			return hitFlashTime > 0.0;
		}

		bool isImmune() {
			return immunityTime > 0.0f;
		}
	};

	struct WeaponAnimationState {
		Vector2 position = Vector2::zero();
		float rotation = 0.0f;
		float transitionSpeed = 0.0f;

		WeaponAnimationState lerp(WeaponAnimationState& other, float amount) {
			return {
				position.lerp(other.position, amount),
				std::lerp(rotation, other.rotation, amount)
			};
		}
	};

	struct Weapon {
		WEAPON_ID id;
		float attackTime = 0.0f;
		float attackChargeAmount = 0.0f; // ranges from 0.0 (no charge) -> 1.0 (full charge)
		float cooldownTime = 0.0f;
		float attackStrength = 0.0f;	// ranges from 0.0 (weak attack) -> 1.0 (full strength attack)
		
		WEAPON_ANIMATION_ID previousAnimationId;
		WEAPON_ANIMATION_ID animationId;
		WeaponAnimationState previousAnimationState;
		WeaponAnimationState animationState;
		float animationTransition = 0.0f;
		float rotation = 0.0f;

		int chargeSoundChannel = -1;

		bool inCooldown() {
			return cooldownTime > 0.0f;
		}

		bool isCharging() {
			return attackChargeAmount > 0.0f;
		}

		bool isAttacking() {
			return attackTime > 0.0f;
		}

		bool isReady() {
			return !inCooldown() && !isAttacking() && !isCharging();
		}

		float getCooldownTimeMax() {
			switch (id) {
			case WEAPON_ID::SWORD: return ATK_CD_KNIGHT;
			case WEAPON_ID::BOW: return ATK_CD_ARCHER;
			case WEAPON_ID::STAFF: return ATK_CD_MAGE;
			}
		}

		float getAttackChargeRate() {
			switch (id) {
			case WEAPON_ID::SWORD: return ATK_CHARGE_RATE_KNIGHT;
			case WEAPON_ID::BOW: return ATK_CHARGE_RATE_ARCHER;
			case WEAPON_ID::STAFF: return ATK_CHARGE_RATE_MAGE;
			}
		}

		float getAttackLength() {
			switch (id) {
			case WEAPON_ID::SWORD: return ATK_LEN_KNIGHT;
			case WEAPON_ID::BOW: return ATK_LEN_ARCHER;
			case WEAPON_ID::STAFF: return ATK_LEN_MAGE;
			}
		}

		float getAttackPercentage() {
			return attackTime / getAttackLength();
		}

		void transitionToAnimation(WEAPON_ANIMATION_ID newAnimationId) {
			animationTransition = 0.0f;

			previousAnimationState = animationState;
			previousAnimationId = animationId;
			animationId = newAnimationId;
		}

		void cancelAttack() {
			attackTime = 0.0f;
			cooldownTime = 0.0f;
			attackChargeAmount = 0.0f;
		}
	};

	enum class ATTACK_FLAGS {
		IS_SPECIAL = (1 << 0),
	};


	struct Attack {
		unsigned int owner = 0; // player id
		float damage = 0.0f;
		Vector2 knockback = Vector2::zero();
		Vector2 knockbackNoRotate = Vector2::zero();
		float radialKnockback = 0.0f;
		float knockbackTime = 0.0f;
		float rotation = 0.0f;
		Entity lastHitEntity;
		unsigned int flags = 0;
	};

	struct Knockable {
		float diControlPercent = 0.0f;	// varies from (0) to (1); higher value means knockback direction is determined more by di direction.
		float weight = 1.0f; // higher weight means less knockback is applied
		float knockbackTime = 0.0f;

		bool isKnocked() {
			return knockbackTime > 0.0f;
		}
	};

	struct Health {
		float health;
	};

	struct PlatformerController {
		float maxWalkSpeed = 0.0f;
		float maxVerticalSpeed = 0.0f;

		float acceleration = 0.0f;
		float jumpStrength = 0.0f;

		float minJumpTime = 0.1f;
		float jumpTime = 0.0f;

		float coyoteTimeMax = 0.2f;
		float coyoteTime = 0.0f;

		float jumpInputTimeMax = 0.2f;
		float jumpInputTime = 0.0f;

		bool isJumping = false;
		bool isJumpCancelPending = false;
	};

	struct Gravity {
		float strength = DEFAULT_GRAVITY;
	};

	struct PlayerInput {
		unsigned int controllerId = 0;

		unsigned int inputs = 0;
		unsigned int previousInputs = 0;

		unsigned int tickCount = 0; // here to prevent isJustPressed being true on first frame

		bool isPressed(INPUT_ID id) const {
			return (inputs & (int)id) > 0;
		}

		bool wasPressed(INPUT_ID id) const {
			return (previousInputs & (int)id) > 0;
		}

		bool isJustPressed(INPUT_ID id) const {
			return tickCount > 1 && isPressed(id) && !wasPressed(id);
		}

		bool isJustReleased(INPUT_ID id) const {
			return tickCount > 1 && wasPressed(id) && !isPressed(id);
		}

		void press(INPUT_ID id) {
			inputs |= (int)id;
		}

		void release(INPUT_ID id) {
			inputs &= ~((int)id);
		}

		Vector2 getAimVector() const {
			return { (float)(isPressed(INPUT_ID::RIGHT) - isPressed(INPUT_ID::LEFT)), (float)(isPressed(INPUT_ID::DOWN) - isPressed(INPUT_ID::UP)) };
		}

		void tick() {
			previousInputs = inputs;
			tickCount++;
		}

		void clear() {
			inputs = 0;
		}

		void set(INPUT_ID id, bool value) {
			if (value) press(id);
			else       release(id);
		}
	};

	enum class COLLISION_LAYER_ID {
		WALL = (1 << 0),		// layer for physics collisions
		PLAYER_HURT = (1 << 1),		// layer for combat hitbox / hurtbox
	};

	struct CollisionBody {
		unsigned int hitMask = 0;
		unsigned int hurtMask = 0;
	};

	struct CollisionBox {
		Rect bounds = Rect::unit();
	};

	struct CollisionPolygon {
		GEOMETRY_BUFFER_ID polygonId;

		const ConvexPolygon& polygon() {
			return POLYGONS[(int)polygonId];
		}
	};

	

	template <size_t LENGTH>
	struct EntityList {
		std::array<unsigned int, LENGTH> entities = {};
		unsigned int i = 0;

		// inserts only if room
		void insert(const Entity& entity) {
			for (unsigned int j = 0; j < entities.size(); j++) {
				if (entities[i] == 0) {
					entities[i] = entity;
					i = (i + 1) % LENGTH;
					return;
				}
				i = (i + 1) % LENGTH;
			}
		}

		void clear() {
			entities.fill(0);
			i = 0;
		}
	};


	struct Kinematic {
		Vector2 velocity = Vector2::zero();
		float bounciness = 0.0f;
		float slipperiness = 1.0f;
	};

	enum class COLLISION_TRACKER_FLAG {
		IS_ON_WALL = (1 << 0),
		IS_ON_GROUND = (1 << 1),
		IS_ON_CEILING = (1 << 2),
	};

	struct CollisionTracker {
		unsigned int flags = 0;

		bool isOnSurface() {
			return flags & ((int)COLLISION_TRACKER_FLAG::IS_ON_CEILING | (int)COLLISION_TRACKER_FLAG::IS_ON_WALL | (int)COLLISION_TRACKER_FLAG::IS_ON_GROUND);
		}
	};


	struct Dash {
		Vector2 velocity = Vector2::zero();
		float mainTime = 0.0f;
		float slowingTime = 0.0f;

		bool isDashing() {
			return mainTime > 0;
		}

		bool isSlowing() {
			return slowingTime > 0;
		}

		bool isActive() {
			return isDashing() || isSlowing();
		}

	};



	struct Tileset {
		TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::TEST_DRAW;
		glm::ivec2 tileCount = { 1, 1 };
	};

	struct Tile {
		unsigned int id = 0;
	};

	template <unsigned int WIDTH, unsigned int HEIGHT>
	struct Tilemap {
		std::array<Tile, WIDTH* HEIGHT> tiles;
		Vector2 tileSize = Vector2::one();
		unsigned int hurtMask = 0;

		Tile& getTile(glm::ivec2 coordinate) {
			return tiles[coordinate.y * WIDTH + coordinate.x];
		}

		Rect getTileRect(glm::ivec2 coordinate) {
			return Rect::fromTopLeftSize(tileSize * coordinate, tileSize);
		}

		unsigned int width() {
			return WIDTH;
		}

		unsigned int height() {
			return HEIGHT;
		}

		Vector2 toTileSpace(Vector2 worldPoint) {
			return worldPoint / tileSize;
		}

		bool hasTile(glm::ivec2 coordinate) {
			return coordinate.x >= 0 && coordinate.y >= 0 && coordinate.x < WIDTH && coordinate.y < HEIGHT;
		}
	};

	enum class PATH_EDGE_FLAVOR {
		FALL_WALK,
		JUMP
	};

	
	struct PathNodeEdge {
		PATH_EDGE_FLAVOR flavor;
		glm::ivec2 coordinate;
	};

	struct Path {
		std::vector<PathNodeEdge> edges;
	};


	struct PathingNode {
		std::vector<PathNodeEdge> reachableCoordinates;
	};

	template <unsigned int WIDTH, unsigned int HEIGHT>
	struct PathingGraph {
		std::array<PathingNode, WIDTH* HEIGHT> nodes;
		std::unordered_set<unsigned int> walkableCoordinates;
		Vector2 tileSize = Vector2::one();

		unsigned int getIndex(glm::ivec2 coordinate) {
			return coordinate.y * WIDTH + coordinate.x;
		}

		PathingNode& getNode(glm::ivec2 coordinate) {
			return nodes[getIndex(coordinate)];
		}

		void addWalkable(glm::ivec2 coordinate) {
			walkableCoordinates.insert(coordinate.y * WIDTH + coordinate.x);
		}

		bool isWalkable(glm::ivec2 coordinate) {
			return walkableCoordinates.contains(coordinate.y * WIDTH + coordinate.x);
		}

		glm::ivec2 getCoordinate(unsigned int i) {
			return { i % WIDTH, i / WIDTH };
		}

		unsigned int width() {
			return WIDTH;
		}

		unsigned int height() {
			return HEIGHT;
		}

		Vector2 getNodePosition(glm::ivec2 pos, Vector2 offset) {
			return (Vector2(pos) + Vector2(0.5, 0.5))* tileSize + offset;
		}

		bool hasTile(glm::ivec2 coordinate) {
			return coordinate.x >= 0 && coordinate.y >= 0 && coordinate.x < WIDTH && coordinate.y < HEIGHT;
		}

		glm::ivec2 getNodeCoordinateAt(Vector2 pos) {
			pos /= tileSize;
			return glm::ivec2(floor(pos.x), floor(pos.y));
		}

		glm::ivec2 getClosestWalkableNode(glm::ivec2 pos) {
			float minDistance = INFINITY;
			glm::ivec2 result;

			for (auto& index : walkableCoordinates) {
				glm::ivec2 pos2 = getCoordinate(index);
				float distance = Vector2(pos).distanceTo(pos2);

				if (distance < minDistance) {
					minDistance = distance;
					result = pos2;
				}
			}

			return result;
		}
	};

	enum class ARENA_BACKGROUND_ID {
		FOREST,
		CAVE,
		TOWN,
		TITLE
	};

	struct Arena {
		// map<tilesetID, tilemap>
		std::unordered_map<int, Tilemap<30, 17>> layers;
		std::vector<int> tileset_order;
		ARENA_BACKGROUND_ID arenaBackgroundId = ARENA_BACKGROUND_ID::FOREST;

		TEXTURE_ASSET_ID getBackgroundTexture() {
			switch (arenaBackgroundId) {
				case ARENA_BACKGROUND_ID::FOREST: return TEXTURE_ASSET_ID::BACKGROUND_FOREST;
				case ARENA_BACKGROUND_ID::CAVE: return TEXTURE_ASSET_ID::BACKGROUND_CAVE;
				case ARENA_BACKGROUND_ID::TOWN: return TEXTURE_ASSET_ID::BACKGROUND_TOWN;
				case ARENA_BACKGROUND_ID::TITLE: return TEXTURE_ASSET_ID::FOREST_BACKGROUND_OLD;
			}
		}

		
	};

	

	struct DeathTimer {
		float timeLeft;
	};

	struct TransformParent {
		Entity parent;
		Transform relativeTransform;
	};

	

	template <typename T>
	struct InstancedRenderVariable {
		std::vector<T> data;
		GLuint vbo = -1;

		// updates data in the shader
		void pushData() {
			assert(vbo != -1);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), GL_STATIC_DRAW);
		}
	};

	
	struct InstancedRender {
		InstancedRenderVariable<Basis> bases;
		InstancedRenderVariable<Vector2> offsets;
		InstancedRenderVariable<Rect> sections;
		InstancedRenderVariable<Color> colors;
		
		void resize(unsigned int n) {
			assert(bases.data.size() == 0 && "Do not resize after this has already been resized.");
			bases.data.resize(n);
			offsets.data.resize(n);
			sections.data.resize(n);
			colors.data.resize(n);
		}

		unsigned int instanceCount() {
			return bases.data.size();
		}
	};

	enum class DEBUG_FLAGS {
		SHOW_COLLISION_BOXES = (1 << 0),
		PLAYER_2_IS_AI = (1 << 1),
	};

	struct Debug {
		unsigned int flags = 0;
	};


	struct Dead {};

	struct Particle {
		Vector2 position = Vector2::zero();
		Vector2 velocity = Vector2::zero();

		float rotation = 0.0f;
		float angularVelocity = 0.0f;

		float totalTime = 0.0f;
		float timeLeft = 0.0f;

		bool isAlive() {
			return timeLeft > 0.0f;
		}

		float getTimePercent() {
			return 1.0f - timeLeft / totalTime;
		}
	};
	

	struct ParticleEmitter {
		std::vector<Particle> particles;

		TEXTURE_ASSET_ID textureId = TEXTURE_ASSET_ID::TEST_DRAW;
		glm::ivec2 spriteCount = {1, 1};

		std::function<Particle()> newParticle = []() { return Particle(); };
		std::function<float	(float)> getDampening			= Curve::constant; // this is a multiplier to velocity each frame (closer to 0 means more dampening, 1 is no dampending)
		std::function<float	(float)> getAngularDampening	= Curve::constant; // this is a multiplier to angular velocity each frame (closer to 0 means more dampening, 1 is no dampending)
		std::function<Vector2 (float)> getAcceleration = [](float input) { return Vector2::zero(); };
		std::function<float	(float)> getAngularAcceleration = Curve::zero;
		std::function<Color	(float)> getColor = [](float input) { return Color::white(); };
		std::function<float	(float)> getFrame = Curve::constant;

		float emmisionTime = 1.0f;
		float lifetime = 0.0f;
		bool repeats = true;
		

		unsigned int getNextParticleToEmit() {
			unsigned int n = (lifetime / emmisionTime) * particles.size();
			return repeats ? n : std::min<unsigned int>(n, particles.size() - 1);
		}
	};

	enum class ATLAS_FONT_ID {
		BORE_BLASTERS,
		SMILEY_5X5,
		COUNT
	};

	struct AtlasFont {

		TEXTURE_ASSET_ID textureId = TEXTURE_ASSET_ID::TEST_DRAW;
		glm::ivec2 glyphCount = {1, 1};
		wchar_t firstCharacter = 0;
		Rect marginSize = Rect::zero();

		static AtlasFont fromId(ATLAS_FONT_ID fontId) {
			switch (fontId) {
			case ATLAS_FONT_ID::BORE_BLASTERS: return {
				TEXTURE_ASSET_ID::FONT_BORE_BLASTERS,
				{10, 10},
				32,
				Rect::centeredSquare(2),
			};
			case ATLAS_FONT_ID::SMILEY_5X5: return {
				TEXTURE_ASSET_ID::FONT_SMILEY_5X5,
				{10, 10},
				' ',
				Rect::centeredSquare(2),
			};
			}
			return AtlasFont();
		}
	};

	struct Text {
		ATLAS_FONT_ID fontId = ATLAS_FONT_ID::BORE_BLASTERS;
		std::wstring str;
		Color color = Color::black();
		Vector2 charSpacing = Vector2::right();
		Vector2 anchor = Vector2::unitTopLeft();
		bool isPendingUpdate = false; // need to know whether to update the shader or not

		Color outlineColor = Color::clear();
		float outlineSize = 0.0f;

		void updateStr(std::wstring newStr) {
			str = newStr;
			isPendingUpdate = true;
		}
	};


	enum class TUTORIAL_STATE_ID {
		INACTIVE,
		WALK,
		JUMP,
		LONG_JUMP,
		DASH,
		ATTACK,
		CHARGE_ATTACK,
		ATTACK_UP_DOWN,
		LAND_HIT,
		SPECIAL_DASH,
		KILL,
		COUNT
	};

	enum class TUTORIAL_ACHIEVEMENT_ID {
		WALKED					= (1 << 0),
		JUMPED					= (1 << 1),
		LONG_JUMPED				= (1 << 2),
		ATTACKED				= (1 << 3),
		FULLY_CHARGED_ATTACK	= (1 << 4),
		ATTACKED_UPWARD			= (1 << 5),
		ATTACKED_DOWNWARD		= (1 << 6),
		KILLED_AI				= (1 << 7),
		DASHED					= (1 << 8),
		CHARGED_SPECIAL			= (1 << 9),
		SPECIAL_DASHED			= (1 << 10),
	};


	struct Tutorial {
		TUTORIAL_STATE_ID state = TUTORIAL_STATE_ID::INACTIVE;
		unsigned int achievements = 0;
		unsigned int stateAchievements = 0; // like achievements, but resets after state changes
		Entity textEntity;
		Entity skipTextEntity;
		Entity subTextEntity;
		bool isPendingStart = false;
		float stateTime = 0.0f;

		void start() {
			isPendingStart = true;
			achievements = 0;
			stateAchievements = 0;
		}

		void unlock(unsigned int newAchievements) {
			achievements |= newAchievements;
			stateAchievements |= newAchievements;
		}
	
		bool hasStarted() {
			return state != TUTORIAL_STATE_ID::INACTIVE;
		}
	};
	

	enum PLAYER_TYPE_ID {
		HUMAN,
		AI,
		TUTORIAL,
		COUNT
	};

	struct PlayerOptions {
		CLASS_ID selectedClass = CLASS_ID::KNIGHT;
		PLAYER_TYPE_ID playerType = PLAYER_TYPE_ID::HUMAN;

		std::wstring getPlayerTitle(int playerNumber) {
			switch (playerType) {
			case PLAYER_TYPE_ID::HUMAN: return L"Player " + std::to_wstring(playerNumber);
			case PLAYER_TYPE_ID::AI: return L"Bot";
			case PLAYER_TYPE_ID::TUTORIAL: return L"Tutorial";
			}
			return L"Unknown";
		}
	};

	struct GameScore {
		unsigned int player1Wins = 0;
		unsigned int player2Wins = 0;
		bool emphasizeP1Won = false;
		bool emphasizeP2Won = false;
	};

	struct GameState {
		GAME_SCREEN_ID currentState = GAME_SCREEN_ID::CHARACTER_SELECT;
		GAME_SCREEN currentScreen = GAME_SCREEN::CHARACTER_SELECT_SCREEN;
		GAME_SCREEN queuedScreen = GAME_SCREEN::CHARACTER_SELECT_SCREEN;
		PlayerOptions player1Options;
		PlayerOptions player2Options;
		Entity gameOverText;
		GameScore gameScore; // should probably move this to its own persistent entity
		ARENA_THEME_ID arenaTheme = ARENA_THEME_ID::FOREST;
		std::vector<GAME_SCREEN> queuedArenas;
	};

	struct Arrow {
		Entity hitbox;
		float rotation;
	};


	

	
	struct CharacterSelect {
		unsigned int playerId;
		bool hasConfirmed = false;
		IdleAnimation idleAnimation;
		Entity textEntity;
		std::vector<Entity> controlTexts;
	};

	struct Fireball {
		Vector2 direction = Vector2::right();
		float power = 1.0f;
		int bouncesLeft;
		
		Entity particle;
		Entity hitbox;
	};

	struct Explosion {
		Entity hitbox;
	};

	struct BackgroundParticle {

	};

	struct AIAgent {
		CLASS_ID class_id = CLASS_ID::KNIGHT;
		Vector2 targetPosition;
		Path path;
		int edgeIndex = 0;
		bool isTutorial = false;

		PathNodeEdge getCurrentEdge() {
			return path.edges[edgeIndex];
		};
	};

};
 
