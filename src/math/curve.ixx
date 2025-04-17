module;

#include <math.h>

export module Curve;

typedef float (*CurveFunc)(float);

export namespace Curve {
	
	float constant(float input) { return 1.0; }
	float zero(float input) { return 0.0; }
	float linear(float input) { return input; }

	template <float EXPONENT>
	float powerIn(float input) { return pow(input, EXPONENT); }

	template <float EXPONENT>
	float powerOut(float input) { return 1 - pow(1 - input, EXPONENT); }
};