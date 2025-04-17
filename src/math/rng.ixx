module;

#include <random>

export module RNG;

export {

	// returns random float in [0.0, 1.0)
	float randomFloat() {
		std::random_device rd; 
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);

		return dis(gen);
	}

	// returns a random int between min and max, inclusive
	float randomIntRange(int min, int max) {
		return rand() % (max - min + 1) + min;
	}
}