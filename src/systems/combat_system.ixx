module;

#include <common.hpp>
#include <iostream>
#include <algorithm>

export module CombatSystem;

import ECS;
import Factories;
import Collision;
import Singletons;
import Range;
import AudioSystem;
import RenderSystem;

export class CombatSystem {

public:
	void update(float delta, AudioSystem& audioSystem, RenderSystem& renderSystem) {

		// GIVE PLAYERS INFINITE HEALTH ON TITLE SCREEN
		if (gameState.currentScreen == GAME_SCREEN::TITLE_SCREEN) {
			for (auto& entity : ecs.players.entities) {
				Health& health = ecs.healths.get(entity);
				health.health = 10;
			}
		}

		// DASH I FRAMES
		for (auto& entity : ecs.players.entities) {
			auto& player = ecs.players.get(entity);
			auto& collisionBody = ecs.collisionBodies.get(entity);
			auto& dash = ecs.dashes.get(entity);

			collisionBody.hurtMask = dash.isActive() ? 0 : (int)COLLISION_LAYER_ID::PLAYER_HURT;
		}

		// HIT FLASH TIMER
		for (auto& entity : ecs.players.entities) {
			auto& player = ecs.players.get(entity);
			player.hitFlashTime -= delta;
			player.hitFlashTime = std::max(player.hitFlashTime, 0.0f);
		}

		// IMMUNITY TIMER
		for (auto& entity : ecs.players.entities) {
			auto& player = ecs.players.get(entity);
			player.immunityTime -= delta;
			player.immunityTime = std::max(player.hitFlashTime, 0.0f);
		}
		

		// CREATE ATTACKS
		for (auto& entity : ecs.players.entities) {
			auto& player = ecs.players.get(entity);
			auto& transform = ecs.transforms.get(entity);
			PlayerInput nullInput;
			auto& playerInput = player.isDead ? nullInput : ecs.playerInputs.get(entity);
			auto& weaponEntity = player.weapon;
			Weapon& weapon = ecs.weapons.get(weaponEntity);
			
			// CHARGING
			bool startCharging = weapon.isReady() && playerInput.isJustPressed(INPUT_ID::ATTACK);

			if (startCharging || (weapon.isCharging() && playerInput.isPressed(INPUT_ID::ATTACK))) {
				weapon.attackChargeAmount += weapon.getAttackChargeRate() * delta;
				weapon.attackChargeAmount = std::min(weapon.attackChargeAmount, 1.0f);

				switch (weapon.id) {
				case WEAPON_ID::BOW: {
					Vector2 aim = playerInput.getAimVector().normalizeSafe();

					if (startCharging) {
						if (aim.length() == 0) aim = player.isFacingRight ? Vector2::right() : Vector2::left();
						weapon.rotation = aim.angle();
					} else if (aim.length() != 0) {
						weapon.rotation = aim.angle();
					}

					if (startCharging) {
						audioSystem.playSfx(SFX_ASSET_ID::BOW_CHARGE);
					}
				} break;

				case WEAPON_ID::STAFF: {
					Vector2 aim = playerInput.getAimVector().normalizeSafe();

					if (startCharging) {
						if (aim.length() == 0) aim = player.isFacingRight ? Vector2::right() : Vector2::left();
						weapon.rotation = aim.angle();
						weapon.chargeSoundChannel = audioSystem.playSfx(SFX_ASSET_ID::STAFF_CHARGE, true);
					} else if (aim.length() != 0) {
						weapon.rotation = aim.angle();
					}

					audioSystem.setSfxVolume(weapon.chargeSoundChannel, weapon.attackChargeAmount * 0.5f);
				} break;
				}
			} else {
				audioSystem.haltSfx(weapon.chargeSoundChannel);
				weapon.chargeSoundChannel = -1;
			}

			// ATTACKING
			bool startAttack = weapon.isCharging() && playerInput.isJustReleased(INPUT_ID::ATTACK);
			if (startAttack) {
				weapon.attackStrength = weapon.attackChargeAmount;
				weapon.attackChargeAmount = 0.0f;
				weapon.attackTime = 0.0f;
			}

			if (startAttack || weapon.isAttacking()) {
				bool isFinishedAttacking = false;
				float oldAttackTimer = weapon.attackTime;
				weapon.attackTime += delta;

				switch (weapon.id) {
				case WEAPON_ID::SWORD: {
					if (startAttack) {
						if (weapon.attackStrength >= 1.0f)
							audioSystem.playSfx(SFX_ASSET_ID::SWORD_SWIPE_CRITICAL);
						else
							audioSystem.playSfxRandomRange(SFX_ASSET_ID::SWORD_SWIPE_1, SFX_ASSET_ID::SWORD_SWIPE_3);
					}
					if (oldAttackTimer <= 0.0f && weapon.attackTime > 0.0f) {
						Vector2 aim = playerInput.getAimVector().normalizeSafe();
						Attack& attack = ecs.attacks.get(weaponEntity);
						attack.damage = weapon.attackStrength >= 1.0f ? ATK_DAMAGE_CHARGED : ATK_DAMAGE_BASE;
						attack.knockback = Vector2::right() * std::lerp(SWORD_KNOCK_DIST_MIN, SWORD_KNOCK_DIST_MAX, powf(weapon.attackStrength, 2));
						attack.knockbackNoRotate = Vector2::up() * std::abs(aim.dot(Vector2::right())) * std::lerp(100, 300, powf(weapon.attackStrength, 2));
						attack.knockbackTime = std::lerp(SWORD_KNOCK_TIME_MIN, SWORD_KNOCK_TIME_MAX, weapon.attackStrength);
						attack.rotation = playerInput.getAimVector().angle();
						attack.flags = 0;
						weapon.rotation = attack.rotation;

						auto& ignoredCollisionEntities = ecs.ignoredCollisionEntities.get(weaponEntity);
						ignoredCollisionEntities.clear();
						ignoredCollisionEntities.insert(entity);
					}

					isFinishedAttacking = weapon.attackTime >= weapon.getAttackLength();
				} break;

				case WEAPON_ID::BOW: {

					if (startAttack) {
						audioSystem.playSfx(SFX_ASSET_ID::BOW_SHOOT);
					}

					isFinishedAttacking = true;
					Vector2 aim = Vector2::right().rotated(weapon.rotation);
					float additionalUpWeight = abs(aim.dot(Vector2::right()));
					aim += Vector2::up() * additionalUpWeight * 0.05;
					aim = aim.normalizeSafe();
					float speed = std::lerp(ARROW_SPEED_MIN, ARROW_SPEED_MAX, weapon.attackStrength);
					Entity arrowEntity = createArrow(transform.getTranslation() + aim * 15.0f, aim * speed, player.id, weapon.attackStrength >= 1.0f ? ATK_DAMAGE_CHARGED : ATK_DAMAGE_BASE);

					Arrow& arrow = ecs.arrows.get(arrowEntity);
					ecs.ignoredCollisionEntities.get(arrow.hitbox).insert(entity);
				} break;
			
				case WEAPON_ID::STAFF: {

					if (startAttack) {
						
						audioSystem.playSfx(SFX_ASSET_ID::STAFF_SHOOT);
						Vector2 aim = Vector2::right().rotated(weapon.rotation);
						float speed = std::lerp(FIREBALL_SPEED_MIN, FIREBALL_SPEED_MAX, weapon.attackStrength);
						float lifetime = std::lerp(FIREBALL_TIME_MIN, FIREBALL_TIME_MAX, weapon.attackStrength);
						std::cout << "LIFE: " << lifetime << " STRENGTH: " << weapon.attackStrength;
						Entity fireballEntity = createFireball(transform.getTranslation() + aim * 10.0f, aim * speed, player.id, weapon.attackStrength >= 1.0f ? ATK_DAMAGE_CHARGED : ATK_DAMAGE_BASE, lifetime, renderSystem);
					
						Fireball& fireball = ecs.fireballs.get(fireballEntity);
						ecs.ignoredCollisionEntities.get(fireball.hitbox).insert(entity);
					}

					isFinishedAttacking = weapon.attackTime >= weapon.getAttackLength();

				} break;
			}

				if (isFinishedAttacking) {
					weapon.cooldownTime = weapon.getCooldownTimeMax();
					weapon.attackTime = 0.0f;
				}
			}

			// COOLDOWN
			if (weapon.cooldownTime > 0.0f) {
				weapon.cooldownTime -= delta;
				weapon.cooldownTime = std::max(weapon.cooldownTime, 0.0f);
			}

			// Tutorial stuff
			bool isNonAIPlayer = ecs.players.has(entity) && !ecs.aiAgents.has(entity);
			if (isNonAIPlayer && startAttack) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::ATTACKED);
			if (isNonAIPlayer && weapon.attackStrength > 0.9f && startAttack) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::FULLY_CHARGED_ATTACK);
			if (isNonAIPlayer && startAttack && playerInput.getAimVector().y < 0.0f) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::ATTACKED_UPWARD);
			if (isNonAIPlayer && startAttack && playerInput.getAimVector().y > 0.0f) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::ATTACKED_DOWNWARD);
		}


		// UPDATE ATTACK HITBOX
		for (auto& entity : ecs.players.entities) {
			auto& player = ecs.players.get(entity);
			if (!ecs.collisionBodies.has(player.weapon)) continue;
			auto& weapon = ecs.weapons.get(player.weapon);
			auto& weaponTransform = ecs.transforms.get(player.weapon);
			auto& weaponCollisionBody = ecs.collisionBodies.get(player.weapon);
			auto& dash = ecs.dashes.get(entity);
			auto& attack = ecs.attacks.get(player.weapon);
			weaponCollisionBody.hitMask = 0;

			if (dash.isActive() && player.isSpecialActive && player.class_id == CLASS_ID::KNIGHT) {
				weaponCollisionBody.hitMask |= (int) COLLISION_LAYER_ID::PLAYER_HURT;
				attack.rotation = weaponTransform.getRotation();
			}
			if (weapon.isAttacking()) {
				weaponCollisionBody.hitMask |= (int)COLLISION_LAYER_ID::PLAYER_HURT;
				attack.rotation = weaponTransform.getRotation();
			}
		}


		// UPDATE ARROWS
		for (auto& entity : ecs.arrows.entities) {
			if (!ecs.kinematics.has(entity)) continue;

			auto& arrow = ecs.arrows.get(entity);
			auto& attack = ecs.attacks.get(arrow.hitbox);
			auto& hitboxTransformParent = ecs.transformParents.get(arrow.hitbox);
			auto& kinematic = ecs.kinematics.get(entity);
			

			float angle = kinematic.velocity.angle();
			float speed = kinematic.velocity.length();

			
			attack.knockback = Vector2::right() * ARROW_KNOCK_DIST_MULT * speed;
			attack.knockbackTime = Range(ARROW_SPEED_MIN * 1.5, ARROW_SPEED_MAX * 0.9).remapValueClamped(Range(ARROW_KNOCK_TIME_MIN, ARROW_KNOCK_TIME_MAX), speed);
			//std::cout << "attack knockback lifetime: " << attack.knockbackTime << std::endl;
			//std::cout << "speed: " << speed << std::endl;

			if (speed > 0) {
				attack.rotation = angle;
				arrow.rotation = angle;
			}

			Transform t;
			t.rotate(-angle);
			hitboxTransformParent.relativeTransform = t;
		}


		// UPDATE FIREBALLS
		for (auto& entity : ecs.fireballs.entities) {
			auto& fireball = ecs.fireballs.get(entity);
			auto& attack = ecs.attacks.get(fireball.hitbox);
			auto& hitboxTransformParent = ecs.transformParents.get(fireball.hitbox);
			auto& particleTransformParent = ecs.transformParents.get(fireball.particle);
			auto& kinematic = ecs.kinematics.get(entity);
			auto& deathTimer = ecs.deathTimers.get(entity);
			auto& collisionTracker = ecs.collisionTrackers.get(entity);

			float angle = kinematic.velocity.angle();
			float speed = kinematic.velocity.length();

			float timePercent = deathTimer.timeLeft / FIREBALL_TIME_MAX;

			fireball.power = std::lerp(0.1, 1.0, std::pow(timePercent, 0.5));

			if (collisionTracker.isOnSurface()) {
				deathTimer.timeLeft = std::min(deathTimer.timeLeft + FIREBALL_BOUNCE_EXTEND, FIREBALL_TIME_MAX);
				fireball.bouncesLeft--;
				audioSystem.playSfx(SFX_ASSET_ID::ARROW_HIT_WALL);
			}

			attack.knockback = Vector2::right() * std::lerp(FIREBALL_KNOCK_DIST_MIN, FIREBALL_KNOCK_DIST_MAX, timePercent);
			attack.knockbackTime = std::lerp(FIREBALL_KNOCK_TIME_MIN, FIREBALL_KNOCK_TIME_MAX, timePercent);
			
			if (speed > 0) {
				attack.rotation = angle;

				Transform t;
				t.rotate(angle);
				particleTransformParent.relativeTransform = t;
			}

			{
				Transform t;
				t.scale(Vector2::one() * std::lerp(0.5, 2.5, timePercent));
				hitboxTransformParent.relativeTransform = t;
			}
		}


		// PROCESS ATTACKS
		for (auto& entity : ecs.attacks.entities) {
			Attack& attack = ecs.attacks.get(entity);
			auto& ignoredCollision = ecs.ignoredCollisionEntities.get(entity);
			auto& transform = ecs.transforms.get(entity);

			std::vector<CollisionEvent> collisions;
			getPolygonCollisions(entity, collisions);

			if (collisions.size() > 0)
				attack.lastHitEntity = collisions[collisions.size() - 1].entity;

			for (const auto& collisionEvent : collisions) {

				// Don't do anything if immunity is active
				if (ecs.players.has(collisionEvent.entity)) {
					auto& player = ecs.players.get(collisionEvent.entity);
					if (player.isImmune()) continue;
				}

				// Health
				if (ecs.healths.has(collisionEvent.entity)) {
					auto& health = ecs.healths.get(collisionEvent.entity);
					health.health -= attack.damage;
					health.health = std::max(health.health, 0.0f);

					audioSystem.playSfx(attack.damage > 1.0f ? SFX_ASSET_ID::DAMAGE_CRITICAL_1 : SFX_ASSET_ID::DAMAGE_1);

					if (health.health <= 0.0f && ecs.players.has(collisionEvent.entity)) {
						audioSystem.playSfx(SFX_ASSET_ID::DEATH);
					}
				}

				// Knockback
				if (ecs.knockables.has(collisionEvent.entity)) {
					auto& otherTransform = ecs.transforms.get(collisionEvent.entity);
					auto& otherKinematic = ecs.kinematics.get(collisionEvent.entity);
					auto& otherKnockable = ecs.knockables.get(collisionEvent.entity);

					Vector2 diDirection = ecs.playerInputs.has(collisionEvent.entity) ? ecs.playerInputs.get(collisionEvent.entity).getAimVector()
						: Vector2::zero();

					Vector2 knockbackVector = attack.knockback.rotated(attack.rotation) + attack.knockbackNoRotate;
					knockbackVector += (otherTransform.getTranslation() - transform.getTranslation()).normalize() * attack.radialKnockback;
					knockbackVector = knockbackVector.lerp(diDirection * knockbackVector.length(), otherKnockable.diControlPercent);

					otherKinematic.velocity = knockbackVector / otherKnockable.weight;
					otherKnockable.knockbackTime = attack.knockbackTime;
					//std::cout << "Knockback lifetime: " << knockable.knockbackTime << std::endl;
				}

				// Cancel Attack; Hit flash; I Frames
				if (ecs.players.has(collisionEvent.entity)) {
					auto& player = ecs.players.get(collisionEvent.entity);
					auto& weapon = ecs.weapons.get(player.weapon);

					// Cancel Attack
					weapon.cancelAttack();

					// Hit Flash
					player.hitFlashTime = 0.2;

					// I Frames
					player.immunityTime = 0.2;
				}
				ignoredCollision.insert(collisionEvent.entity);
			}

			bool landedAttack = collisions.size() > 0;
			if (landedAttack && !(attack.flags & (int) ATTACK_FLAGS::IS_SPECIAL)) {
				Entity playerEntity = getPlayer(attack.owner);
				ecs.players.get(playerEntity).isSpecialCharged = true;

				bool isNonAIPlayer = ecs.players.has(playerEntity) && !ecs.aiAgents.has(playerEntity);
				if (isNonAIPlayer) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::CHARGED_SPECIAL);
			}
		}


		// DISABLE ARROW IF HIT SOMETHING
		for (auto& entity : ecs.arrows.entities) {

			Entity attachTo = Entity::null();
			bool disableArrow = false;

			auto& arrow = ecs.arrows.get(entity);
			if (ecs.deathTimers.has(entity)) continue;


			auto& transform = ecs.transforms.get(entity);
			auto& attack = ecs.attacks.get(arrow.hitbox);
			auto& collisionTracker = ecs.collisionTrackers.get(entity);

			// stick in wall
			if (collisionTracker.isOnSurface()) {
				disableArrow = true;
				audioSystem.playSfx(SFX_ASSET_ID::ARROW_HIT_WALL);
			}
			
			// stick in player
			if (ecs.healths.has(attack.lastHitEntity) && ecs.transforms.has(attack.lastHitEntity)) {
				disableArrow = true;
				attachTo = attack.lastHitEntity;
			}
			
			if (disableArrow) {
				ecs.kinematics.remove(entity);
				ecs.gravities.remove(entity);
				ecs.remove_all_components_of(arrow.hitbox);

				auto& deathTimer = ecs.deathTimers.emplace(entity);
				deathTimer.timeLeft = 1.0f;

				if (attachTo != Entity::null()) {
					TransformParent& transformParent = ecs.transformParents.emplace(entity);
					transformParent.parent = attachTo;
					transformParent.relativeTransform.matrix = transform.matrix / ecs.transforms.get(attachTo).matrix;
				}
			}
		}


		// QUEUE FIREBALL KILL
		for (auto& entity : ecs.fireballs.entities) {
			auto& fireball = ecs.fireballs.get(entity);
			auto& attack = ecs.attacks.get(fireball.hitbox);
			auto& deathTimer = ecs.deathTimers.get(entity);

			// delete upon reaching max bounces
			if (fireball.bouncesLeft <= 0) {
				deathTimer.timeLeft = 0.0f;
				continue;
			}

			// destroy when hit player
			if (ecs.healths.has(attack.lastHitEntity) && ecs.transforms.has(attack.lastHitEntity)) {
				deathTimer.timeLeft = 0.0f;
				audioSystem.playSfx(SFX_ASSET_ID::FIREBALL_LAND);
				continue;
			}
		}
	}

};