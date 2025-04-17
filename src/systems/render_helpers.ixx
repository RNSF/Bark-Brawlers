module;

#include <common.hpp>

export module RenderHelpers;
import Rect;
import Components;

export {
	Rect getSubSpriteRect(glm::ivec2 spriteCount, unsigned int spriteIndex) {
		Vector2 subSpriteSize = Vector2::one() / Vector2(spriteCount);
		Vector2 subSpritePosition = { (float)(spriteIndex % spriteCount.x), (float)(spriteIndex / spriteCount.x) };
		subSpritePosition /= spriteCount;
		return Rect::fromTopLeftSize(subSpritePosition, subSpriteSize);
	};

	unsigned int getInputSpriteIndex(unsigned int playerId, INPUT_ID inputId) {
		switch (playerId) {
		case 1: {
			switch (inputId) {
			case INPUT_ID::LEFT: return 120;
			case INPUT_ID::UP: return 86;
			case INPUT_ID::RIGHT: return 122;
			case INPUT_ID::DOWN: return 121;
			case INPUT_ID::ATTACK: return 124;
			case INPUT_ID::JUMP: return 123;
			case INPUT_ID::SPECIAL: return 125;
			}
		} break;
		case 2: {
			switch (inputId) {
			case INPUT_ID::LEFT: return 169;
			case INPUT_ID::UP: return 166;
			case INPUT_ID::RIGHT: return 167;
			case INPUT_ID::DOWN: return 168;
			case INPUT_ID::ATTACK: return 197;
			case INPUT_ID::JUMP: return 165;
			case INPUT_ID::SPECIAL: return 231;
			}
		} break;
		}

		return 0;
	}

	TEXTURE_ASSET_ID getArenaBackgroundFromTheme(ARENA_THEME_ID themeId) {
		switch (themeId) {
		default:
		case ARENA_THEME_ID::FOREST: return TEXTURE_ASSET_ID::BACKGROUND_FOREST;
		case ARENA_THEME_ID::TOWN: return TEXTURE_ASSET_ID::BACKGROUND_TOWN;
		case ARENA_THEME_ID::CAVE: return TEXTURE_ASSET_ID::BACKGROUND_CAVE;
		}
	}
}