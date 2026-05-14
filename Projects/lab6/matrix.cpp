#include "matrix.hpp"

Vec2::Vec2(Vec3 v): x(v.x), y(v.y) {}

Vec3::Vec3(Vec4 v): x(v.x), y(v.y), z(v.z) {}

inline float dot(Vec2 v1, Vec2 v2) {
    Vec2 tmp = v1 * v2;
    return tmp.x + tmp.y;
}

float length(Vec3 p) {
    return sqrtf(dot(p, p));
}

float dot(const Vec3 &a, const Vec3 &b) {
    Vec3 tmp = a * b;
    return tmp.x + tmp.y + tmp.z;
}

Vec2 normalize(const Vec3 &v) {
    return Vec2(v.x / v.z, v.y / v.z);
}

float dot(Vec4 v1, Vec4 v2) {
    Vec4 tmp = v1 * v2;
    return tmp.x + tmp.y + tmp.z + tmp.a;
}

Vec3 normalize(Vec4 v) {
    return Vec3(v.x / v.a, v.y / v.a, v.z / v.a);
}

Vec3 norm(Vec3 p) {
    return normalize(Vec4(p, length(p)));
}

Mat3 crossM(Vec3 p) {
    return Mat3(
        Vec3(0.f, -p.z, p.y),

        Vec3(p.z, 0.f, -p.x), Vec3(-p.y, p.x, 0.f)
    );
}

Vec3 cross(Vec3 p, Vec3 q) {
    return crossM(p) * q;
}
