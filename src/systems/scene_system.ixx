module;

#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <random>
#include "common.hpp"

export module SceneSystem;

import ECS;
import WorldSystem;
import Components;
import Singletons;
import RenderSystem;
import Factories;
import Entity;
import AudioSystem;

export class SceneSystem {
	WorldSystem& world;
	RenderSystem& rs;
	AudioSystem& as;
public:
    SceneSystem(WorldSystem& ws, RenderSystem& renderSys, AudioSystem& audioSys) : world(ws), rs(renderSys), as(audioSys) {}



	void setScene(GAME_SCREEN newScene) {
		killScene();
		gameState.currentScreen = newScene;
		gameState.currentState = GAME_SCREEN_ID::PLAYING;
		sceneTime = 0.0f;
		tutorial = Tutorial();
		createScene(newScene, rs);
	}

	void nextScene() {
		setScene(static_cast<GAME_SCREEN> (((int)gameState.currentScreen + 1) % (int) GAME_SCREEN::COUNT));
	}
	

	void killScene() {
		for (auto& instancedRender : ecs.instancedRenders.components) {
			rs.destroyInstancedRender(instancedRender);
			std::cout << "Destroyed Instanced Renderer" << std::endl;
		}

		as.haltAllSfx();

		ecs.clear_all_components();
	}

	void resetGame() {
		gameState = GameState();
		setScene(GAME_SCREEN::TITLE_SCREEN);
	}

	void nextArena() {
		gameState.currentState = GAME_SCREEN_ID::PLAYING;
		if (gameState.queuedArenas.size() > 0) {
			setScene(gameState.queuedArenas.back());
			gameState.queuedArenas.pop_back();
		} else {
			std::cout << "Ran out of arenas!" << std::endl;
			setScene(GAME_SCREEN::GAME_END_SCREEN);
		}
	}

	void startFight(bool isTutorial = false) {
		// Randomly queue arenas based on theme
		gameState.queuedArenas.clear();

		if (isTutorial) {
			gameState.queuedArenas.push_back(GAME_SCREEN::ARENA_1); // can swap out for tutorial arena if you want
		} else {
			for (unsigned int i = 0; i < 5; i++) {
				GAME_SCREEN screen = arenaSets[(int)gameState.arenaTheme][i];
				gameState.queuedArenas.push_back(screen);
			}

			static auto rng = std::default_random_engine{};
			std::ranges::shuffle(gameState.queuedArenas, rng);
		}

		

		// Play
		gameState.gameScore = GameScore();
		tutorial = Tutorial();
		nextArena();
	}


    void update(float delta) {
		sceneTime += delta;


		if (gameState.currentScreen == GAME_SCREEN::CHARACTER_SELECT_SCREEN) {
			bool allConfirmed = true;
			std::vector<std::pair<unsigned int, CLASS_ID>> selectedClasses;




			for (auto& entity : ecs.characterSelects.entities) {
				auto& characterSelect = ecs.characterSelects.get(entity);
				auto& playerOptions = ecs.playerOptions.get(entity);

				if (playerOptions.playerType == PLAYER_TYPE_ID::HUMAN && !characterSelect.hasConfirmed) {
					allConfirmed = false;
					break;
				}
				
			}

			if (allConfirmed) {
				// Set player classes and start the game

				for (auto& entity : ecs.characterSelects.entities) {
					CharacterSelect& characterSelect = ecs.characterSelects.get(entity);
					auto& playerOptions = ecs.playerOptions.get(entity);
					
					if (characterSelect.playerId == 1) gameState.player1Options = playerOptions;
					if (characterSelect.playerId == 2) gameState.player2Options = playerOptions;
				}

				bool isTutorial = gameState.player2Options.playerType == PLAYER_TYPE_ID::TUTORIAL;
				

				if (isTutorial) {
					startFight(true);
				} else {
					setScene(GAME_SCREEN::ARENA_SELECT_SCREEN);
				}
				

				
				
			}
		}



		else if (gameState.currentScreen == GAME_SCREEN::ARENA_SELECT_SCREEN) {
			for (auto& playerInput : ecs.playerInputs.components) {
				if (playerInput.isJustPressed(INPUT_ID::JUMP)) {
					startFight();
					break;
				}
			}
		}

		if ((int) gameState.currentScreen >= (int)GAME_SCREEN::ARENA_1 && (int) gameState.currentScreen <= (int)GAME_SCREEN::ARENA_15) {
			for (auto& playerInput : ecs.playerInputs.components) {
				if (playerInput.isJustPressed(INPUT_ID::PAUSE)) {
					setScene(GAME_SCREEN::CHARACTER_SELECT_SCREEN);
					break;
				}
			}
			
		}


        if (!ecs.dead.entities.empty()) {
            // switch to the game end screen if one of the players has won 5 times
			if (gameState.gameScore.player1Wins >= MAX_ROUNDS || gameState.gameScore.player2Wins >= MAX_ROUNDS) {
				setScene(GAME_SCREEN::GAME_END_SCREEN);
			}

			// if there's at least one dead component, switch to next arena, if we are on the last arena, switch back to first arena
			if (ecs.dead.entities.size() > 0) {
				tutorial.unlock((int)TUTORIAL_ACHIEVEMENT_ID::KILLED_AI);
				if (tutorial.hasStarted()) {
					setScene(GAME_SCREEN::CHARACTER_SELECT_SCREEN);
				} else {
					nextArena();
				}
			}

        }
    }
};
