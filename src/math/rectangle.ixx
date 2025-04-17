module;

#include <utility>
#include <limits>

export module Rect;
import Vector2;

export struct Rect {
	float left;
	float top;
	float right;
	float bottom;


	static const Rect zero() {
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	static const Rect unit() {
		return { 0.0f, 0.0f, 1.0f, 1.0f };
	}

	static const Rect fromTopLeftSize(Vector2 topLeft, Vector2 size) {
		return { topLeft.x, topLeft.y, topLeft.x + size.x, topLeft.y + size.y };
	}

	static const Rect centeredSquare(float halfWidth) {
		return { -halfWidth, -halfWidth, halfWidth, halfWidth };
	}

	static const Rect infinite() {
		return {
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity()
		};
	}

	static const Rect reverseInfinite() {
		return {
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity()
		};
	}

	static const Rect point(Vector2 p) {
		return {
			p.x,
			p.y,
			p.x,
			p.y
		};
	}

	Vector2 size() const {
		return { right - left, bottom - top };
	}

	Vector2 center() const {
		return { (left + right) / 2, (top + bottom) / 2 };
	}

	Vector2 topLeft() const {
		return { left, top };
	}

	Vector2 topRight() const {
		return { right, top };
	}

	Vector2 bottomRight() const {
		return { right, bottom };
	}

	Vector2 bottomLeft() const {
		return { left, bottom };
	}

	Rect translate(Vector2 amount) const {
		Rect newRect = Rect(*this);

		newRect.left += amount.x;
		newRect.right += amount.x;
		newRect.top += amount.y;
		newRect.bottom += amount.y;

		return newRect;
	}

	Rect scale(Vector2 amount) const {
		Rect newRect = Rect(*this);

		newRect.left *= amount.x;
		newRect.right *= amount.x;
		newRect.top *= amount.y;
		newRect.bottom *= amount.y;

		return newRect;
	}

	Rect overlappingRect(Rect other) const {
		return {
			std::max(left, other.left),
			std::max(top, other.top),
			std::min(right, other.right),
			std::min(bottom, other.bottom)
		};
	}

	bool overlaps(Rect other) const {
		Vector2 overlapSize = overlappingRect(other).size();
		return overlapSize.x >= 0 && overlapSize.y >= 0;
	}

	bool overlapsPoint(Vector2 point) const {
		return point.x > left && point.y > top && point.x < right && point.y < bottom;
	}

	Rect shrink(const Rect& other) const {
		return { 
			std::max(left,   other.left), 
			std::max(top,	 other.top),
			std::min(right,  other.right),
			std::min(bottom, other.bottom),
		};
	}

	Rect expand(const Rect& other) const {
		return {
			std::min(left,   other.left),
			std::min(top,	 other.top),
			std::max(right,  other.right),
			std::max(bottom, other.bottom),
		};
	}
};