module;

#include <cmath>

export module Math;

export {

	float signf(float num1) {
		if (num1 > 0.0f) return 1.0f;
		if (num1 < 0.0f) return -1.0f;
		return 0.0f;
	}

	// min is inclusive, max is non inclusive
	int wrapMod(int number, int min, int max) {

		int width = max - min;
		number -= min;
		number += (1 - number / width) * width;
		number = number % width;
		return number + min;
	}

	float roundToNearest(float number, float roundTo) {
		return std::round(number / roundTo) * roundTo;
	}
}