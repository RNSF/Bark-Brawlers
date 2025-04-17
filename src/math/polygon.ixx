module;

#include <vector>
#include <glm/glm.hpp>
#include <iostream>

export module Polygon;
import Vector2;
import Transform;
import Range;
import Rect;

export struct ConvexPolygon {
	std::vector<Vector2> points; // counter clockwise

	static const ConvexPolygon fromRect(const Rect rect) {
		ConvexPolygon polygon = ConvexPolygon({
			rect.topLeft(),
			rect.bottomLeft(),
			rect.bottomRight(),
			rect.topRight(),
		});

		assert(polygon.verify());
		return polygon;
	}

	void transform(Transform trans) {
		for (size_t i = 0; i < points.size(); i++) {
			points[i] *= trans.getScale();
			points[i] = points[i].rotated(trans.getRotation());
			points[i] += trans.getTranslation();
		}

		assert(verify());
	}

	const std::vector<glm::vec3> getRenderVerticies(float z = 0.0f) const {

		std::vector<glm::vec3> result;
		result.resize(points.size());

		for (size_t i = 0; i < points.size(); i++) {
			result[i] = { points[i].x, points[i].y, z };
		}

		return result;
	}

	const std::vector<uint16_t> getRenderIndices() const {
		uint16_t lower = 0;
		uint16_t upper = points.size() - 1;
		std::vector<uint16_t> result;
		result.resize((points.size() - 2) * 3);
		

		for (size_t i = 0; i < result.size(); i += 3) {
			bool isLowerMain = lower + upper == points.size() - 1;

			if (isLowerMain) {
				result[i] = lower;
				result[i + 1] = lower + 1;
				result[i + 2] = upper;
				lower++;
			} else {
				result[i] = upper - 1;
				result[i + 1] = upper;
				result[i + 2] = lower;
				upper--;
			}
		};

		return result;
	};

	Range projectOntoLine(Vector2 line) const {
		assert(std::abs(line.length() - 1.0f) <= FLT_EPSILON && "line must be normalized");

		Range result = Range::reverseInfinite();

		for (size_t i = 0; i < points.size(); i++) {
			float proj = line.dot(points[i]);
			result = result.expand(Range::point(proj));
		}
		
		return result;
	}


	// uses Separating Axis Theorem. Read about it here: https://dyn4j.org/2010/01/sat/
	bool overlaps(ConvexPolygon& other) const {

		/*for (int i = 0; i < points.size(); i++) {
			std::cout << "(" << points[i].x << ", " << points[i].y << ")" << "; ";
		}
		std::cout << std::endl;*/

		const ConvexPolygon* p[] = {this, &other};
		for (const ConvexPolygon* polygon : p) {
		for (int i = 0; i < polygon->points.size(); i++) {
			Vector2 point1 = polygon->points[i];
			Vector2 point2 = polygon->points[(i + 1) == polygon->points.size() ? 0 : i + 1];

			Vector2 line = (point2 - point1).perpindicular().normalize();

			Range range1 = projectOntoLine(line);
			Range range2 = other.projectOntoLine(line);

			if (!range1.overlaps(range2)) {
				return false;
			}
		}}

		return true;
	}

	// verify this polygon is actually convex. 
	bool verify() const {
		for (int i = 0; i < points.size(); i++) {
			Vector2 point1 = points[i];
			Vector2 point2 = points[(i + 1) % points.size()];
			Vector2 point3 = points[(i + 2) % points.size()];

			Vector2 vec1 = point2 - point1;
			Vector2 vec2 = point3 - point2;

			if (vec1.crossZ(vec2) >= 0.0f) {
				return false;
			}
		}

		return true;
	}

	// returns smallest possible AABB that bounds this polygon
	Rect getAABB() const {

		Rect result = Rect::reverseInfinite();

		for (size_t i = 0; i < points.size(); i++) {
			result.expand(Rect::point(points[i]));
		}

		return result;
	}
};

