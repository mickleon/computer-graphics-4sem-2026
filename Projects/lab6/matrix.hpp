#pragma once

#include <cstddef>
#include <math.h>

struct Vec3;
struct Vec4;

struct Vec2 {
    float x = 0;
    float y = 0;

    Vec2() {}

    Vec2(float a, float b): x(a), y(b) {}

    Vec2(Vec3 v);

    Vec2 &operator*=(const Vec2 &v) {
        x *= v.x;
        y *= v.y;
        return *this;
    }

    Vec2 operator*(const Vec2 &v) const { return Vec2(*this) *= v; }

    float &operator[](int i) const { return ((float *)this)[i]; }
};

float dot(Vec2 v1, Vec2 v2);

struct Vec3 {
    float x = 0;
    float y = 0;
    float z = 0;

    Vec3() {}

    Vec3(float a, float b, float c): x(a), y(b), z(c) {}

    Vec3(Vec2 v, float c): Vec3(v.x, v.y, c) {}

    Vec3(Vec4 v);

    Vec3 &operator*=(const Vec3 &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    Vec3 operator*(const Vec3 &v) const { return Vec3(*this) *= v; }

    float &operator[](size_t i) { return (reinterpret_cast<float *>(this))[i]; }

    const float &operator[](size_t i) const { return (reinterpret_cast<const float *>(this))[i]; }

    Vec3 &operator+=(const Vec3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3 operator+(const Vec3 &v) const { return Vec3(*this) += v; }

    Vec3 &operator-=(const Vec3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vec3 operator-(const Vec3 &v) const { return Vec3(*this) -= v; }

    Vec3 &operator*=(const float &n) {
        (*this) *= Vec3(n, n, n);
        return *this;
    }

    Vec3 operator*(const float &n) const { return Vec3(*this) *= n; }
};

float length(Vec3 p);

float dot(const Vec3 &a, const Vec3 &b);

struct Vec4 {
    float x, y, z, a;

    Vec4() {}

    Vec4(float a, float b, float c, float d): x(a), y(b), z(c), a(d) {}

    Vec4(Vec3 v, float c): Vec4(v.x, v.y, v.z, c) {}

    Vec4 &operator*=(const Vec4 &v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        a *= v.a;
        return *this;
    }

    Vec4 operator*(const Vec4 &v) const { return Vec4(*this) *= v; }

    float &operator[](int i) const { return ((float *)this)[i]; }
};

Vec2 normalize(const Vec3 &v);

float dot(Vec4 v1, Vec4 v2);

Vec3 normalize(Vec4 v);

Vec3 norm(Vec3 p);

struct Mat4 {
    Vec4 row1, row2, row3, row4;

    Mat4() {}

    Mat4(Vec4 r1, Vec4 r2, Vec4 r3, Vec4 r4): row1(r1), row2(r2), row3(r3), row4(r4) {}

    Mat4(float a) {
        row1 = Vec4(a, 0.f, 0.f, 0.f);
        row2 = Vec4(0.f, a, 0.f, 0.f);
        row3 = Vec4(0.f, 0.f, a, 0.f);
        row4 = Vec4(0.f, 0.f, 0.f, a);
    }

    Vec4 &operator[](int i) const { return ((Vec4 *)this)[i]; }

    Mat4 &transpose() {
        Mat4 tmp(*this);
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                (*this)[i][j] = tmp[j][i];
            }
        }
        return *this;
    }

    Vec4 operator*(const Vec4 &v) const {
        Vec4 res{};
        for (int i = 0; i < 4; i++) {
            res[i] = dot((*this)[i], v);
        }
        return res;
    }

    Mat4 &operator*=(const Mat4 &m) {
        Mat4 A(*this), B(m);
        B.transpose();
        for (int i = 0; i < 4; i++) {
            (*this)[i] = A * B[i];
        }
        *this = transpose();
        return *this;
    }

    Mat4 operator*(const Mat4 &m) const { return Mat4(*this) *= m; }
};

struct Mat3 {
    Vec3 row1{};
    Vec3 row2{};
    Vec3 row3{};

    Mat3() {}

    Mat3(Vec3 r1, Vec3 r2, Vec3 r3): row1(r1), row2(r2), row3(r3) {}

    Mat3(float a) {
        row1 = Vec3(a, 0.f, 0.f);
        row2 = Vec3(0.f, a, 0.f);
        row3 = Vec3(0.f, 0.f, a);
    }

    Mat3(Mat4 m) {
        row1 = Vec3(m.row1.x, m.row1.y, m.row1.z);
        row2 = Vec3(m.row2.x, m.row2.y, m.row2.z);
        row3 = Vec3(m.row3.x, m.row3.y, m.row3.z);
    }

    Vec3 &operator[](size_t i) { return (reinterpret_cast<Vec3 *>(this))[i]; }

    const Vec3 &operator[](size_t i) const { return (reinterpret_cast<const Vec3 *>(this))[i]; }

    Mat3 &transpose() {
        Mat3 tmp(*this);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                (*this)[i][j] = tmp[j][i];
            }
        }
        return *this;
    }

    Vec3 operator*(const Vec3 &v) const {
        Vec3 res{};
        for (int i = 0; i < 3; ++i) {
            res[i] = dot((*this)[i], v);
        }
        return res;
    }

    Mat3 &operator*=(const Mat3 &m) {
        Mat3 A(*this);
        Mat3 B(m);
        B.transpose();
        for (int i = 0; i < 3; i++) {
            (*this)[i] = A * B[i];
        }
        return this->transpose();
    }

    Mat3 operator*(const Mat3 &m) const { return Mat3(*this) *= m; }

    Mat3 &operator+=(const Mat3 &m) {
        Mat3 B(m);
        for (int i = 0; i < 3; i++) {
            (*this)[i] += B[i];
        }
        return *this;
    }

    Mat3 operator+(const Mat3 &m) const { return Mat3(*this) += m; }

    Mat3 &operator*=(const float &n) {
        for (int i = 0; i < 3; i++) {
            (*this)[i] *= n;
        }
        return *this;
    }

    Mat3 operator*(const float &n) const { return Mat3(*this) *= n; }
};

Mat3 crossM(Vec3 p);

Vec3 cross(Vec3 p, Vec3 q);

struct Mat2 {
    Vec2 row1, row2;

    Mat2() {}

    Mat2(Vec2 r1, Vec2 r2): row1(r1), row2(r2) {}

    Mat2(float a) {
        row1 = Vec2(a, 0.f);
        row2 = Vec2(0.f, a);
    }

    Mat2(Mat3 m) {
        row1 = Vec2(m[0][0], m[0][1]);
        row2 = Vec2(m[1][0], m[1][1]);
    }

    Vec2 &operator[](int i) const { return ((Vec2 *)this)[i]; }

    Mat2 &transpose() {
        Mat2 tmp(*this);
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                (*this)[i][j] = tmp[j][i];
            }
        }
        return *this;
    }

    Vec2 operator*(const Vec2 &v) const {
        Vec2 *res = new (Vec2);
        for (int i = 0; i < 2; i++) {
            (*res)[i] = dot((*this)[i], v);
        }
        return *res;
    }

    Mat2 &operator*=(const Mat2 &m) {
        Mat2 A(*this), B(m);
        B.transpose();
        for (int i = 0; i < 2; i++) {
            (*this)[i] = A * B[i];
        }
        return this->transpose();
    }

    Mat2 operator*(const Mat2 &m) const { return Mat2(*this) *= m; }
};
