#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <raylib.h>
#include <sstream>
#include <vector>

#include "matrix.hpp"

inline bool isIgnorableLine(const std::string &line) {
    return line.find_first_not_of(" \t\r\n") == std::string::npos || line.front() == '#';
}

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

    struct Figure {
        std::vector<Path> paths;
        float Vx, Vy;
        Figure() = default;

        Figure(const std::vector<Path> &p, float x, float y): paths(p), Vx(x), Vy(y) {}

        Figure(const char *filePath) {
            std::ifstream in(filePath);
            int r, g, b;
            float thickness;

            std::string line;
            while (in) {
                getline(in, line);

                if (isIgnorableLine(line)) {
                    continue;
                }

                std::stringstream s(line);

                std::string cmd;
                s >> cmd;
                if (cmd == "frame") {
                    s >> this->Vx >> this->Vy;
                    std::cout << this->Vx << ' ' << this->Vy << std::endl;
                } else if (cmd == "color") {
                    s >> r >> g >> b;
                } else if (cmd == "thickness") {
                    s >> thickness;
                } else if (cmd == "path") {
                    std::vector<Vec2> vertices;
                    int n;
                    s >> n;
                    std::string str1;
                    while (n > 0) {
                        getline(in, str1);

                        if (isIgnorableLine(str1)) {
                            continue;
                        }

                        float x, y;
                        std::stringstream s1(str1);
                        s1 >> x >> y;
                        vertices.push_back(Vec2(x, y));

                        --n;
                    }
                    this->paths.push_back(
                        ssu::Path(
                            vertices,
                            Color{
                                static_cast<uint8_t>(r), static_cast<uint8_t>(g),
                                static_cast<uint8_t>(b), 255
                            },
                            thickness
                        )
                    );
                }
            }
        }
    };
} // namespace ssu
