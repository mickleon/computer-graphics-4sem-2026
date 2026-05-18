#include <iostream>
#include <vector>

#include <nfd.h>
#include <raygui.h>
#include <raylib.h>

#include "clip.hpp"
#include "figure.hpp"
#include "matrix.hpp"
#include "screen.hpp"

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
    if (NFD_Init() != NFD_OKAY) {
        std::cerr << "ERROR: " << NFD_GetError() << std::endl;
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(30 + 140 + 20 * 50, 20 + 50 + 10 * 50, "Lab6");
    SetWindowMinSize(170, 70);
    SetTargetFPS(60);
    SetExitKey(0);

    Font font = initFont("Assets/Fonts/AdwaitaSans-Regular.ttf", 20.f);
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 2);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x00000000FF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0x00000000FF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, 0x00000000FF);
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0xE1E1E1FF);

    std::vector<ssu::Model> models;
    Screen s;

    while (!WindowShouldClose()) {
        float screenWidth = static_cast<float>(GetScreenWidth());
        float screenHeigth = static_cast<float>(GetScreenHeight());
        s.rectCalc(screenWidth, screenHeigth);

        s.keyPressHandle();

        BeginDrawing();
        ClearBackground(Color{129, 211, 248, 255});

        DrawRectangleLines(
            s.left, s.top, screenWidth - s.left - s.right, screenHeigth - s.top - s.bottom, BLACK
        );

        Mat4 proj; // матрица перехода в пространство отсечения
        switch (s.pType) {
        case Screen::ProjType::Ortho: // прямоугольная проекция
            proj = ortho(s.l, s.r, s.b, s.t, -s.n, -s.f);
            break;
        case Screen::ProjType::Frustum: // перспективная проекция с Frustum
            proj = frustum(s.l, s.r, s.b, s.t, s.n, s.f);
            break;
        case Screen::ProjType::Perspective: // перспективная проекция с Perspective
            proj = perspective(s.fovyWork, s.aspectWork, s.n, s.f);
            break;
        }
        // матрица кадрирования
        Mat3 cdr = cadrRL(Vec2(-1.f, -1.f), Vec2(2.f, 2.f), Vec2(s.Wcx, s.Wcy), Vec2(s.Wx, s.Wy));
        Mat4 C = proj * s.T; // матрица перехода от мировых координат в пространство отсечения

        for (const auto &model : models) {
            Mat4 TM = C * model.modelM;
            for (const auto &lines : model.figure) {
                // начальная точка первого отрезка в трехмерных евклидовых координатах
                Vec2 start3D = normalize(TM * Vec4(lines.vertices[0], 1.0));
                // начальная точка первого отрезка в координатах экрана
                Vec2 start = normalize(cdr * Vec3(Vec2(start3D), 1.f));
                for (const auto &line : lines.vertices) {
                    // конечная точка отрезка в трехмерных евклидовых координатах
                    Vec3 end3D = normalize(TM * Vec4(line, 1.0));
                    // конечная точка отрезка в координатах экрана
                    Vec2 end = normalize(cdr * Vec3(Vec2(end3D), 1.f));
                    Vec2 tmpEnd = end;
                    if (clip(start, end, s.minX, s.minY, s.maxX, s.maxY)) {
                        DrawLineEx(
                            {start.x, start.y}, {end.x, end.y}, lines.thickness, lines.color
                        );
                    }
                    start = tmpEnd;
                }
            }
        }

        // Нажата кнопка "Открыть"
        if (GuiButton({screenWidth - 120, 20, 100, 30}, "Открыть")) {
            nfdchar_t *outPath;
            nfdfilteritem_t filterItem[2] = {{"Text files", "txt"}, {"All files", "*"}};
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, nullptr);

            if (result == NFD_OKAY) {
                models = s.getModels(outPath);
                NFD_FreePath(outPath);
                s.initWorkPars();
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
