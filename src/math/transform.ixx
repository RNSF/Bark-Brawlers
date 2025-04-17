module;

#include "common.hpp"

export module Transform;
import Vector2;
import Math;


export struct Basis {
	glm::mat2 matrix = { {1.0f, 0.0f}, {0.0f, 1.0f}};



	static const Basis withScaleRotation(Vector2 s, float r) {
		Basis result;
		result.scale(s);
		result.rotate(r);
		return result;
	}

	void scale(Vector2 scale) {
		glm::mat2 S = { { scale.x, 0.f },{ 0.f, scale.y } };
		matrix = matrix * S;
	}

	void rotate(float radians) {
		float c = cosf(radians);
		float s = sinf(radians);
		glm::mat2 R = { { c, s },{ -s, c } };
		matrix = matrix * R;
	}
};


export struct Transform {
	glm::mat3 matrix = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f } };

	Transform operator*(const Transform& other) const {
		return Transform(matrix * other.matrix);
	}

	static const Transform withTranslation(Vector2 trans) {
		Transform result;
		result.translate(trans);
		return result;
	}

	static const Transform withTranslation(Vector2 trans, Vector2 scale) {
		Transform result;
		result.translate(trans);
		result.scale(scale);
		return result;
	}

	void scale(Vector2 scale) {
		glm::mat3 S = { { scale.x, 0.f, 0.f },{ 0.f, scale.y, 0.f },{ 0.f, 0.f, 1.f } };
		matrix = matrix * S;
	}

	void rotate(float radians) {
		float c = cosf(radians);
		float s = sinf(radians);
		glm::mat3 R = { { c, s, 0.f },{ -s, c, 0.f },{ 0.f, 0.f, 1.f } };
		matrix = matrix * R;
	}

	void translate(Vector2 offset) {
		glm::mat3 T = { { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ offset.x, offset.y, 1.f } };
		matrix = matrix * T;
	}

	Vector2 getTranslation() const {
		return { matrix[2][0] , matrix[2][1] };
	}

	float getScaleX() const {
		return Vector2(matrix[0][0], matrix[1][0]).length();
	}

	float getScaleY() const {
		return Vector2(matrix[0][1], matrix[1][1]).length();
	}

	Vector2 getScale() const {
		return { getScaleX(), getScaleY()};
	}

	float getRotation() const {
		return atan2f(-matrix[0][1], matrix[0][0]);
	}

	/*Vector2 getSignedScale() const {
		float rotation = getRotation();
		return Vector2(matrix[0][0] * cosf(rotation) - matrix[1][0] * sinf(rotation), - matrix[0][1] * sinf(rotation) + matrix[1][1] * cosf(rotation));
	}*/

	glm::mat2 getBasis() const {
		return glm::mat2(matrix);
	}

	void setTranslation(Vector2 newTranslation) {
		matrix[2][0] = newTranslation.x;
		matrix[2][1] = newTranslation.y;
	}

	/*void setScale(Vector2 newScale) {
		Vector2 currentScale = getScale();
		scale(Vector2::one() / currentScale * newScale);
	}*/

	/*Vector2 setSignedScale(Vector2 newScale) const {
		Vector2 currentScale = getSignedScale();
		scale(Vector2::one() / currentScale * newScale);
	}*/

	/*void setRotation(float newRotation) {
		rotate(newRotation - getRotation());
	}*/

	Vector2 operator*(const Vector2& other) const {
		return Vector2(other.x * matrix[0][0] + other.y * matrix[0][1] + matrix[0][2], other.x * matrix[1][0] + other.y * matrix[1][1] + matrix[1][2]) + Vector2(getTranslation());
	}


};