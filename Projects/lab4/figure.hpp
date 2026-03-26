#pragma once

#include <raylib.h>
#include <vector>

#include "matrix.hpp"

namespace ssu {
    struct Path {
        std::vector<Vec2> vertices;
        Color color;
        float thickness;

        Path(std::vector<Vec2> vertices, Color color, float thickness):
            vertices(vertices),
            color(color),
            thickness(thickness) {}
    };

    struct ModelFig {
        std::vector<Path> figure;
        Mat3 modelM;
        float Vx, Vy;

        ModelFig(const std::vector<Path> &p, Mat3 mat, float x, float y):
            figure(p),
            modelM(mat),
            Vx(x),
            Vy(y) {}
    };
} // namespace ssu
