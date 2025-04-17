
module;

#include <string>

export module ArenaSelectSystem;

import ECS;
import Components;
import Math;
import Singletons;

export class ArenaSelectSystem {

public:
	void update() {
		if (gameState.currentScreen != GAME_SCREEN::ARENA_SELECT_SCREEN) return;

		for (auto& playerInput : ecs.playerInputs.components) {
			int dir = (int)playerInput.isJustPressed(INPUT_ID::RIGHT) - (int)playerInput.isJustPressed(INPUT_ID::LEFT);
			gameState.arenaTheme = (ARENA_THEME_ID)wrapMod(((int)gameState.arenaTheme + dir), 0, (int)ARENA_THEME_ID::COUNT);
		}

	}

};