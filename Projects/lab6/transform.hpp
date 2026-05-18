#pragma once

#include <cmath>
#include <math.h>

#include "matrix.hpp"

inline Mat3 translate(float Tx, float Ty) {
    Mat3 res = Mat3(1.f);
    res[0][2] = Tx;
    res[1][2] = Ty;
    return res;
}

inline Mat4 translate(float Tx, float Ty, float Tz) {
    Mat4 res = Mat4(1.f);
    res[0][3] = Tx;
    res[1][3] = Ty;
    res[2][3] = Tz;
    return res;
}

inline Mat3 scale(float Sx, float Sy) {
    Mat3 res = Mat3(1.f);
    res[0][0] = Sx;
    res[1][1] = Sy;
    return res;
}

inline Mat4 scale(float Sx, float Sy, float Sz) {
    Mat4 res = Mat4(1.f);
    res[0][0] = Sx;
    res[1][1] = Sy;
    res[2][2] = Sz;
    return res;
}

inline Mat3 scale(float S) {
    return scale(S, S);
}

inline Mat3 mirrorX() {
    return scale(-1, 1);
}

inline Mat3 mirrorY() {
    return scale(1, -1);
}

inline Mat3 rotate(float theta) {
    Mat3 res = Mat3(1.f);
    res[0][0] = static_cast<float>(cos(theta));
    res[1][1] = res[0][0];
    res[0][1] = static_cast<float>(sin(theta));
    res[1][0] = -res[0][1];
    return res;
}

// преобразование Родригеса: вращение относительно оси, проходящей через начало координат, заданной
// вектором n
inline Mat4 rotate(float theta, Vec3 n) {
    Vec3 n_norm = norm(n);

    Mat3 K = crossM(n_norm);
    Mat3 K2 = K * K;

    Mat3 R3 = Mat3(1.f) + K * sin(theta) + K2 * (1.f - cos(theta));

    // clang-format off
    return Mat4(
        Vec4(R3[0][0], R3[0][1], R3[0][2], 0.f),
        Vec4(R3[1][0], R3[1][1], R3[1][2], 0.f),
        Vec4(R3[2][0], R3[2][1], R3[2][2], 0.f),
        Vec4(0.f, 0.f, 0.f, 1.f)
    );
    // clang-format on
}

inline Mat4 rotateP(float theta, Vec3 n, Vec3 P) {
    return translate(P.x, P.y, P.z) * (rotate(theta, n) * translate(-P.x, -P.y, -P.z));
}

// реализует переход в систему координат наблюдателя
// S - точка наблюдения
// точка P - на которую направлен вектор наблюдения
// вектор u - указывает условное направление вверх
inline Mat4 lookAt(Vec3 S, Vec3 P, Vec3 u) {
    // clang-format off
    Mat4 T = Mat4(
    Vec4(1.f, 0.f, 0.f, -S.x),
    Vec4(0.f, 1.f, 0.f, -S.y), 
    Vec4(0.f, 0.f, 1.f, -S.z), 
    Vec4(0.f, 0.f, 0.f, 1.f)
    );
    // clang-format on

    Vec3 e3 = norm(S - P);
    Vec3 e1 = norm(cross(u, e3));
    Vec3 e2 = norm(cross(e3, e1));

    // clang-format off
    Mat4 R = Mat4(
        Vec4(e1, 0.f),
        Vec4(e2, 0.f),
        Vec4(e3, 0.f),
        Vec4(0.f, 0.f, 0.f, 1.f)
    );
    // clang-format on
    return R * T;
}

inline Mat4 ortho(float l, float r, float b, float t, float zn, float zf) {
    return Mat4(
        Vec4(2.f / (r - l), 0.f, 0.f, -(r + l) / (r - l)),
        Vec4(0.f, 2.f / (t - b), 0.f, -(t + b) / (t - b)),
        Vec4(0.f, 0.f, -2 / (zf - zn), -(zf + zn) / (zf - zn)), Vec4(0.f, 0.f, 0.f, 1.f)
    );
}

inline Mat4 frustum(float l, float r, float b, float t, float n, float f) {
    return Mat4(
        Vec4(2.f * n / (r - l), 0.f, (r + l) / (r - l), 0.f),
        Vec4(0.f, 2.f * n / (t - b), (t + b) / (t - b), 0.f),
        Vec4(0.f, 0.f, -(f + n) / (f - n), -2.f * f * n / (f - n)), Vec4(0.f, 0.f, -1.f, 0.f)
    );
}

inline Mat4 perspective(float fovy, float aspect, float n, float f) {
    return Mat4(
        Vec4(1.f / (aspect * tan(fovy / 2.0f)), 0.f, 0.f, 0.f),
        Vec4(0.f, 1.f / tan(fovy / 2.0f), 0.f, 0.f),
        Vec4(0.f, 0.f, -(f + n) / (f - n), -2.f * f * n / (f - n)), Vec4(0.f, 0.f, -1.f, 0.f)
    );
}

inline Mat3 cadrRL(Vec2 Vc, Vec2 V, Vec2 Wc, Vec2 W) {
    return translate(Wc.x, Wc.y) * (scale(W.x / V.x, -W.y / V.y) * translate(-Vc.x, -Vc.y));
}
