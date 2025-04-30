
module;

#include <GLFW/glfw3.h>

export module Singletons;
import Components;

export {


	GLFWwindow* window;
	Debug debug;
	Tutorial tutorial;
	GameState gameState;
	Camera defaultCamera = Camera(
		Vector2::zero(),
		{ 1920.0f, 1080.0f },
		Vector2::unitCenter(),
		0.25f,
		0.0f,
		false
	);

	float gameTime;
	float sceneTime = 0.0f;

	float mousePosX = 0.0f;
	float mousePosY = 0.0f;

	bool isTutorialCompleted = false;
	bool isAnyInputJustPressed = false;

	bool isGamepad1Connected = false;
	bool isGamepad2Connected = false;
}