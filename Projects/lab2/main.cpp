#include "figure.hpp"
#include "hare.hpp"
#include "turtle.hpp"
#include <raylib.h>

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(400, 400, "Lab2");
    SetWindowMinSize(100, 100);
    SetTargetFPS(60);

    const ssu::Figure *hare = &ssu::figure::HARE;
    const ssu::Figure *turtle = &ssu::figure::TURTLE;
    const ssu::Figure *currentFigure = turtle;
    bool keepAspect = true;

    while (!WindowShouldClose()) {
        const float Wx = static_cast<float>(GetScreenWidth());
        const float Wy = static_cast<float>(GetScreenHeight());
        const float windowAspect = Wx / Wy;

        if (IsKeyPressed(KEY_N)) {
            currentFigure = (currentFigure == hare) ? turtle : hare;
        }
        if (IsKeyPressed(KEY_M)) {
            keepAspect = !keepAspect;
        }

        float Sx, Sy;
        ssu::Figure figure = *currentFigure;
        if (keepAspect) {
            float figureAspect = figure.Vx / figure.Vy;
            Sx = Sy = figureAspect < windowAspect ? Wy / figure.Vy : Wx / figure.Vx;
        } else {
            Sx = Wx / figure.Vx;
            Sy = Wy / figure.Vy;
        }
        const float Ty = Sy * figure.Vy;

        BeginDrawing();
        ClearBackground(Color{127, 255, 212, 255});

        for (size_t i = 0; i < figure.vertices.size(); i += 4) {
            DrawLineEx(
                {Sx * figure.vertices[i], Ty - Sy * figure.vertices[i + 1]},
                {Sx * figure.vertices[i + 2], Ty - Sy * figure.vertices[i + 3]}, 2, BLACK
            );
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
