#define GL3W_IMPLEMENTATION
#include <gl3w.h>
#include <GLFW/glfw3.h>


// stdlib
#include <chrono>
#include <iostream>
#include <thread>
#include "common.hpp"

// sdl
//#include <SDL.h>
//#include <SDL_mixer.h>



// systems
import WorldSystem;
import RenderSystem;
import MovementSystem;
import InputSystem;
import CombatSystem;
import DeathSystem;
import ECS;
import Entity;
import AiSystem;
import SceneSystem;
import ParticleSystem;
import Singletons;
import TutorialSystem;
import AudioSystem;
import Factories;
import CharacterSelectSystem;
import ArenaSelectSystem;



// clock
using Clock = std::chrono::high_resolution_clock;


// Entry point
int main() {
	WorldSystem worldSystem = WorldSystem();
	RenderSystem renderSystem = RenderSystem();
	MovementSystem movementSystem = MovementSystem();
	CombatSystem combatSystem = CombatSystem();
	DeathSystem deathSystem = DeathSystem();
	AISystem aiSystem = AISystem();
	ParticleSystem particleSystem = ParticleSystem();
	TutorialSystem tutorialSystem = TutorialSystem();
	AudioSystem audioSystem = AudioSystem();
	SceneSystem sceneSystem = SceneSystem(worldSystem, renderSystem, audioSystem);
	InputSystem inputSystem = InputSystem(sceneSystem);
	CharacterSelectSystem characterSelectSystem = CharacterSelectSystem();
	ArenaSelectSystem arenaSelectSystem = ArenaSelectSystem();

	// make sure all polygons are valid
	for (size_t i = 0; i < (int)GEOMETRY_BUFFER_ID::COUNT; i++) {
		assert(POLYGONS[i].verify());
	}

	// initialize window
	GLFWwindow* window = worldSystem.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window." << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!audioSystem.init()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	renderSystem.init(window);
	inputSystem.connectWindow(window);

	sceneSystem.setScene(GAME_SCREEN::TITLE_SCREEN);

	// tutorial.start();

	audioSystem.playMusic(MUSIC_ASSET_ID::BATTLE_1);


	// variable timestep loop
	auto t = Clock::now();
	float accumulator = 0.0f;

	GAME_SCREEN current_screen = gameState.currentScreen;
	while (!worldSystem.is_over()) {

		/*Vector2 pos = ecs.transforms.get(player1Entity).getTranslation();
		std::cout << pos.x << ", " << pos.y << std::endl;*/

		// processes system messages, if this wasn't present the window would become unresponsive
		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float delta = std::chrono::duration<float>(now - t).count();
		gameTime += delta;
		t = now;
		accumulator += delta;

		if (accumulator >= FIXED_FRAME_PERIOD) {
			glfwPollEvents();
			inputSystem.tick(window);
			current_screen = gameState.currentScreen;
			
			aiSystem.update(FIXED_FRAME_PERIOD);

			characterSelectSystem.update();
			arenaSelectSystem.update();

			// prevents game from updating when in drawing state
			if (gameState.currentState == GAME_SCREEN_ID::PLAYING) {
				movementSystem.update(FIXED_FRAME_PERIOD, audioSystem, renderSystem);
				combatSystem.update(FIXED_FRAME_PERIOD, audioSystem, renderSystem);
				deathSystem.update(FIXED_FRAME_PERIOD, renderSystem);
				particleSystem.update(FIXED_FRAME_PERIOD);
				tutorialSystem.update(FIXED_FRAME_PERIOD, renderSystem);
			}
			if (gameState.currentState == GAME_SCREEN_ID::DRAWING_MAIN) {
				// emptee for nawo
			}
			if (gameState.currentState == GAME_SCREEN_ID::TITLE_SCREEN) {
				movementSystem.update(FIXED_FRAME_PERIOD, audioSystem, renderSystem);
				combatSystem.update(FIXED_FRAME_PERIOD, audioSystem, renderSystem);
			}
			
			sceneSystem.update(FIXED_FRAME_PERIOD);
			worldSystem.update_window_caption(accumulator);
			renderSystem.draw(FIXED_FRAME_PERIOD, current_screen);
			accumulator = 0;
		}
	}

	sceneSystem.killScene();
	audioSystem.destroy();

	return EXIT_SUCCESS;
}