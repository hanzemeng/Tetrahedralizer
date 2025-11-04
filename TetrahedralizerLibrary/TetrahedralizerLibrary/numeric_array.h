#ifndef numeric_array_h
#define numeric_array_h

#include <cmath>
#include <iostream>

class double2 {
public:
    // Members
    double x, y;

    // Constructors
    double2() : x(0.0), y(0.0) {}
    double2(double x, double y) : x(x), y(y) {}
    
    double& operator[](uint32_t i) {
        switch (i) {
            case 0: return x;
            case 1: return y;
            default: throw std::out_of_range("double4 index out of range");
        }
    }

    // Basic arithmetic operators
    double2 operator+(const double2& other) const {
        return double2(x + other.x, y + other.y);
    }

    double2 operator-(const double2& other) const {
        return double2(x - other.x, y - other.y);
    }

    double2 operator*(double scalar) const {
        return double2(x * scalar, y * scalar);
    }

    double2 operator/(double scalar) const {
        return double2(x / scalar, y / scalar);
    }

    // Compound assignment
    double2& operator+=(const double2& other) {
        x += other.x; y += other.y;
        return *this;
    }

    double2& operator-=(const double2& other) {
        x -= other.x; y -= other.y;
        return *this;
    }

    double2& operator*=(double scalar) {
        x *= scalar; y *= scalar;
        return *this;
    }

    double2& operator/=(double scalar) {
        x /= scalar; y /= scalar;
        return *this;
    }

    // Unary operators
    double2 operator-() const {
        return double2(-x, -y);
    }

    // Comparison
    bool operator==(const double2& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const double2& other) const {
        return !(*this == other);
    }

    // Vector operations
    double dot(const double2& other) const {
        return x * other.x + y * other.y;
    }

    double cross(const double2& other) const {
        // 2D "cross product" result is scalar (z-component of 3D cross)
        return x * other.y - y * other.x;
    }

    double length() const {
        return std::sqrt(x * x + y * y);
    }

    double length_squared() const {
        return x * x + y * y;
    }

    double2 normalized() const {
        double len = length();
        if (len == 0.0) return double2(0.0, 0.0);
        return *this / len;
    }

    void normalize() {
        double len = length();
        if (len != 0.0) {
            x /= len; y /= len;
        }
    }
    
    double2 min(const double2& other) const {
        return double2(
                       std::min(x, other.x),
                       std::min(y, other.y)
        );
    }
    double2 max(const double2& other) const {
        return double2(
                       std::max(x, other.x),
                       std::max(y, other.y)
        );
    }

    // Utility
    friend std::ostream& operator<<(std::ostream& os, const double2& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }
};

// Scalar * double2 (commutative multiply)
inline double2 operator*(double scalar, const double2& v) {
    return double2(v.x * scalar, v.y * scalar);
}


class double3 {
public:
    // Members
    double x, y, z;

    // Constructors
    double3() : x(0.0), y(0.0), z(0.0) {}
    double3(double x, double y, double z) : x(x), y(y), z(z) {}
    double3(const double2& o) : x(o.x), y(o.y), z(0.0) {}
    
    double& operator[](uint32_t i) {
        switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::out_of_range("double4 index out of range");
        }
    }

    // Basic arithmetic operators
    double3 operator+(const double3& other) const {
        return double3(x + other.x, y + other.y, z + other.z);
    }

    double3 operator-(const double3& other) const {
        return double3(x - other.x, y - other.y, z - other.z);
    }

    double3 operator*(double scalar) const {
        return double3(x * scalar, y * scalar, z * scalar);
    }

    double3 operator/(double scalar) const {
        return double3(x / scalar, y / scalar, z / scalar);
    }

    // Compound assignment
    double3& operator+=(const double3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    double3& operator-=(const double3& other) {
        x -= other.x; y -= other.y; z -= other.z;
        return *this;
    }

    double3& operator*=(double scalar) {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }

    double3& operator/=(double scalar) {
        x /= scalar; y /= scalar; z /= scalar;
        return *this;
    }

    // Unary operators
    double3 operator-() const {
        return double3(-x, -y, -z);
    }

    // Comparison
    bool operator==(const double3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const double3& other) const {
        return !(*this == other);
    }

    // Vector operations
    double dot(const double3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    double3 cross(const double3& other) const {
        return double3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    double length_squared() const {
        return x * x + y * y + z * z;
    }

    double3 normalized() const {
        double len = length();
        if (len == 0.0) return double3(0.0, 0.0, 0.0);
        return *this / len;
    }

    void normalize() {
        double len = length();
        if (len != 0.0) {
            x /= len; y /= len; z /= len;
        }
    }
    
    double3 min(const double3& other) const {
        return double3(
                       std::min(x, other.x),
                       std::min(y, other.y),
                       std::min(z, other.z)
        );
    }
    double3 max(const double3& other) const {
        return double3(
                       std::max(x, other.x),
                       std::max(y, other.y),
                       std::max(z, other.z)
        );
    }

    // Utility
    friend std::ostream& operator<<(std::ostream& os, const double3& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }
};

// Scalar * Vector (commutative multiply)
inline double3 operator*(double scalar, const double3& v) {
    return double3(v.x * scalar, v.y * scalar, v.z * scalar);
}


class double4 {
public:
    // Members
    double x, y, z, w;

    // Constructors
    double4() : x(0.0), y(0.0), z(0.0), w(0.0) {}
    double4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
    double4(const double2& o) : x(o.x), y(o.y), z(0.0), w(0.0) {}
    double4(const double3& o) : x(o.x), y(o.y), z(o.z), w(0.0) {}
    
    double& operator[](uint32_t i) {
        switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            case 3: return w;
            default: throw std::out_of_range("double4 index out of range");
        }
    }

    // Basic arithmetic operators
    double4 operator+(const double4& other) const {
        return double4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    double4 operator-(const double4& other) const {
        return double4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    double4 operator*(double scalar) const {
        return double4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    double4 operator/(double scalar) const {
        return double4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    // Compound assignment
    double4& operator+=(const double4& other) {
        x += other.x; y += other.y; z += other.z; w += other.w;
        return *this;
    }

    double4& operator-=(const double4& other) {
        x -= other.x; y -= other.y; z -= other.z; w -= other.w;
        return *this;
    }

    double4& operator*=(double scalar) {
        x *= scalar; y *= scalar; z *= scalar; w *= scalar;
        return *this;
    }

    double4& operator/=(double scalar) {
        x /= scalar; y /= scalar; z /= scalar; w /= scalar;
        return *this;
    }

    // Unary operators
    double4 operator-() const {
        return double4(-x, -y, -z, -w);
    }

    // Comparison
    bool operator==(const double4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    bool operator!=(const double4& other) const {
        return !(*this == other);
    }

    // Vector operations
    double dot(const double4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    double length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    double length_squared() const {
        return x * x + y * y + z * z + w * w;
    }

    double4 normalized() const {
        double len = length();
        if (len == 0.0) return double4(0.0, 0.0, 0.0, 0.0);
        return *this / len;
    }

    void normalize() {
        double len = length();
        if (len != 0.0) {
            x /= len; y /= len; z /= len; w /= len;
        }
    }

    // Utility
    friend std::ostream& operator<<(std::ostream& os, const double4& v) {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }
};

// Scalar * double4 (commutative multiply)
inline double4 operator*(double scalar, const double4& v) {
    return double4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
}

#endif
