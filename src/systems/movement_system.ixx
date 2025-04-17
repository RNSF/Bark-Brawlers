module;

#include <cmath>
#include <common.hpp>
#include <iostream>
#include <optional>
#include <numbers>

export module MovementSystem;
import ECS;
import Components;
import Entity;
import Rect;
import Collision;
import Singletons;
import WeaponAnimationHelpers;
import Math;
import Range;
import Factories;
import AudioSystem;
import RenderSystem;

export class MovementSystem {
	
public:
	void update(float delta, AudioSystem& audioSystem, RenderSystem& renderSystem) {

		// Update player stats
		for (auto& entity : ecs.players.entities) {
			PlatformerController& platformerController = ecs.platformerControllers.get(entity);
			Player& player = ecs.players.get(entity);
			
			bool isChargingWeapon = false;
			bool isAttacking = false;

			if (player.weapon) {
				Weapon& weapon = ecs.weapons.get(player.weapon);
				isChargingWeapon = weapon.isCharging();
				isAttacking = weapon.isAttacking();
			}

			platformerController.acceleration = 20.0f;
			platformerController.jumpStrength = (isChargingWeapon || isAttacking) ? 350.0f * 0.5 : 350.0f;
			platformerController.maxWalkSpeed = (isChargingWeapon || isAttacking) ? 170.0 * 0.3 : 170.0;
			platformerController.maxVerticalSpeed = platformerController.jumpStrength;
		}

		// Update knockback
		for (auto& entity : ecs.knockables.entities) {
			Knockable& knockable = ecs.knockables.get(entity);
			Kinematic& kinematic = ecs.kinematics.get(entity);
			knockable.knockbackTime -= delta;
			kinematic.bounciness = knockable.isKnocked() ? 0.5f : 0.0f;
		}
		

		// Trigger dash / special
		for (auto& entity : ecs.players.entities) {
			Player& player = ecs.players.get(entity);
			if (player.isDead) continue;
			PlayerInput& input = ecs.playerInputs.get(entity);
			Dash& dash = ecs.dashes.get(entity);
			Weapon& weapon = ecs.weapons.get(player.weapon);
			auto& collisionTracker = ecs.collisionTrackers.get(entity);

			if (dash.isActive()) continue;
			
			if (collisionTracker.flags & (int)COLLISION_TRACKER_FLAG::IS_ON_GROUND) player.hasTouchedGroundSinceLastDash = true;
			
			player.isSpecialActive = false;
			player.dashCooldown -= delta;

			bool dashed = false;
			if (player.canDash() && input.isJustPressed(INPUT_ID::SPECIAL)) {

				dashed = true;
				Vector2 dashDirection = input.getAimVector().normalizeSafe();
				if (dashDirection.length() == 0) dashDirection = player.isFacingRight ? Vector2::right() : Vector2::left();

				float dashMainTime = 0.0f;
				float dashSlowingTime = 0.0f;
				float dashSpeed = 0.0;

				switch (player.class_id) {
				case CLASS_ID::KNIGHT: {
					dashDirection.y = 0;

					dashMainTime = KNIGHT_DASH_MAIN;
					dashSlowingTime = KNIGHT_DASH_SLOW;
					dashSpeed = KNIGHT_DASH_SPEED;
				} break;
				case CLASS_ID::ARCHER: {
					dashDirection.y = 0.5;
					dashDirection *= -0.85;

					dashMainTime = ARCHER_DASH_MAIN;
					dashSlowingTime = ARCHER_DASH_SLOW;
					dashSpeed = ARCHER_DASH_SPEED;
				} break;
				case CLASS_ID::MAGE: {
					if (dashDirection.y != 0) dashDirection.x = 0;
					else					  dashDirection.y = 0;

					dashMainTime = MAGE_DASH_MAIN;
					dashSlowingTime = MAGE_DASH_SLOW;
					dashSpeed = MAGE_DASH_SPEED;
				} break;
				//case CLASS_ID::THIEF: {
				//	// do nothing :)
				//} break;
				}

				dash.mainTime = dashMainTime;
				dash.slowingTime = dashSlowingTime;
				dash.velocity = dashDirection * dashSpeed;

				player.dashCooldown = 0.5f;
				player.hasTouchedGroundSinceLastDash = false;

				weapon.cancelAttack();

				audioSystem.playSfx(SFX_ASSET_ID::DASH);

				// SPECIAL DASH
				if (input.isPressed(INPUT_ID::ATTACK)) {
					player.isSpecialActive = player.isSpecialCharged;
					player.isSpecialCharged = false;

					if (player.isSpecialActive) {

						audioSystem.playSfx(SFX_ASSET_ID::USE_SPECIAL);

						if (player.class_id == CLASS_ID::KNIGHT) {
							auto& ignoredCollisionEntities = ecs.ignoredCollisionEntities.get(player.weapon);
							ignoredCollisionEntities.clear();
							ignoredCollisionEntities.insert(entity);

							Attack& attack = ecs.attacks.get(player.weapon);
							attack.damage = ATK_DAMAGE_BASE;
							attack.knockback = Vector2::zero();
							attack.knockbackNoRotate = Vector2::up() * SWORD_KNOCK_DIST_MIN + dashDirection * SWORD_KNOCK_DIST_MAX;
							attack.knockbackTime = SWORD_KNOCK_TIME_SPECIAL;
							attack.rotation = dashDirection.angle();
							attack.flags |= (int)ATTACK_FLAGS::IS_SPECIAL;

							

							weapon.rotation = attack.rotation;
						}

						if (player.class_id == CLASS_ID::ARCHER) {
							Vector2 pos = ecs.transforms.get(entity).getTranslation();
							Vector2 aims[] = {-dashDirection.rotated(0.15f), -dashDirection.rotated(-0.10f)};

							for (Vector2 aim : aims) {
								createArrow(pos + aim * 15.0f, aim * 500.0f, player.id, 1.0, (int) ATTACK_FLAGS::IS_SPECIAL);
							}
						}

						if (player.class_id == CLASS_ID::MAGE) {
							dash.mainTime = MAGE_SPECIAL_DASH_MAIN;
							dash.slowingTime = MAGE_SPECIAL_DASH_SLOW;
							dash.velocity = dashDirection * MAGE_SPECIAL_DASH_SPEED;

							Vector2 pos = ecs.transforms.get(entity).getTranslation();
							createExplosion(pos + dashDirection * EXPLOSION_OFFSET, player.id, renderSystem);
							audioSystem.playSfx(SFX_ASSET_ID::EXPLOSION);
						}

						weapon.cooldownTime = weapon.getCooldownTimeMax();
					}
				}
			
				
			}

			// Tutorial
			bool isNonAIPlayer = ecs.players.has(entity) && !ecs.aiAgents.has(entity);
			if (isNonAIPlayer && player.isSpecialActive) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::SPECIAL_DASHED);
			if (isNonAIPlayer && dashed)
				tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::DASHED);
		}


		// Update dash
		for (auto& entity : ecs.dashes.entities) {
			Dash& dash = ecs.dashes.get(entity);
			Kinematic& kinematic = ecs.kinematics.get(entity);

			if (dash.isDashing()) {
				dash.mainTime -= delta;
				kinematic.velocity = dash.velocity;
			} else if (dash.isSlowing()) {
				dash.slowingTime -= delta;
				kinematic.velocity = kinematic.velocity.lerp(Vector2::zero(), 20.0f * delta);
			}
		}

		// M1 Basic Feature: [8] Basic physics
		// Platforming
		for (auto& entity : ecs.platformerControllers.entities) {
			if (ecs.knockables.has(entity) && ecs.knockables.get(entity).isKnocked()) continue;
			if (ecs.dashes.has(entity) && ecs.dashes.get(entity).isActive()) continue;

			PlatformerController& platformerController = ecs.platformerControllers.get(entity);
			Kinematic& kinematic = ecs.kinematics.get(entity);
			Gravity& gravity = ecs.gravities.get(entity);
			
			float walkDirection = 0.0f;
			
			if (ecs.playerInputs.has(entity)) {
				Player& player = ecs.players.get(entity);
				if (!player.isDead) {
					PlayerInput& input = ecs.playerInputs.get(entity);
					walkDirection = input.getAimVector().x;

					if (input.isJustPressed(INPUT_ID::JUMP) && !(ecs.aiAgents.has(entity)))
						platformerController.jumpInputTime = platformerController.jumpInputTimeMax;

					if (input.isPressed(INPUT_ID::JUMP) && ecs.aiAgents.has(entity))
						platformerController.jumpInputTime = 0.02f;

					if (input.isJustReleased(INPUT_ID::JUMP))
						platformerController.isJumpCancelPending = true;
				}
			}

			platformerController.coyoteTime -= delta;
			platformerController.jumpInputTime -= delta;

			if (ecs.collisionTrackers.has(entity)) {
				CollisionTracker& collisionTracker = ecs.collisionTrackers.get(entity);
				if (collisionTracker.flags & (int) COLLISION_TRACKER_FLAG::IS_ON_GROUND) {
					platformerController.coyoteTime = platformerController.coyoteTimeMax;
				}
				//std::cout << collisionTracker.flags << std::endl;
			}

			// Jumping
			bool canJump = platformerController.coyoteTime > 0.0f;
			if (canJump) {

				if (platformerController.jumpInputTime > 0.0f) {
					kinematic.velocity.y = -platformerController.jumpStrength;
					platformerController.coyoteTime = 0.0f;
					platformerController.jumpInputTime = 0.0f;
					platformerController.isJumping = true;
					platformerController.jumpTime = 0.0f;
					audioSystem.playSfxRandomRange(SFX_ASSET_ID::FOOTSTEP_1, SFX_ASSET_ID::FOOTSTEP_5);
				}
				else {
					platformerController.isJumpCancelPending = false;
				}
			}

			if (platformerController.isJumping) {
				platformerController.jumpTime += delta;
			}
			
			bool canCancelJump = platformerController.jumpTime > platformerController.minJumpTime && platformerController.isJumping;
			if (canCancelJump && (platformerController.isJumpCancelPending || kinematic.velocity.y > 0)) {
				platformerController.isJumping = false;
				kinematic.velocity.y = std::max(kinematic.velocity.y * 0.3f, kinematic.velocity.y);
				platformerController.isJumpCancelPending = false;
			}

			gravity.strength = DEFAULT_GRAVITY * (platformerController.isJumping ? 1.0f : 1.3f);
			
			kinematic.velocity.x = std::lerp(kinematic.velocity.x, walkDirection * platformerController.maxWalkSpeed, platformerController.acceleration * delta);
			kinematic.velocity.y = std::lerp(kinematic.velocity.y, std::min(platformerController.maxVerticalSpeed, abs(kinematic.velocity.y)) * signf(kinematic.velocity.y), platformerController.acceleration * delta);

			// Tutorial stuff
			bool isNonAIPlayer = ecs.players.has(entity) && !ecs.aiAgents.has(entity);
			if (isNonAIPlayer && walkDirection != 0) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::WALKED);
			if (isNonAIPlayer && platformerController.isJumping) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::JUMPED);
			if (isNonAIPlayer && platformerController.jumpTime > 0.4f) tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::LONG_JUMPED);
		}

		// Gravity
		for (auto& entity : ecs.gravities.entities) {
			if (ecs.dashes.has(entity) && ecs.dashes.get(entity).isActive()) continue;

			Gravity& gravity = ecs.gravities.get(entity);
			Kinematic& kinematic = ecs.kinematics.get(entity);
			kinematic.velocity.y += gravity.strength * delta;
		}

		// M1 [7] Simple collision detection & resolution (e.g. between square sprites)
		// Kinematics
		for (auto& entity : ecs.kinematics.entities) {
			Transform& transform = ecs.transforms.get(entity);
			Kinematic& kinematic = ecs.kinematics.get(entity);

			Vector2 dx = kinematic.velocity * delta;
			
			CollisionTracker nullTracker;
			CollisionTracker& collisionTracker = ecs.collisionTrackers.has(entity) ? ecs.collisionTrackers.get(entity) : nullTracker;
			collisionTracker.flags = 0;
			
			while (dx.length() > COLLISION_EPSILON) {

				transform.translate(dx);

				CollisionBox& collisionBox = ecs.collisionBoxes.get(entity);
				std::optional<CollisionEvent> collision = getFirstBoxCollision(entity);

				if (ecs.collisionBoxes.has(entity) && collision) {
					
					Rect rect = collision.value().rect;
					// cancel velocity in dx direction
				

					// move back
					transform.translate(-dx);

					// binary search for closest position
					Vector2 subDx = dx;
					Vector2 appliedDx = Vector2::zero();

					while (subDx.length() > COLLISION_EPSILON) {
						subDx /= 2;
						std::optional<CollisionEvent> newCollision = getFirstBoxCollision(entity);
						Vector2 signedSubDx = subDx * (newCollision ? -1 : 1);
						transform.translate(signedSubDx);
						appliedDx += signedSubDx;
						rect = newCollision ? newCollision.value().rect : rect;
					}

					transform.translate(dx.normalize() * -COLLISION_EPSILON);

					Vector2 dif = getAABB(entity).overlappingRect(rect).size();
					float difDif = dif.y - dif.x;

					//std::cout << abs(displacement.x) << ", " << abs(displacement.y) << std::endl;

					float remainingDelta = (1.0f - appliedDx.length() / dx.length()) * delta;

					if (difDif <= 0.0) {
						if (kinematic.velocity.y > 0) collisionTracker.flags |= (int)COLLISION_TRACKER_FLAG::IS_ON_GROUND;
						else						  collisionTracker.flags |= (int)COLLISION_TRACKER_FLAG::IS_ON_CEILING;
						kinematic.velocity.y *= -kinematic.bounciness;
						kinematic.velocity.x *= kinematic.slipperiness;
						
					}
					
					if (difDif >= 0.0) {
						collisionTracker.flags |= (int)COLLISION_TRACKER_FLAG::IS_ON_WALL;
						kinematic.velocity.x *= -kinematic.bounciness;
						kinematic.velocity.y *= kinematic.slipperiness;
					}

					dx = kinematic.velocity * remainingDelta;
					//std::cout << dx.length() << std::endl;
				
				} else {
					dx = Vector2::zero();
				}
			}
		}


		// Update player facing direction
		for (auto& entity : ecs.players.entities) {
			Player& player = ecs.players.get(entity);
			Weapon& weapon = ecs.weapons.get(player.weapon);
			if (weapon.isAttacking()) continue; // don't turn around if attack
			
			PlayerInput& input = ecs.playerInputs.get(entity);
			float walkDirection = input.getAimVector().x;
			if (walkDirection != 0)
				player.isFacingRight = walkDirection > 0;
		}


		// Weapon animation
		for (auto& playerEntity : ecs.players.entities) {
			Player& player = ecs.players.get(playerEntity);
			Entity& weaponEntity = player.weapon;
			
			Weapon& weapon = ecs.weapons.get(weaponEntity);
			Dash& dash = ecs.dashes.get(playerEntity);
			TransformParent& weaponParentTransform = ecs.transformParents.get(weaponEntity);

			// Update animation id
			WEAPON_ANIMATION_ID newId = weapon.animationId;
			if (weapon.isAttacking())		newId = WEAPON_ANIMATION_ID::ATTACKING;
			else if (weapon.isCharging())	newId = WEAPON_ANIMATION_ID::CHARGING;
			else if (dash.isActive())		newId = player.isSpecialActive ? WEAPON_ANIMATION_ID::SPECIAL : WEAPON_ANIMATION_ID::DASH;
			else							newId = WEAPON_ANIMATION_ID::RESTING;

			if (newId != weapon.animationId) {
				weapon.transitionToAnimation(newId);
			}

			// Get animation state
			weapon.animationState = getWeaponAnimationState(weapon, weapon.animationId);
			weapon.animationTransition = std::lerp(weapon.animationTransition, 1.0f, weapon.animationState.transitionSpeed * delta);
			WeaponAnimationState state = weapon.previousAnimationState.lerp(weapon.animationState, weapon.animationTransition);

			// Apply new relative transform
			Transform relativeTransform;
			relativeTransform.translate((player.isFacingRight ? Vector2::one() : Vector2::flippedX()) * state.position);
			relativeTransform.rotate((player.isFacingRight) ? (state.rotation) : (std::numbers::pi - state.rotation));
			weaponParentTransform.relativeTransform = relativeTransform;
		}
	

		// Transform parents (this should go last)
		for (auto& entity : ecs.transformParents.entities) {
			auto& transformParent = ecs.transformParents.get(entity);
			auto& transform = ecs.transforms.get(entity);
			auto identityTransform = Transform();
			auto& parentTransform = ecs.transforms.has(transformParent.parent) ? ecs.transforms.get(transformParent.parent) : identityTransform;
			transform = parentTransform * transformParent.relativeTransform;
		}
	}

	

};