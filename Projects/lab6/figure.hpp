#pragma once

#include <raylib.h>
#include <vector>

#include "matrix.hpp"

namespace ssu {
    struct Path {
        std::vector<Vec3> vertices;
        Color color;
        float thickness;

        Path(std::vector<Vec3> vertices, Color color, float thickness):
            vertices(vertices),
            color(color),
            thickness(thickness) {}
    };

    struct Model {
        std::vector<Path> figure;
        Mat4 modelM;

        Model(const std::vector<Path> &p, Mat4 mat): figure(p), modelM(mat) {}
    };
} // namespace ssu
