
module;

#include <array>
#include <assert.h>

export module Ramp;
import Color;
import Range;

export template <size_t _Size>
struct ColorRamp {
	std::array< float, _Size> points;
	std::array< Color, _Size> values;


	ColorRamp(std::array< float, _Size> p, std::array< Color, _Size> v) {
		points = p;
		values = v;
		verify();
	}

	bool verify() const {
		return points.size() == values.size();
	}


	Color getAt(float point) const {
		assert(verify());
		assert(point >= points.front() && point <= points.back());
		unsigned int endIndex = 0;
		while (point >= points[endIndex] && endIndex < points.size()) endIndex++;
		
		float startPoint = points[endIndex - 1];
		float endPoint = points[endIndex];

		Color startColor = values[endIndex - 1];
		Color endColor = values[endIndex];

		return startColor.lerp(endColor, Range(startPoint, endPoint).unlerpPoint(point));
	}
};