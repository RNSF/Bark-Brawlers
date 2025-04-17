module;

#include <gl3w.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <glm/glm.hpp>
#include <common.hpp>

export module WorldSystem;
import Vector2;
import Components;


 
export class WorldSystem {

	GLFWwindow* window;

public:
	GLFWwindow* create_window() {
		///////////////////////////////////////
		// Initialize GLFW
		glfwSetErrorCallback([](int error, const char* desc) {
			std::cerr << error << ": " << desc << std::endl;
			});

		if (!glfwInit()) {
			std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
			return nullptr;
		}

		//-------------------------------------------------------------------------
		// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
		// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
		// GLFW / OGL Initialization
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
		// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

		// Create the main window (for rendering, keyboard, and mouse input)
		window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Bark Brawlers", nullptr, nullptr);
		if (window == nullptr) {
			std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
			return nullptr;
		}


		return window;
	};



	bool is_over() const {
		return bool(glfwWindowShouldClose(window));
	}

	void update_window_caption(float deltaTime) {
		float fps = 1.0f / deltaTime;
		//std::cout << "fps: " << fps << " deltaTime: " << deltaTime << std::endl;
		int roundedFps = static_cast<int>(std::round(fps));
		std::string caption = "Bark Brawlers - " + std::to_string(roundedFps) + " FPS";
		glfwSetWindowTitle(window, caption.c_str());
	}

};
