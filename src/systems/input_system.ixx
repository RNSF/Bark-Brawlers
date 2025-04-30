
module;

#include <common.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <numbers>

export module InputSystem;
import ECS;
import Components;
import Vector2;
import Factories;
import Singletons;
import EditorHelpers;
import SceneSystem;
import Singletons;
import WindowHelpers;
import Math;

using std::cout;
using std::endl;

export class InputSystem {

private:
	SceneSystem& sceneSystem;
	

public:
	InputSystem(SceneSystem& scene) : sceneSystem(scene) {}
	void connectWindow(GLFWwindow* window) {

		glfwSetWindowUserPointer(window, this);
		auto keyRedirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((InputSystem*)glfwGetWindowUserPointer(wnd))->onKey(_0, _1, _2, _3); };
		auto cursorPosRedirect = [](GLFWwindow* wnd, double _0, double _1) { ((InputSystem*)glfwGetWindowUserPointer(wnd))->onMouseMotion({ _0, _1 }); };
		auto mouseButtonPressedRedirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((InputSystem*)glfwGetWindowUserPointer(wnd))->onMouseButtonPressed(_button, _action, _mods); };
		auto joystickRedirect = [](int jid, int event) {};


		glfwSetKeyCallback(window, keyRedirect);
		glfwSetCursorPosCallback(window, cursorPosRedirect);
		glfwSetMouseButtonCallback(window, mouseButtonPressedRedirect);
		glfwSetJoystickCallback(joystickRedirect);
	}

	void tick(GLFWwindow* window) {
		isAnyInputJustPressed = false;
		glfwPollEvents();

		for (unsigned int i = 0; i < 2; i++) {
			int buttonCount;
			const unsigned char* buttons = glfwGetJoystickButtons(i, &buttonCount);
			for (int i = 0; i < buttonCount; ++i) {
				if (buttons[i] == GLFW_PRESS) {
					isAnyInputJustPressed |= true;
					break;
				}
			}
		}

		// Store previous inputs
		for (auto& entity : ecs.playerInputs.entities) {
			PlayerInput& playerInput = ecs.playerInputs.get(entity);
			playerInput.tick();
		}

		// Get new inputs
		for (auto& entity : ecs.playerInputs.entities) {
			if (ecs.aiAgents.has(entity)) continue;
			PlayerInput& playerInput = ecs.playerInputs.get(entity);
			playerInput.clear();
			playerInput.inputs |= getGamepadInput(playerInput.controllerId, playerInput.controllerId == 1 ? GLFW_JOYSTICK_1 : GLFW_JOYSTICK_2).inputs;
			playerInput.inputs |= getKeyboardInput(playerInput.controllerId, window).inputs;
			
		}

		isGamepad1Connected = glfwJoystickPresent(GLFW_JOYSTICK_1);
		isGamepad2Connected = glfwJoystickPresent(GLFW_JOYSTICK_2);
		


		
	

		// DEBUG: Check if controllers are detected
		//if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
		//	std::cout << "Controller 1 is connected" << std::endl;
		//}
		//if (glfwJoystickPresent(GLFW_JOYSTICK_2)) {
		//	std::cout << "Controller 2 is connected" << std::endl;
		//}

	}

	void clearInputs() {
		for (auto& entity : ecs.playerInputs.entities) {
			PlayerInput& playerInput = ecs.playerInputs.get(entity);
			playerInput.clear();
		}
	}

	void setAllInputs(INPUT_ID inputId, unsigned int controllerId, bool doPress) {
		for (auto& entity : ecs.playerInputs.entities) {
			PlayerInput& playerInput = ecs.playerInputs.get(entity);
			if (playerInput.controllerId == controllerId) {
				if (doPress)	playerInput.press(inputId);
				else			playerInput.release(inputId);
			}
		}
	}


	PlayerInput getGamepadInput(unsigned int controllerId, int joystickId) {
		const float DEADZONE = 0.2f;
		PlayerInput result;

		if (glfwJoystickPresent(joystickId)) {
			int axesCount;
			const float* axes = glfwGetJoystickAxes(joystickId, &axesCount);

			int buttonCount;
			const unsigned char* buttons = glfwGetJoystickButtons(joystickId, &buttonCount);

			if (axes && buttons) {
				// Left stick X and Y
				Vector2 stickPos = Vector2(
					std::abs(axes[0]) > DEADZONE ? axes[0] : 0.0f,
					std::abs(axes[1]) > DEADZONE ? axes[1] : 0.0f
				);

				if (stickPos.length() < DEADZONE) {
					stickPos = Vector2::zero();
				} else {
					stickPos = Vector2::right().rotated(roundToNearest(stickPos.angle(), std::numbers::pi / 4));
				}


				// Update directional inputs based on left stick
				result.set(INPUT_ID::RIGHT, stickPos.x > DEADZONE || buttons[11]);
				result.set(INPUT_ID::LEFT, stickPos.x < -DEADZONE || buttons[13]);
				result.set(INPUT_ID::DOWN, stickPos.y > DEADZONE || buttons[12]);
				result.set(INPUT_ID::UP, stickPos.y < -DEADZONE || buttons[10]);

				// Map controller buttons to actions
				result.set(INPUT_ID::JUMP, buttons[3] == GLFW_PRESS);
				result.set(INPUT_ID::ATTACK, buttons[1] == GLFW_PRESS);
				result.set(INPUT_ID::SPECIAL,	buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS 
											|| buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS);

				result.set(INPUT_ID::PAUSE, buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS);
			}
		}

		return result;
	}

	PlayerInput getKeyboardInput(unsigned int controllerId, GLFWwindow* window) {
		PlayerInput result;

		switch (controllerId) {
		case 1: {
			result.set(INPUT_ID::UP, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
			result.set(INPUT_ID::LEFT, glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
			result.set(INPUT_ID::DOWN, glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
			result.set(INPUT_ID::RIGHT, glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
			result.set(INPUT_ID::JUMP, glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
			result.set(INPUT_ID::ATTACK, glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS);
			result.set(INPUT_ID::SPECIAL, glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS);
			result.set(INPUT_ID::DEBUG, glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS);
			result.set(INPUT_ID::PAUSE, glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
		} break;
		case 2: {
			result.set(INPUT_ID::UP, glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
			result.set(INPUT_ID::LEFT, glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
			result.set(INPUT_ID::DOWN, glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
			result.set(INPUT_ID::RIGHT, glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
			result.set(INPUT_ID::JUMP, glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS);
			result.set(INPUT_ID::ATTACK, glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS);
			result.set(INPUT_ID::SPECIAL, glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS);
			result.set(INPUT_ID::PAUSE, glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
		}
		}

		return result;
		
	}



	


	// M1 [4] Keyboard/mouse control
	void onKey(int key, int, int action, int mods) {

		if (action == GLFW_PRESS) {
			switch (key) {
			//case GLFW_KEY_I: saveArena(arenasPath("untitled.txt")); break;
			case GLFW_KEY_P: printTilemap(); break; // print the current on-screen tilemap
			//case GLFW_KEY_U: toggleAI(); break;
			case GLFW_KEY_O: debug.flags ^= (int)DEBUG_FLAGS::SHOW_COLLISION_BOXES; break;
			case GLFW_KEY_R: sceneSystem.resetGame(); break;

			//case GLFW_KEY_T: tutorial.start(); break;
			// case GLFW_KEY_M: sceneSystem.nextScene(); break;

			//case GLFW_KEY_1: gameState.currentState = GAME_SCREEN_ID::PLAYING; break;
			//case GLFW_KEY_2: gameState.currentState = GAME_SCREEN_ID::DRAWING_MAIN; break;
			//case GLFW_KEY_3: gameState.currentState = GAME_SCREEN_ID::DRAWING_BACKGROUND; break;
			//case GLFW_KEY_4: gameState.currentState = GAME_SCREEN_ID::TILE_SELECTOR; break;
			
			
			}
			isAnyInputJustPressed = true;

			
		}

		

	};

	static void toggleAI() {
		if (gameState.currentState == GAME_SCREEN_ID::CHARACTER_SELECT) {
			cout << "AI cannot be toggled in character select" << endl;
			return;
		}

		static bool toggleP1 = false;
		toggleP1 = !toggleP1;

		auto& playerOptions = toggleP1 ? gameState.player1Options : gameState.player2Options;
		Entity player = getPlayer(toggleP1 ? 1 : 2);

		// Check if player is not a knight
		if (ecs.players.get(player).class_id != CLASS_ID::KNIGHT) {
			cout << "Only Knights can be AI controlled" << endl;
			return;
		}

		playerOptions.playerType = (PLAYER_TYPE_ID)(((int)playerOptions.playerType + 1) % (int)PLAYER_TYPE_ID::COUNT);

		if (player == Entity::null()) return;
		if (!ecs.aiAgents.has(player)) {
			cout << "Toggle CPU on for Player " << (toggleP1 ? "1" : "2") << endl;
			AIAgent& ai = ecs.aiAgents.emplace(player);
			ai.class_id = ecs.players.get(player).class_id;
		}
		else {
			cout << "Toggle CPU off for Player " << (toggleP1 ? "1" : "2") << endl;
			ecs.aiAgents.remove(player);
		}
	}

	void onMouseMotion(Vector2 mousePosition) {
		mousePosX = mousePosition.x;
		mousePosY = mousePosition.y;
	};

	// M1 [4] Keyboard/mouse control
	void onMouseButtonPressed(int button, int action, int mods) {
		if (action == GLFW_PRESS && gameState.currentState != GAME_SCREEN_ID::CHARACTER_SELECT) {
			Vector2 windowSize = WindowHelpers::getWindowSize(window);
			int tileX = (int)(mousePosX / windowSize.x * TILEMAP_W);
			int tileY = (int)(mousePosY / windowSize.y * TILEMAP_H);
			vec2 tilePos = vec2(tileX, tileY);
			Tilemap<30, 17>& tilemap = ecs.tilemaps.components[0];

			cout << "mouse position: " << mousePosX << ", " << mousePosY << endl;
			cout << "mouse tile position: " << tileX << ", " << tileY << endl;

			bool tileHasBeenModified = false;

			// Can only draw when in drawing state
			// Modify this when adding new tiles
			// Will also need to modify to add/remove background tiles with DRAWING_BACKGROUND
			//      and tile selector with TILE_SELECTOR
			if (gameState.currentState == GAME_SCREEN_ID::DRAWING_MAIN) {
				if (button == GLFW_MOUSE_BUTTON_LEFT) {
					tilemap.getTile(tilePos).id = 1;
					tileHasBeenModified = true;
				}

			// Erase tile
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				if (tileX != 0 && tileX != TILEMAP_W - 1 && 
					tileY != 0 && tileY != TILEMAP_H - 1) {
					tilemap.getTile(tilePos).id = 0;
					tileHasBeenModified = true;
				}
			}

			if (tileHasBeenModified)
				updateTilemap(ecs.tilemaps.entities[0], true);
			}
		}
	};
};