
module;

#include <string>

export module CharacterSelectSystem;

import ECS;
import Components;
import Math;

export class CharacterSelectSystem {

public:
	void update() {

		for (auto& entity : ecs.characterSelects.entities) {
			auto& characterSelect = ecs.characterSelects.get(entity);
			auto& playerInput = ecs.playerInputs.get(entity);
			auto& playerOptions = ecs.playerOptions.get(entity);

			characterSelect.idleAnimation.update(1.0f / 30.0f);

			if (!characterSelect.hasConfirmed) {
				
				// CHANGE CLASS
				int classDir = (int)playerInput.isJustPressed(INPUT_ID::RIGHT) - (int)playerInput.isJustPressed(INPUT_ID::LEFT);
				playerOptions.selectedClass = (CLASS_ID) wrapMod(((int)playerOptions.selectedClass + classDir), 0, (int)CLASS_ID::COUNT);

				// CHANGE HUMAN vs AI vs TUTORIAL
				if (characterSelect.playerId != 1) {
					auto oldPlayerType = playerOptions.playerType;
					int typeDir = (int)playerInput.isJustPressed(INPUT_ID::DOWN) - (int)playerInput.isJustPressed(INPUT_ID::UP);
					playerOptions.playerType = (PLAYER_TYPE_ID)wrapMod(((int)playerOptions.playerType + typeDir), 0, (int)PLAYER_TYPE_ID::COUNT);
					if (oldPlayerType != playerOptions.playerType) {
						ecs.texts.get(characterSelect.textEntity).updateStr(playerOptions.getPlayerTitle(characterSelect.playerId));

						for (auto& characterSelect2 : ecs.characterSelects.components) {
							characterSelect2.hasConfirmed = false;
						}
					}
				}

				// PLAYER MUST BE KNIGHT IF THEY ARE NOT HUMAN
				if (playerOptions.playerType != PLAYER_TYPE_ID::HUMAN)
					playerOptions.selectedClass = CLASS_ID::KNIGHT;
			}

			if (playerInput.isJustPressed(INPUT_ID::JUMP)) {
				characterSelect.hasConfirmed = !characterSelect.hasConfirmed;
			}
				

		}

	}

};