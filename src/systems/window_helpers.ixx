module;

#include <GLFW/glfw3.h>

export module WindowHelpers;
import Vector2;

export namespace WindowHelpers {

	Vector2 getWindowSize(GLFWwindow* gameWindow) {
		int WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX;
		glfwGetWindowSize(gameWindow, &WINDOW_WIDTH_PX, &WINDOW_HEIGHT_PX);
		return Vector2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
	}

}