module;

#include <glm/glm.hpp>
#include <string>
#include <format>
#include <algorithm>

export module Vector2;

export struct Vector2 {
	float x = 0.0f;
	float y = 0.0f;

    Vector2() {
        x = 0.0f;
        y = 0.0f;
    }

    Vector2(double x_d, double y_d) {
        x = (float) x_d;
        y = (float) y_d;
    }

    Vector2(glm::vec2 v) {
        x = (float) v.x;
        y = (float) v.y;
    }

    Vector2(glm::uvec2 v) {
        x = (float)v.x;
        y = (float)v.y;
    }

    Vector2(glm::ivec2 v) {
        x = (float)v.x;
        y = (float)v.y;
    }

    static const Vector2 zero() {
        return { 0, 0 };
    }

    static const Vector2 one() {
        return { 1, 1 };
    }

    static const Vector2 right() {
        return { 1, 0 };
    }

    static const Vector2 left() {
        return { -1, 0 };
    }

    static const Vector2 up() {
        return { 0, -1 };
    }

    static const Vector2 down() {
        return { 0, 1 };
    }

    static const Vector2 unitTopLeft() {
        return { 0.0, 0.0 };
    }

    static const Vector2 unitTopRight() {
        return { 0.0, 1.0 };
    }

    static const Vector2 unitBottomLeft() {
        return { 1.0, 0.0 };
    }

    static const Vector2 unitBottomRight() {
        return { 1.0, 1.0 };
    }

    static const Vector2 unitCenter() {
        return { 0.5, 0.5 };
    }

    static const Vector2 flippedX() {
        return { -1.0f, 1.0f };
    };

    static const Vector2 flippedY() {
        return { 1.0f, -1.0f };
    }

    Vector2 operator+(const Vector2& other) const {
        return { x + other.x, y + other.y };
    }

    Vector2 operator-(const Vector2& other) const {
        return { x - other.x, y - other.y };
    }

    Vector2 operator-() const {
        return { -x, -y };
    }

    Vector2 operator*(const Vector2& other) const {
        return { x * other.x, y * other.y };
    }

    Vector2 operator/(const Vector2& other) const {
        return { x / other.x, y / other.y };
    }

    Vector2 operator*(const float& other) const {
        return { x * other, y * other };
    }

    Vector2 operator/(const float& other) const {
        return { x / other, y / other };
    }

    bool operator==(const Vector2& other) const {
        return other.x == x && other.y == y;
    }

    Vector2& operator+=(const Vector2& other) {
        *this = *this + other;
        return *this;
    }

    Vector2& operator-=(const Vector2& other) {
        *this = *this - other;
        return *this;
    }

    Vector2& operator*=(const Vector2& other) {
        *this = *this * other;
        return *this;
    }

    Vector2& operator/=(const Vector2& other) {
        *this = *this / other;
        return *this;
    }

    Vector2& operator*=(const float& other) {
        *this = *this * other;
        return *this;
    }

    Vector2& operator/=(const float& other) {
        *this = *this / other;
        return *this;
    }

    operator glm::vec2() const {  return {x, y}; }

    operator glm::ivec2() const { return { (int)x, (int)y }; }

    operator std::string() const {
        return std::format("({}, {})", x, y);
    }

    Vector2 swap() const {
        return { y, x };
    }

    Vector2 floor() const {
        return { std::floor(x), std::floor(y) };
    }

    Vector2 ceil() const {
        return { std::ceil(x), std::ceil(y) };
    }

    Vector2 round() const {
        return { std::round(x), std::round(y) };
    }

    float maxComponent() const {
        return std::max(x, y);
    }

    float minComponent() const {
        return std::min(x, y);
    }

    float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    Vector2 min(const Vector2& other) const {
        return {
            std::min(x, other.x),
            std::min(y, other.y)
        };
    }

    Vector2 max(const Vector2& other) const {
        return {
            std::max(x, other.x),
            std::max(y, other.y)
        };
    }

    Vector2 clamp(const Vector2& min, const Vector2& max) const {
        return {
            std::clamp(x, min.x, max.x),
            std::clamp(y, min.y, max.y)
        };
    }

    Vector2 lerp(const Vector2& other, float amount) const {
        return {
            std::lerp(x, other.x, amount),
            std::lerp(y, other.y, amount)
        };
    }

    // returns a perpindicular vector
    Vector2 perpindicular() const {
        return { -y, x };
    }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    float lengthSqr() const {
        return x * x + y * y;
    }

    float distanceTo(const Vector2& other) const {
        return (other - *this).length();
    }

    float distanceToSqr(const Vector2& other) const {
        return (other - *this).lengthSqr();
    }

    Vector2 normalize() const {
        return (*this) / (*this).length();
    }

    Vector2 normalizeSafe() const {
        float l = length();
        if (l == 0) return Vector2::zero();
        return (*this) / l;
    }


    Vector2 rotated(float angle) const {
        return { x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle) };
    }

    
    float crossZ(Vector2& other) const {
        return x * other.y - y * other.x;
    }

    float angle() const {
        return atan2f(y, x);
    }

};