#ifndef numeric_array_h
#define numeric_array_h

#include <cmath>

class double3 {
public:
    // Members
    double x, y, z;

    // Constructors
    double3() : x(0.0), y(0.0), z(0.0) {}
    double3(double x, double y, double z) : x(x), y(y), z(z) {}
    
    double& operator[](uint32_t i) {
        switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::out_of_range("double3 index out of range");
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

    double3 normalized() const
    {
        double len = length();
        if (len > 1e-12)
        {
            return *this / len;
        }
        return *this;
    }

    void normalize() {
        double len = length();
        if (len > 1e-12) {
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
};

// Scalar * Vector (commutative multiply)
inline double3 operator*(double scalar, const double3& v) {
    return double3(v.x * scalar, v.y * scalar, v.z * scalar);
}


#endif
