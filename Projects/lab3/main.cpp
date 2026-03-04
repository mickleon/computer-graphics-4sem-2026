#include <iostream>
#include <nfd.h>
#include <raygui.h>
#include <raylib.h>

#include "figure.hpp"
#include "matrix.hpp"
#include "transform.hpp"

Font initFont(const char *fontPath, float fontSize) {
    int charsCount = 0;
    const char *const CHARS =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" \
        "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~" \
        "абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    int *chars = LoadCodepoints(CHARS, &charsCount);

    Font font = LoadFontEx(fontPath, fontSize, chars, charsCount);
    UnloadCodepoints(chars);
    return font;
}

int main() {
    NFD_Init();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(400, 400, "Lab3");
    SetWindowMinSize(100, 100);
    SetTargetFPS(60);

    Font font = initFont("Assets/Fonts/AdwaitaSans-Regular.ttf", 20.f);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 2);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x00000000FF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0x00000000FF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, 0x00000000FF);
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0xE1E1E1FF);

    ssu::Figure figure;
    Mat3 initT;         // матрица начального преобразования
    Mat3 T = Mat3(1.f); // матрица, в которой накапливаются все преобразования
                        // первоначально - единичная матрица

    while (!WindowShouldClose()) {
        const float Wx = static_cast<float>(GetScreenWidth());
        const float Wy = static_cast<float>(GetScreenHeight());

        const float Wcx = Wx / 2.0f;
        const float Wcy = Wy / 2.0f;

        const float windowAspect = Wx / Wy;

        // Поворот против часовой стрелки
        if (IsKeyDown(KEY_Q)) {
            T = translate(-Wcx, -Wcy) * T; // перенос начала координат в (Wcx, Wcy)
            T = rotate(-0.01f) * T;        // поворот на -0.01 радиан относительно нового центра
            T = translate(Wcx, Wcy) * T;   // перенос начала координат обратно
        }

        // Сдвиг вверх
        if (IsKeyDown(KEY_W)) {
            T = translate(0, -1) * T;
        } // Сдвиг вниз
        if (IsKeyDown(KEY_S)) {
            T = translate(0, 1) * T;
        } // Сдвиг влево
        if (IsKeyDown(KEY_A)) {
            T = translate(-1, 0) * T;
        } // Сдвиг вправо
        if (IsKeyDown(KEY_D)) {
            T = translate(1, 0) * T;
        }

        // Сброс преобразований
        if (IsKeyPressed(KEY_C)) {
            T = initT;
        }

        BeginDrawing();
        ClearBackground(Color{129, 211, 248, 255});

        for (const auto &lines : figure.paths) {
            Vec2 start = normalize(T * Vec3(lines.vertices[0], 1));
            for (const auto &line : lines.vertices) {
                const Vec2 end = normalize(T * Vec3(line, 1));
                DrawLineEx({start.x, start.y}, {end.x, end.y}, lines.thickness, lines.color);
                start = end;
            }
        }

        // Нажата кнопка "Открыть"
        if (GuiButton({Wx - 120, 20, 100, 30}, "Открыть")) {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[2] = {{"Text files", "txt"}, {"All files", "*"}};
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, nullptr);

            if (result == NFD_OKAY) {
                figure = ssu::Figure(outPath);
                NFD_FreePath(outPath);

                float figureAspect = figure.Vx / figure.Vy;
                float S = figureAspect < windowAspect ? Wy / figure.Vy : Wx / figure.Vx;
                const float Ty = S * figure.Vy;
                initT = translate(0, Ty) * scale(S, -S);
                T = initT;
            } else if (result == NFD_CANCEL) {
                std::cerr << "INFO: NFD: user pressed cancel" << std::endl;
            } else {
                std::cerr << "ERROR: " << NFD_GetError() << std::endl;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    NFD_Quit();
    return 0;
}
