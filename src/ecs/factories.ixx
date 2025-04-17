module;

#include <fstream>
#include <iostream>
#include <filesystem>
#include <common.hpp>
#include <numbers>

export module Factories;
import Entity;
import ECS;
import Vector2;
import Components;
import Curve;
import Range;
import RenderHelpers;
import Tiling;
import EditorHelpers;
import RenderSystem;
import Singletons;
import Ramp;
import Pathing;
import Transform;
import Color;
import Rect;

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;

export {


Entity getPlayer(int player_id) {
	for (auto& player_entity : ecs.players.entities) {
		Player& player = ecs.players.get(player_entity);
		if (player.id == player_id) return player_entity;
	}
	return Entity::null();
}

Entity createSword(Entity& parentEntity, unsigned int playerId = 0) {
	Entity weaponEntity;

	auto& weapon = ecs.weapons.emplace(weaponEntity);

	auto& transformParent = ecs.transformParents.emplace(weaponEntity);
	transformParent.parent = parentEntity;
	transformParent.relativeTransform.translate(Vector2::right() * 10);
	transformParent.relativeTransform.rotate(std::numbers::pi / 2);


	auto& transform = ecs.transforms.emplace(weaponEntity);
	transform = ecs.transforms.get(parentEntity) * transformParent.relativeTransform;

	auto& collisionPolygon = ecs.collisionPolygons.emplace(weaponEntity);
	collisionPolygon.polygonId = GEOMETRY_BUFFER_ID::SWORD;

	auto& collisionBody = ecs.collisionBodies.emplace(weaponEntity);

	auto& ignoredCollisionEntities = ecs.ignoredCollisionEntities.emplace(weaponEntity);
	ignoredCollisionEntities.insert(parentEntity);

	Attack& attack = ecs.attacks.emplace(weaponEntity);
	attack.owner = playerId;

	return weaponEntity;
}


Entity createBow(Entity parentEntity) {
	Entity weaponEntity;

	auto& weapon = ecs.weapons.emplace(weaponEntity);
	weapon.id = WEAPON_ID::BOW;

	auto& transformParent = ecs.transformParents.emplace(weaponEntity);
	transformParent.parent = parentEntity;
	transformParent.relativeTransform.translate(Vector2::right() * 10);
	transformParent.relativeTransform.rotate(std::numbers::pi / 2);


	auto& transform = ecs.transforms.emplace(weaponEntity);
	transform = ecs.transforms.get(parentEntity) * transformParent.relativeTransform;

	return weaponEntity;
}

Entity createStaff(Entity parentEntity) {
	Entity weaponEntity;

	auto& weapon = ecs.weapons.emplace(weaponEntity);
	weapon.id = WEAPON_ID::STAFF;

	auto& transformParent = ecs.transformParents.emplace(weaponEntity);
	transformParent.parent = parentEntity;
	transformParent.relativeTransform.translate(Vector2::right() * 10);
	transformParent.relativeTransform.rotate(std::numbers::pi / 2);


	auto& transform = ecs.transforms.emplace(weaponEntity);
	transform = ecs.transforms.get(parentEntity) * transformParent.relativeTransform;

	return weaponEntity;
}

Entity createPlayerInput(int id) {
	Entity entity;

	auto& playerInput = ecs.playerInputs.emplace(entity);
	playerInput.controllerId = id;

	return entity;
}

Entity createPlayer(int id, Vector2 position, CLASS_ID class_id) {
	Entity playerEntity;

	auto& player = ecs.players.emplace(playerEntity);
	player.id = id;
	player.class_id = class_id;
	
	auto& health = ecs.healths.emplace(playerEntity);
	health.health = 10.0f;

	auto& transform = ecs.transforms.emplace(playerEntity);
	transform.setTranslation(position);

	auto& kinematic = ecs.kinematics.emplace(playerEntity);

	auto& playerInput = ecs.playerInputs.emplace(playerEntity);
	playerInput.controllerId = id;

	auto& platformerController = ecs.platformerControllers.emplace(playerEntity);
	platformerController.acceleration = 20.0f;
	platformerController.jumpStrength = 330.0f;
	platformerController.maxWalkSpeed = 155.0f;

	auto& collisionBox = ecs.collisionBoxes.emplace(playerEntity);
	collisionBox.bounds = { -6, -11, 6, 11 };

	auto& collisionBody = ecs.collisionBodies.emplace(playerEntity);
	collisionBody.hitMask |= (int)COLLISION_LAYER_ID::WALL;
	collisionBody.hurtMask |= (int)COLLISION_LAYER_ID::PLAYER_HURT;

	auto& collisionTracker = ecs.collisionTrackers.emplace(playerEntity);

	auto& knockable = ecs.knockables.emplace(playerEntity);
	knockable.weight = 1.0f;

	auto& gravity = ecs.gravities.emplace(playerEntity);

	auto& dash = ecs.dashes.emplace(playerEntity);

	
	switch (class_id) {
	case CLASS_ID::KNIGHT: {
		player.weapon = createSword(playerEntity, id);
	} break;
	case CLASS_ID::ARCHER: {
		player.weapon = createBow(playerEntity);
	} break;
	case CLASS_ID::MAGE: {
		player.weapon = createStaff(playerEntity);
	}
	}
	

	return playerEntity;
};



Entity createArrow(Vector2 position, Vector2 velocity, unsigned int playerId, float damage, unsigned int attackFlags = 0) {
	Entity arrowEntity;

	auto& transform = ecs.transforms.emplace(arrowEntity);
	transform.setTranslation(position);

	auto& kinematic = ecs.kinematics.emplace(arrowEntity);
	kinematic.velocity = velocity;
	kinematic.slipperiness = 0.0f;

	auto& collisionBox = ecs.collisionBoxes.emplace(arrowEntity);
	collisionBox.bounds = Rect(-1, -1, 1, 1);

	

	auto& collisionBody = ecs.collisionBodies.emplace(arrowEntity);
	collisionBody.hitMask |= (int)COLLISION_LAYER_ID::WALL;
	

	auto& collisionTracker = ecs.collisionTrackers.emplace(arrowEntity);

	auto& gravity = ecs.gravities.emplace(arrowEntity);
	gravity.strength = DEFAULT_GRAVITY / 2;

	auto& arrow = ecs.arrows.emplace(arrowEntity);

	// HITBOX
	Entity hitboxEntity; {
		
		if (abs(transform.matrix[0][0]) > 10000)
			ecs.list_all_components_of(arrowEntity);

		Transform& hitboxTransform = ecs.transforms.emplace(hitboxEntity);
		hitboxTransform = ecs.transforms.get(arrowEntity); // have to fetch again since adding another transform could invalidate the old transform reference
		
		auto& transformParent = ecs.transformParents.emplace(hitboxEntity);
		transformParent.parent = arrowEntity;


		auto& collisionPolygon = ecs.collisionPolygons.emplace(hitboxEntity);
		collisionPolygon.polygonId = GEOMETRY_BUFFER_ID::ARROW;

		auto& ignoredCollision = ecs.ignoredCollisionEntities.emplace(hitboxEntity);

		auto& attack = ecs.attacks.emplace(hitboxEntity);
		attack.owner = playerId;
		attack.damage = damage;
		attack.flags = attackFlags;

		auto& collisionBody = ecs.collisionBodies.emplace(hitboxEntity);
		collisionBody.hitMask |= (int)COLLISION_LAYER_ID::PLAYER_HURT;

	}

	arrow.hitbox = hitboxEntity;

	return arrowEntity;
}

Entity createParticleEmitter(unsigned int count) {
	Entity particleEntity;

	auto& instancedRender = ecs.instancedRenders.emplace(particleEntity);
	instancedRender.resize(count);

	for (unsigned int i = 0; i < count; i++) {
		instancedRender.colors.data[i] = Color::clear();
	}

	auto& transform = ecs.transforms.emplace(particleEntity);
	ParticleEmitter& particleEmitter = ecs.particleEmitters.emplace(particleEntity);
	particleEmitter.particles.resize(count);

	return particleEntity;
}


Entity createExplosionDebrisEmitter(Vector2 position) {
	Entity particleEntity = createParticleEmitter(256);

	ParticleEmitter& particleEmitter = ecs.particleEmitters.get(particleEntity);
	particleEmitter.textureId = TEXTURE_ASSET_ID::CIRCLE_PARTICLE_10PX;
	particleEmitter.spriteCount = { 5, 1 };

	particleEmitter.newParticle = []() {
		Particle particle;

		Vector2 direction = Vector2::right().rotated(Range(0, 2 * std::numbers::pi).randomPoint());

		particle.position = Vector2::right().rotated(Range(0, 2 * std::numbers::pi).randomPoint()) * sqrt(Range(0, 24 * 24).randomPoint());
		particle.velocity = direction * Range(20, 30).randomPoint();
		particle.totalTime = Range(1, 1.2).randomPoint();
		particle.timeLeft = particle.totalTime;

		return particle;
		};
	particleEmitter.getDampening = [](float input) { return 0.9f; };
	particleEmitter.getAcceleration = [](float input) { return Vector2::up() * 100.0f; };
	particleEmitter.getColor = [](float input) { 
		static const ColorRamp ramp = ColorRamp<5>({ 0.0, 0.1, 0.2, 0.6, 1.0 }, { Color::yellow(), Color::orange(), Color::red(), Color::gray(0.2), Color::clear()});
		return  ramp.getAt(input);
	};
	particleEmitter.getFrame = Curve::linear;

	particleEmitter.emmisionTime = 0.1f;
	particleEmitter.repeats = false;

	auto& deathTimer = ecs.deathTimers.emplace(particleEntity);
	deathTimer.timeLeft = 3.0f;

	auto& transform = ecs.transforms.get(particleEntity);
	transform.translate(position);

	return particleEntity;
}

Entity createFireballEmitter(Entity& fireballEntity) {
	Entity particleEntity = createParticleEmitter(256);

	ParticleEmitter& particleEmitter = ecs.particleEmitters.get(particleEntity);
	particleEmitter.textureId = TEXTURE_ASSET_ID::CIRCLE_PARTICLE_10PX;
	particleEmitter.spriteCount = { 5, 1 };


	particleEmitter.newParticle = [fireballEntity]() {
		Particle particle;
		Fireball& fireball = ecs.fireballs.get(fireballEntity);
		/*Transform& transform = ecs.transforms.get(fireballEntity);
		Kinematic& kinematic = ecs.kinematics.get(fireballEntity);

		particle.position = transform.getTranslation();
		particle.velocity = fireball.direction.rotated(Range(-0.3, 0.3).randomPoint()) * Range(10, 20).randomPoint() + kinematic.velocity;*/
		particle.position = Vector2::zero();
		particle.velocity = fireball.direction.rotated(Range(-0.3, 0.3).randomPoint()) * Range(10, 20).randomPoint();
		particle.totalTime = Range(0.6, 0.7).randomPoint() * sqrt(fireball.power);
		particle.timeLeft = particle.totalTime;

		return particle;
	};

	particleEmitter.getDampening = [](float input) { return 0.9f; };
	particleEmitter.getAcceleration = [fireballEntity](float input) {
		Fireball& fireball = ecs.fireballs.get(fireballEntity);
		return fireball.direction * 13.0f;
	};

	particleEmitter.getColor = [](float input) {
		static const ColorRamp ramp = ColorRamp<5>({ 0.0, 0.2, 0.4, 0.8, 1.0 }, { Color::white(), Color::yellow(), Color::orange(), Color::red(), Color::gray(0.2) });
		return  ramp.getAt(input);
	};

	particleEmitter.getFrame = [fireballEntity](float input) {
		Fireball& fireball = ecs.fireballs.get(fireballEntity);
		return (1.0 - fireball.power) + fireball.power * input;
	};

	particleEmitter.emmisionTime = 1.2f;

	return particleEntity;
}


Entity createFireball(Vector2 position, Vector2 velocity, unsigned int playerId, float damage, float lifetime, RenderSystem& renderSystem, unsigned int attackFlags = 0) {
	Entity fireballEntity;

	auto& transform = ecs.transforms.emplace(fireballEntity);
	transform.setTranslation(position);

	auto& kinematic = ecs.kinematics.emplace(fireballEntity);
	kinematic.velocity = velocity;
	kinematic.slipperiness = 1.0f;
	kinematic.bounciness = 1.0f;

	auto& collisionBox = ecs.collisionBoxes.emplace(fireballEntity);
	collisionBox.bounds = Rect(-1, -1, 1, 1);

	auto& collisionBody = ecs.collisionBodies.emplace(fireballEntity);
	collisionBody.hitMask |= (int)COLLISION_LAYER_ID::WALL;

	auto& collisionTracker = ecs.collisionTrackers.emplace(fireballEntity);

	auto& fireball = ecs.fireballs.emplace(fireballEntity);

	assert(Range(0.0f, FIREBALL_TIME_MAX).hasPointInc(lifetime));
	auto& deathTimer = ecs.deathTimers.emplace(fireballEntity);
	deathTimer.timeLeft = lifetime;

	// HITBOX
	Entity hitboxEntity; {
		Transform& hitboxTransform = ecs.transforms.emplace(hitboxEntity);
		hitboxTransform = ecs.transforms.get(fireballEntity); // have to fetch again since adding another transform could invalidate the old transform reference

		auto& transformParent = ecs.transformParents.emplace(hitboxEntity);
		transformParent.parent = fireballEntity;


		auto& collisionPolygon = ecs.collisionPolygons.emplace(hitboxEntity);
		collisionPolygon.polygonId = GEOMETRY_BUFFER_ID::OCTAGON;

		auto& ignoredCollision = ecs.ignoredCollisionEntities.emplace(hitboxEntity);

		auto& attack = ecs.attacks.emplace(hitboxEntity);
		attack.owner = playerId;
		attack.damage = damage;
		attack.flags = attackFlags;

		auto& collisionBody = ecs.collisionBodies.emplace(hitboxEntity);
		collisionBody.hitMask |= (int)COLLISION_LAYER_ID::PLAYER_HURT;

	}

	Entity particleEntity; {
		particleEntity = createFireballEmitter(fireballEntity);

		Transform& particleTransform = ecs.transforms.get(particleEntity);
		particleTransform = ecs.transforms.get(fireballEntity); // have to fetch again since adding another transform could invalidate the old transform reference

		auto& transformParent = ecs.transformParents.emplace(particleEntity);
		transformParent.parent = fireballEntity;
	}

	fireball.bouncesLeft = FIREBALL_BOUNCE_LIMIT;
	fireball.hitbox = hitboxEntity;
	fireball.particle = particleEntity;
	fireball.direction = Vector2::left();
	renderSystem.initInstancedRender(ecs.instancedRenders.get(fireball.particle));

	return fireballEntity;
}


Entity createExplosion(Vector2 position, unsigned int playerId, RenderSystem& renderSystem) {
	Entity explosionEntity = createExplosionDebrisEmitter(position);

	Entity hitboxEntity;  {
		auto& transform = ecs.transforms.emplace(hitboxEntity);
		transform.translate(position);
		transform.scale(Vector2::one() * 12);

		auto& collisionPolygon = ecs.collisionPolygons.emplace(hitboxEntity);
		collisionPolygon.polygonId = GEOMETRY_BUFFER_ID::OCTAGON;

		auto& collisionBody = ecs.collisionBodies.emplace(hitboxEntity);
		collisionBody.hitMask |= (int)COLLISION_LAYER_ID::PLAYER_HURT;

		auto& deathTimer = ecs.deathTimers.emplace(hitboxEntity);
		deathTimer.timeLeft = 0.1f;

		auto& ignoredCollisionEntities = ecs.ignoredCollisionEntities.emplace(hitboxEntity);
		ignoredCollisionEntities.insert(getPlayer(playerId));


		Attack& attack = ecs.attacks.emplace(hitboxEntity);
		attack.damage = ATK_DAMAGE_BASE;
		attack.radialKnockback = EXPLOSION_KNOCK_DIST;
		attack.knockbackTime = EXPLOSION_KNOCK_TIME;
		attack.owner = playerId;
		attack.flags |= (int)ATTACK_FLAGS::IS_SPECIAL;
	};

	

	Explosion& explosion = ecs.explosions.emplace(explosionEntity);
	explosion.hitbox = hitboxEntity;
	renderSystem.initInstancedRender(ecs.instancedRenders.get(explosionEntity));

	return explosionEntity;
}

void saveArena(const string& filename) 
{
	if (ecs.arenas.size() == 0) {
		cout << "No arenas exist" << endl; // Should never happen
	}
	auto arena = ecs.arenas.components[0]; // Assuming only 1 arena exists at a time

	std::ofstream file(filename);
	if (!file) {
		std::cerr << "Error: Could not open file " << filename << " for writing.\n";
		return;
	}

	file << "Background: " << (int) arena.arenaBackgroundId << endl << endl;

	for (int layer = 0; layer < arena.tileset_order.size(); layer++) 
	{
		int tilesetID = arena.tileset_order[layer];
		if (arena.layers.find(tilesetID) == arena.layers.end()) {
			cerr << "Layer " << layer << " for tileset " << tilesetID << " not found" << endl;
			continue;
		}

		auto& tilemap = arena.layers[tilesetID];

		file << "Tileset: " << tilesetID << " (Layer " << layer << ")\n";

		for (int y = 0; y < tilemap.height(); y++) {
			for (int x = 0; x < tilemap.width(); x++) {
				file << tilemap.tiles[y * tilemap.width() + x].id << " ";
			}
			file << "\n";
		}
		file << "\n";
	}

	file.close();
	std::cout << "Arena saved successfully to " << filename << "\n";
}

Arena loadArena(const std::string& filename) {
	std::ifstream file(filename);
	if (!file) {
		cerr << "Error: Could not open arena file " << filename << endl;
		return Arena();
	}

	Arena arena;
	std::string line;

	while (std::getline(file, line)) {
		if (line.find("Background:") == 0) {
			int backgroundId;
			sscanf(line.c_str(), "Background: %d", &backgroundId);

			arena.arenaBackgroundId = (ARENA_BACKGROUND_ID)backgroundId;
		}
		if (line.find("Tileset:") == 0) {
			int tilesetID, layerID;
			sscanf(line.c_str(), "Tileset: %d (Layer %d)", &tilesetID, &layerID);

			Tilemap<30, 17> tilemap;

			for (int y = 0; y < tilemap.height(); ++y) {
				for (int x = 0; x < tilemap.width(); ++x) {
					file >> tilemap.tiles[y * tilemap.width() + x].id;
				}
			}
			arena.layers[tilesetID] = tilemap;
			arena.tileset_order.push_back(tilesetID);
		}
	}

	

	file.close();
	std::cout << "Arena loaded successfully from " << filename << "\n";
	return arena;
}

void printArena()
{
	if (ecs.arenas.size() == 0) {
		cout << "No arenas exist" << endl; // Should never happen
		return;
	}
	auto arena = ecs.arenas.components[0]; // Assuming only 1 arena exists at a time

	for (int layer = 0; layer < arena.tileset_order.size(); layer++)
	{
		int tilesetID = arena.tileset_order[layer];
		if (arena.layers.find(tilesetID) == arena.layers.end()) {
			cerr << "Layer " << layer << " for tileset " << tilesetID << " not found" << endl;
			continue;
		}

		auto& tilemap = arena.layers[tilesetID];

		cout << "Tileset: " << tilesetID << " (Layer " << layer << ")\n";

		for (int y = 0; y < tilemap.height(); y++) {
			for (int x = 0; x < tilemap.width(); x++) {
				cout << tilemap.tiles[y * tilemap.width() + x].id << " ";
			}
			cout << "\n";
		}
		cout << "\n";
	}
}

void printTilemap() {
	auto tilemap = ecs.tilemaps.components[0]; // assuming only 1 tilemap at a time
	for (int y = 0; y < tilemap.height(); ++y) {
		for (int x = 0; x < tilemap.width(); ++x) {
			cout << tilemap.tiles[y * tilemap.width() + x].id << " ";
		}
		cout << endl;
	}
}

Entity createArena(string filename) {
	Entity arenaEntity;

	auto arena = loadArena(arenasPath(filename)); // Loading this map on startup
	ecs.arenas.emplace(arenaEntity, arena);

	for (auto& pair : arena.layers) {
		auto& tilesetID = pair.first;
		auto& tilemap = pair.second;
		Entity tilemapEntity;

		tilemap.hurtMask |= (int)COLLISION_LAYER_ID::WALL;
		tilemap.tileSize = Vector2::one() * 16;
		ecs.tilemaps.emplace(tilemapEntity, tilemap);

		auto& tileset = ecs.tilesets.emplace(tilemapEntity);
		tileset.tileCount = Tiling::tilesetSize.at(tilesetID);
		tileset.texture = static_cast<TEXTURE_ASSET_ID>(tilesetID);

		auto& transform = ecs.transforms.emplace(tilemapEntity);
		transform.translate(-Vector2(tilemap.width(), tilemap.height()) * tilemap.tileSize * 0.5f);

		auto& instancedRender = ecs.instancedRenders.emplace(tilemapEntity);
		instancedRender.resize(TILEMAP_H * TILEMAP_W);


		updateTilemap(tilemapEntity);
	}

	Entity pathingGraph = createPathingGraph();

	return arenaEntity;
};

Entity createAttackHitbox(Entity& parentEntity, Vector2 relativePosition, Vector2 size, float lifeTime) {
	Entity attackHitbox;

	auto& transformParent = ecs.transformParents.emplace(attackHitbox);
	transformParent.parent = parentEntity;
	transformParent.relativeTransform.translate(relativePosition);
	transformParent.relativeTransform.rotate(relativePosition.angle());
	transformParent.relativeTransform.scale(size);
	

	auto& transform = ecs.transforms.emplace(attackHitbox);
	transform = ecs.transforms.get(parentEntity) * transformParent.relativeTransform;

	auto& collisionPolygon = ecs.collisionPolygons.emplace(attackHitbox);
	collisionPolygon.polygonId = GEOMETRY_BUFFER_ID::SWORD;

	auto& collisionBody = ecs.collisionBodies.emplace(attackHitbox);
	collisionBody.hitMask |= (int) COLLISION_LAYER_ID::PLAYER_HURT;

	auto& deathTimer = ecs.deathTimers.emplace(attackHitbox);
	deathTimer.timeLeft = lifeTime;

	auto& ignoredCollisionEntities = ecs.ignoredCollisionEntities.emplace(attackHitbox);
	ignoredCollisionEntities.insert(parentEntity);

	Attack& attack = ecs.attacks.emplace(attackHitbox);
	attack.damage = 1.0f;
	attack.knockback = Vector2::right() * size.x * size.y * 4;
	attack.knockbackNoRotate = Vector2::up() * std::abs(relativePosition.normalize().dot(Vector2::right())) * size.x * size.y * 4;
	attack.knockbackTime = size.x * 0.05f;
	

	return attackHitbox;
}








Entity createSmokeParticleEmitter() {
	Entity particleEntity = createParticleEmitter(512);
	
	ParticleEmitter& particleEmitter = ecs.particleEmitters.get(particleEntity);
	particleEmitter.textureId = TEXTURE_ASSET_ID::CIRCLE_PARTICLE_10PX;
	particleEmitter.spriteCount = { 5, 1 };

	particleEmitter.newParticle = []() { 
		Particle particle;

		particle.position = Vector2(Range(-200, 200).randomPoint(), 130);
		particle.velocity = Vector2::up().rotated(Range(-1,1).randomPoint()) * Range(20, 30).randomPoint();
		particle.totalTime = Range(1, 1.2).randomPoint();
		particle.timeLeft = particle.totalTime;

		return particle;
	};
	particleEmitter.getDampening = [](float input) { return 0.9f; };
	particleEmitter.getAcceleration = [](float input) { return Vector2::up() * 100.0f; };
	particleEmitter.getColor = [](float input) { return Color::black().lerp(Color::white(), 0.1f).lerp(Color(1.0f, 1.0f, 1.0f, 0.0f), input); };
	particleEmitter.getFrame = Curve::linear;

	particleEmitter.emmisionTime = 1.2f;
	
	BackgroundParticle backgroundParticle = ecs.backgroundParticles.emplace(particleEntity);
	
	return particleEntity;
}


// if maxCharacterCount is 0, it will be set to the length of the str
Entity createText(std::wstring str, ATLAS_FONT_ID fontId, Transform trans, Color color, Vector2 anchor, Color outlineColor = Color::clear(), float outlineSize = 0.0f, unsigned int maxCharacterCount = 0) {
	maxCharacterCount = maxCharacterCount == 0 ? str.length() : maxCharacterCount;
	assert(str.length() <= maxCharacterCount);

	Entity textEntity;

	auto& transform = ecs.transforms.emplace(textEntity);
	transform = trans;

	auto& instancedRender = ecs.instancedRenders.emplace(textEntity);
	instancedRender.resize(maxCharacterCount);

	auto& text = ecs.texts.emplace(textEntity);
	text.fontId = fontId;
	text.updateStr(str);
	text.color = color;
	text.anchor = anchor;
	text.outlineColor = outlineColor;
	text.outlineSize = outlineSize;

	return textEntity;
}


//void resetPlayerPositions() {
//	// reset player1 and 2 positions
//	ecs.transforms.get(getPlayer(1)).setTranslation({ -100, 0 });
//	ecs.transforms.get(getPlayer(2)).setTranslation({ 100, 0 });
//}

//void changePlayerClass(Entity player, CLASS_ID newClass) {
//	auto& _player = ecs.players.get(player);
//	auto& transform = ecs.transforms.get(player);
//	// remove old weapon
//	ecs.weapons.remove(_player.weapon);
//	switch (newClass) {
//	case CLASS_ID::KNIGHT: {
//		_player.weapon = createSword(player, _player.id);
//	} break;
//	case CLASS_ID::ARCHER: {
//		_player.weapon = createBow(player);
//	} break;
//	}
//	_player.class_id = newClass;
//}


Entity createInputEntity(unsigned int controllerId) {
	Entity inputEntity;

	auto& playerInput = ecs.playerInputs.emplace(inputEntity);
	playerInput.controllerId = controllerId;

	return inputEntity;
}


Entity createCharacterSelectUI(unsigned int playerId, Vector2 position, RenderSystem& renderSystem) {
	Entity entity;

	auto& transform = ecs.transforms.emplace(entity);
	transform.setTranslation(position);
	transform.scale(Vector2(2.f, 2.f));

	auto& characterSelect = ecs.characterSelects.emplace(entity);
	characterSelect.playerId = playerId;

	auto& playerInput = ecs.playerInputs.emplace(entity);
	playerInput.controllerId = playerId;

	auto& playerOptions = ecs.playerOptions.emplace(entity);

	if (playerId == 1) playerOptions = gameState.player1Options;
	if (playerId == 2) playerOptions = gameState.player2Options;

	if (playerId == 2) {
		if (!isTutorialCompleted)
			playerOptions.playerType = PLAYER_TYPE_ID::TUTORIAL;
		if (isTutorialCompleted && playerOptions.playerType == PLAYER_TYPE_ID::TUTORIAL)
			playerOptions.playerType = PLAYER_TYPE_ID::AI;
	}

	
	characterSelect.textEntity = createText(L"Player " + std::to_wstring(playerId), ATLAS_FONT_ID::BORE_BLASTERS, Transform::withTranslation(Vector2(position.x - 5, position.y + Vector2::up().y * 85)), Color::black(), Vector2(0.5, 0.5));
	renderSystem.initInstancedRender(ecs.instancedRenders.get(characterSelect.textEntity));

	ecs.texts.get(characterSelect.textEntity).updateStr(playerOptions.getPlayerTitle(characterSelect.playerId));
	
	Vector2 inputPos = transform.getTranslation() + Vector2(-44 + 32, 42);
	Vector2 spacing = Vector2(13, 15);

	Entity textEntity;
	textEntity = createText(L"Move / Aim", ATLAS_FONT_ID::SMILEY_5X5, Transform::withTranslation(inputPos + Vector2::down() * spacing * 0), Color::black(), Vector2(0, 0.5));
	renderSystem.initInstancedRender(ecs.instancedRenders.get(textEntity));
	characterSelect.controlTexts.push_back(textEntity);

	textEntity = createText(L"Jump", ATLAS_FONT_ID::SMILEY_5X5, Transform::withTranslation(inputPos + Vector2::down() * spacing * 1), Color::black(), Vector2(0, 0.5));
	renderSystem.initInstancedRender(ecs.instancedRenders.get(textEntity));
	characterSelect.controlTexts.push_back(textEntity);

	textEntity = createText(L"Attack", ATLAS_FONT_ID::SMILEY_5X5, Transform::withTranslation(inputPos + Vector2::down() * spacing * 2), Color::black(), Vector2(0, 0.5));
	renderSystem.initInstancedRender(ecs.instancedRenders.get(textEntity));
	characterSelect.controlTexts.push_back(textEntity);

	textEntity = createText(L"Dash", ATLAS_FONT_ID::SMILEY_5X5, Transform::withTranslation(inputPos + Vector2::down() * spacing * 3), Color::black(), Vector2(0, 0.5));
	renderSystem.initInstancedRender(ecs.instancedRenders.get(textEntity));
	characterSelect.controlTexts.push_back(textEntity);

	return entity;
}

void createScene(GAME_SCREEN scene, RenderSystem& renderSystem) {
	// Any scene specific entities can be instantiated here
	switch (scene) {
	case GAME_SCREEN::CHARACTER_SELECT_SCREEN: {

		

		gameState.currentState = GAME_SCREEN_ID::CHARACTER_SELECT;

		createCharacterSelectUI(1, Vector2(-103, 25), renderSystem);
		createCharacterSelectUI(2, Vector2(104, 25), renderSystem);

		Entity textEntity = createText(L"[Jump] to lock in", ATLAS_FONT_ID::SMILEY_5X5, Transform::withTranslation(Vector2::down() * 90), Color::green(), Vector2::unitCenter(), Color::black(), 1);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(textEntity));

	} break;
	case GAME_SCREEN::ARENA_SELECT_SCREEN: {
		gameState.currentState = GAME_SCREEN_ID::CHARACTER_SELECT;

		Entity textEntity;
		textEntity = createText(L"Choose Arena", ATLAS_FONT_ID::BORE_BLASTERS, Transform::withTranslation(Vector2::up() * 100), Color::white(), Vector2::unitCenter(), Color::black(), 1);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(textEntity));

		createInputEntity(1);
		createInputEntity(2);


		textEntity = createText(L"Confirm ", ATLAS_FONT_ID::SMILEY_5X5, Transform::withTranslation(Vector2::down() * 110), Color::white(), Vector2::unitCenter(), Color::black(), 1);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(textEntity));
		

	} break;
	case GAME_SCREEN::ARENA_1:
	case GAME_SCREEN::ARENA_2:
	case GAME_SCREEN::ARENA_3:
	case GAME_SCREEN::ARENA_4:
	case GAME_SCREEN::ARENA_5: 
	case GAME_SCREEN::ARENA_6:
	case GAME_SCREEN::ARENA_7:
	case GAME_SCREEN::ARENA_8:
	case GAME_SCREEN::ARENA_9:
	case GAME_SCREEN::ARENA_10:
	case GAME_SCREEN::ARENA_11:
	case GAME_SCREEN::ARENA_12:
	case GAME_SCREEN::ARENA_13:
	case GAME_SCREEN::ARENA_14:
	case GAME_SCREEN::ARENA_15:
	{

		

		Entity player1Entity = createPlayer(1, { -100, 0 }, gameState.player1Options.selectedClass);
		renderSystem.setPlayer1(player1Entity);

		Entity player2Entity = createPlayer(2, { 100, 0 }, gameState.player2Options.selectedClass);
		renderSystem.setPlayer2(player2Entity);

		if (gameState.player1Options.playerType == PLAYER_TYPE_ID::AI &&
			gameState.player1Options.selectedClass == CLASS_ID::KNIGHT) {
			AIAgent& ai = ecs.aiAgents.emplace(player1Entity);
			ai.class_id = ecs.players.get(player1Entity).class_id;
		}

		if ((gameState.player2Options.playerType == PLAYER_TYPE_ID::AI || gameState.player2Options.playerType == PLAYER_TYPE_ID::TUTORIAL) &&
			gameState.player2Options.selectedClass == CLASS_ID::KNIGHT) {
			AIAgent& ai = ecs.aiAgents.emplace(player2Entity);
			ai.class_id = ecs.players.get(player2Entity).class_id;
			ai.isTutorial = gameState.player2Options.playerType == PLAYER_TYPE_ID::TUTORIAL;
		}

		if (gameState.player2Options.playerType == PLAYER_TYPE_ID::TUTORIAL) {
			tutorial.start();
		}
		
		Entity arenaEntity = createArena(getArenaFilenameForScene(scene));

		for (auto& tilemap : ecs.tilemaps.entities) {
			auto& instancedRender = ecs.instancedRenders.get(tilemap);
			renderSystem.initInstancedRender(instancedRender);
		}

		if (ecs.arenas.get(arenaEntity).arenaBackgroundId == ARENA_BACKGROUND_ID::CAVE) {
			Entity smoke = createSmokeParticleEmitter();
			renderSystem.initInstancedRender(ecs.instancedRenders.get(smoke));
		}
		
	} break;
	
	case GAME_SCREEN::GAME_END_SCREEN: {
		isTutorialCompleted = true;

		Entity player1Input = createPlayerInput(1);
		Entity player2Input = createPlayerInput(2);

		Entity arenaEntity = createArena("border.txt");
		for (auto& tilemap : ecs.tilemaps.entities) {
			auto& instancedRender = ecs.instancedRenders.get(tilemap);
			renderSystem.initInstancedRender(instancedRender);
		}

		Transform transform = Transform::withTranslation(Vector2(0, -100));
		transform.scale({2, 2});

		std::wstring whoWon;
		if (gameState.gameScore.player1Wins > gameState.gameScore.player2Wins) {
			whoWon = L"P1";
		}
		else {
			whoWon = L"P2";
		}

		Entity titleEntity = createText(L"WINNER " + whoWon,
			ATLAS_FONT_ID::BORE_BLASTERS,
			transform,
			Color::black(),
			Vector2(0.5f, 0.5f),
			Color::white(),
		2.0f);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(titleEntity));

		transform = Transform::withTranslation(Vector2(0, 100));
		titleEntity = createText(L"PRESS ANY KEY TO CONTINUE",
			ATLAS_FONT_ID::SMILEY_5X5,
			transform,
			Color::black(),
			Vector2(0.5f, 0.5f),
			Color::white(),
		0);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(titleEntity));

	} break;
	case GAME_SCREEN::TITLE_SCREEN: {
		gameState.currentState = GAME_SCREEN_ID::TITLE_SCREEN;
		Entity arenaEntity = createArena("title.txt");

		Entity player1Entity = createPlayer(1, { -100, 0 }, CLASS_ID::KNIGHT);
		Entity player2Entity = createPlayer(2, { 100, 0 }, CLASS_ID::KNIGHT);

		AIAgent& ai1 = ecs.aiAgents.emplace(player1Entity);
		ai1.class_id = CLASS_ID::KNIGHT;

		AIAgent& ai2 = ecs.aiAgents.emplace(player2Entity);
		ai2.class_id = CLASS_ID::KNIGHT;

		for (auto& tilemap : ecs.tilemaps.entities) {
			auto& instancedRender = ecs.instancedRenders.get(tilemap);
			renderSystem.initInstancedRender(instancedRender);
		}

		Entity titleEntity = createText(L"BARK BRAWLERS",
			ATLAS_FONT_ID::BORE_BLASTERS,
			Transform::withTranslation(Vector2(0, -65), Vector2(1.7f, 1.7f)),
			Color::white(),
			Vector2(0.5f, 0.5f),
			Color::black(),
			2.0f);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(titleEntity));

		Entity startText = createText(L"Any Key to Start",
			ATLAS_FONT_ID::SMILEY_5X5,
			Transform::withTranslation(Vector2(0, 117), Vector2(1.0f, 1.0f)),
			Color::white(),
			Vector2(0.5f, 0.5f),
			Color::black(),
			1.0f);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(startText));

		/*Entity exitText = createText(L"[ESC] Exit",
			ATLAS_FONT_ID::SMILEY_5X5,
			Transform::withTranslation(Vector2(0, 30), Vector2(1.5f, 1.5f)),
			Color::white(),
			Vector2(0.5, 0.5f),
			Color::black(),
			1.0f);
		renderSystem.initInstancedRender(ecs.instancedRenders.get(exitText));*/

		createInputEntity(1);
		createInputEntity(2);
	} break;
	}
}




}
