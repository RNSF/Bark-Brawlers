module;

#include <common.hpp>

export module Color;

export struct Color {
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;


	// grayscale
	static Color white() {
		return { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	static Color black() {
		return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	static Color gray(float brightness) {
		return { brightness, brightness, brightness, 1.0f };
	}

	// primary
	static Color red() {
		return { 1.0f, 0.0f, 0.0f, 1.0f };
	}

	static Color green() {
		return { 0.0f, 1.0f, 0.0f, 1.0f };
	}

	static Color blue() {
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	// secondary
	static Color yellow() {
		return { 1.0f, 1.0f, 0.0f, 1.0f };
	}

	
	static Color cyan() {
		return { 0.0f, 1.0f, 1.0f, 1.0f };
	}

	static Color magenta() {
		return { 1.0f, 0.0f, 1.0f, 1.0f };
	}

	

	// tertiary
	static Color orange() {
		return { 1.0f, 0.5f, 0.0f, 1.0f };
	}

	static Color lime() {
		return { 0.5f, 1.0f, 0.0f, 1.0f };
	}

	static Color azure() {
		return { 0.0f, 0.5f, 1.0f, 1.0f };
	}

	static Color springGreen() {
		return { 0.0f, 1.0f, 0.5f, 1.0f };
	}

	static Color rose() {
		return { 1.0f, 0.0f, 0.5f, 1.0f };
	}

	static Color violet() {
		return { 0.5f, 0.0f, 1.0f, 1.0f };
	}


	// other
	static Color clear() {
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	operator glm::vec3() const {
		return {r, g, b};
	}

	operator glm::vec4() const {
		return { r, g, b, a };
	}

	// M1 interpolation implementation
	Color lerp(const Color& other, float amount) const {
		return {
			std::lerp(r, other.r, amount),
			std::lerp(g, other.g, amount),
			std::lerp(b, other.b, amount),
			std::lerp(a, other.a, amount)
		};
	}

	
};