
module;

#include <limits>
#include <utility>
#include <cmath>
#include <algorithm>

export module Range;
import RNG;

export struct Range {
	float lower = 0.0;
	float upper = 0.0;

	static const Range infinite() {
		return {
			-std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity()
		};
	}

	static const Range reverseInfinite() {
		return {
			std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity()
		};
	}

	static const Range point(float value) {
		return { value, value };
	}

	Range shrink(const Range& other) const  {
		return { std::max(this->lower, other.lower), std::min(this->upper, other.upper) };
	}

	Range expand(const Range& other) const {
		return { std::min(this->lower, other.lower), std::max(this->upper, other.upper) };
	}

	float length() {
		return upper - lower;
	}

	bool overlaps(const Range& other) const {
		return shrink(other).length() > 0;
	}

	float randomPoint() {
		return std::lerp(lower, upper, randomFloat());
	}

	float modPoint(float value) {
		return fmod(value - lower, length()) + lower;
	}

	float unlerpPoint(float value) {
		return (value - lower) / (upper - lower);
	}

	float lerpedPoint(float amount) {
		return (upper - lower) * amount + lower;
	}

	float lerpedPointClamped(float amount) {
		return lerpedPoint(std::clamp(amount, 0.0f, 1.0f));
	}

	float remapValue(Range other, float value) {
		return lerpedPoint(other.unlerpPoint(value));
	}

	float remapValueClamped(Range other, float value) {
		return other.lerpedPointClamped(unlerpPoint(value));
	}

	bool hasPointInc(float point) {
		return point >= lower && point <= upper;
	}
};