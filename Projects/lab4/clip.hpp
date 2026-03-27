#pragma once

#include <utility>

#include "matrix.hpp"

unsigned int codeKS(const Vec2 &P, float minX, float minY, float maxX, float maxY) {
    unsigned int code = 0;

    if (P.x < minX) {
        code += 1;
    } else if (P.x > maxX) {
        code += 2;
    }

    if (P.y < minY) {
        code += 4;
    } else if (P.y > maxY) {
        code += 8;
    }

    return code;
}

bool clip(Vec2 &A, Vec2 &B, float minX, float minY, float maxX, float maxY) {
    unsigned int codeA = codeKS(A, minX, minY, maxX, maxY);
    unsigned int codeB = codeKS(B, minX, minY, maxX, maxY);

    while (codeA | codeB) {
        if (codeA & codeB) {
            return false; // отрезок полностью невидим
        }

        // Один из концо отрезка за областью видимости
        if (codeA == 0) { // Если A видима, а B невидима
            // Меняем местами названия переменных
            std::swap(A, B);
            std::swap(codeA, codeB);
        }
        if (codeA & 1) { // точка A левее области видимости
            A.y = A.y + (B.y - A.y) * (minX - A.x) / (B.x - A.x);
            A.x = minX;
        } else if (codeA & 2) { // точка A правее области видимости
            A.y = A.y + (B.y - A.y) * (maxX - A.x) / (B.x - A.x);
            A.x = maxX;
        } else if (codeA & 4) { // точка A ниже области видимости
            A.x = A.x + (B.x - A.x) * (minY - A.y) / (B.y - A.y);
            A.y = minY;
        } else { // точка A выше области видимости
            A.x = A.x + (B.x - A.x) * (maxY - A.y) / (B.y - A.y);
            A.y = maxY;
        }
        // обновляем код области для точки A
        codeA = codeKS(A, minX, minY, maxX, maxY);
    }

    return true;
}
