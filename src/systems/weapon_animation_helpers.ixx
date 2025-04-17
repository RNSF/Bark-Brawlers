
module;

#include <corecrt_math.h>
#include <cmath>
#include <numbers>
#include <iostream>

export module WeaponAnimationHelpers;
import Transform;
import Components;
import Vector2;
import Singletons;
import Curve;
import Range;

export {

	

	WeaponAnimationState getWeaponAnimationState(Weapon& weapon, WEAPON_ANIMATION_ID animationId) {

		WeaponAnimationState animState;
		animState.transitionSpeed = 12.0f;

		switch (weapon.id) {
		case WEAPON_ID::SWORD: {
			
			switch (animationId) {
			case WEAPON_ANIMATION_ID::ATTACKING: {
				// https://www.desmos.com/calculator/yc4odffyva

				
				float range = std::lerp(0.8f, 1.7f, weapon.attackStrength);
				Vector2 scale = Vector2(
					std::lerp(0.2f, 2.0f, weapon.attackStrength) * 20.0f,
					std::lerp(0.8f, 2.4f, weapon.attackStrength) * 10.0f
				);
				float reach = std::lerp(5.0f, 20.0f, weapon.attackStrength);
				float t = Curve::powerIn<0.7f>(weapon.getAttackPercentage());

				animState.position = Vector2(cosf(range * (1 - 2*t)), - sinf(range * (1 - 2 * t))) * scale;
				animState.position += Vector2::right() * reach;
				animState.rotation = Vector2(-sinf(range * (1 - 2 * t)), - cosf(range * (1 - 2 * t))).angle();

				Vector2 rotVec = Vector2::right().rotated(weapon.rotation);
				rotVec.x = abs(rotVec.x);
				float rotation = rotVec.angle();
				//rotation = - std::numbers::pi / 2;
				animState.position = animState.position.rotated(rotation);
				animState.rotation -= rotation;

			} break;
			case WEAPON_ANIMATION_ID::CHARGING: {
				// https://www.desmos.com/calculator/vgmkys1uls

				const float WIDTH = 7.0f;
				const float HEIGHT = 11.0f;

				float t = Curve::powerIn<0.5f>(weapon.attackChargeAmount);
				animState.position = Vector2(-WIDTH * t, -HEIGHT * powf(t, 0.2f));
				animState.position += Vector2::right() * 5.f;
				animState.rotation = Vector2(-WIDTH, -0.2f * powf(t, -0.8f)).angle() + 3 * std::numbers::pi / 2;

			} break;
			case WEAPON_ANIMATION_ID::RESTING: {
				animState.position = Vector2::up() * sinf(gameTime * 45.0f) * 1;
				animState.position += Vector2(5, 5);
				animState.rotation = std::numbers::pi / 4;
			} break;

			case WEAPON_ANIMATION_ID::DASH: {
				animState.position += Vector2(-10, 5);
				animState.transitionSpeed = 24.0f;
				animState.rotation = 0;
			} break;

			case WEAPON_ANIMATION_ID::SPECIAL: {
				animState.position += Vector2(20, 5);
				animState.transitionSpeed = 24.0f;
				animState.rotation = std::numbers::pi / 2 + 0.5f;
			}

			}
			
		} break;
		case WEAPON_ID::BOW: {

			switch (animationId) {
			case WEAPON_ANIMATION_ID::ATTACKING: {

			} break;
			case WEAPON_ANIMATION_ID::CHARGING: {
				
				Vector2 rotVec = Vector2::right().rotated(weapon.rotation);
				rotVec.x = abs(rotVec.x);
				animState.rotation = -rotVec.angle();
				animState.position = rotVec * 20;

			} break;
			case WEAPON_ANIMATION_ID::RESTING: {
				animState.position = Vector2::up() * sinf(gameTime * 45.0f) * 1;
				animState.position += Vector2(8, 8);
				animState.rotation = - std::numbers::pi / 4;
			} break;

			case WEAPON_ANIMATION_ID::DASH: {
				animState.position += Vector2(10, 5);
				animState.transitionSpeed = 24.0f;
				animState.rotation = 0;
			} break;

			case WEAPON_ANIMATION_ID::SPECIAL: {
				Vector2 rotVec = Vector2::right().rotated(weapon.rotation);
				animState.transitionSpeed = 24.0f;
				rotVec.x = abs(rotVec.x);
				animState.rotation = -rotVec.angle();
				animState.position = rotVec * 20;
			}

			}

		} break;
		

		case WEAPON_ID::STAFF: {

			switch (animationId) {
			case WEAPON_ANIMATION_ID::ATTACKING: {
				animState.transitionSpeed = 24.0f;
				Vector2 rotVec = Vector2::right().rotated(weapon.rotation);
				rotVec.x = abs(rotVec.x);
				rotVec = (rotVec + (Vector2::up() + Vector2::right()) * 0.2f).normalize();
				animState.rotation = -rotVec.angle();
				animState.position = -rotVec * std::lerp(0, 5, weapon.attackStrength) + Vector2::down() * 5;
			} break;
			case WEAPON_ANIMATION_ID::CHARGING: {

				Vector2 rotVec = Vector2::right().rotated(weapon.rotation);
				rotVec.x = abs(rotVec.x);
				rotVec = (rotVec + (Vector2::up() + Vector2::right()) * 0.2f).normalize();
				animState.rotation = -rotVec.angle();
				animState.position = -rotVec * std::lerp(-15, -5, weapon.attackChargeAmount) + Vector2::down() * 5;

				float shakeAmount = std::lerp(0, 1, weapon.attackChargeAmount);
				animState.position += Vector2(Range(-shakeAmount, shakeAmount).randomPoint(), Range(-shakeAmount, shakeAmount).randomPoint());

			} break;
			case WEAPON_ANIMATION_ID::RESTING: {
				animState.position = Vector2::up() * sinf(gameTime * 45.0f) * 1;
				animState.position += Vector2(5, 5);
				animState.rotation = std::numbers::pi / 4;
			} break;

			case WEAPON_ANIMATION_ID::DASH: {
				animState.position += Vector2(-10, 5);
				animState.transitionSpeed = 24.0f;
				animState.rotation = 0;
			} break;

			case WEAPON_ANIMATION_ID::SPECIAL: {
				animState.position += Vector2(20, 5);
				animState.transitionSpeed = 24.0f;
				animState.rotation = std::numbers::pi / 2 + 0.5f;
			}

			}

		} break;
		}

		return animState;
	}

}