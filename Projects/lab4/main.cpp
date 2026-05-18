#include <iostream>
#include <thread>
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

    std::atomic<bool> fileDialogRunning = false;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(30 + 140 + 20 * 50, 20 + 50 + 10 * 50, "Lab4");
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

        if (!fileDialogRunning) {
            GuiUnlock();
            s.keyPressHandle();
        } else {
            GuiLock();
        }

        BeginDrawing();
        ClearBackground(Color{129, 211, 248, 255});

        DrawRectangleLines(
            s.left, s.top, screenWidth - s.left - s.right, screenHeigth - s.top - s.bottom, BLACK
        );

        /* for (int y = s.minY; y < s.maxY; y += s.Wy / 20) {
            DrawLine(s.minX, y, s.maxX, y, BLACK);
        }
        for (int x = s.minX; x < s.maxX; x += s.Wx / 40) {
            DrawLine(x, s.minY, x, s.maxY, BLACK);
        } */

        for (const auto &model : models) {
            Mat3 TM = s.T * model.modelM;
            for (const auto &lines : model.figure) {
                Vec2 start = normalize(TM * Vec3(lines.vertices[0], 1));
                for (const auto &line : lines.vertices) {
                    Vec2 end = normalize(TM * Vec3(line, 1));
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

        if (GuiButton({screenWidth - 120, 20, 100, 30}, "Открыть") && !fileDialogRunning) {
            fileDialogRunning = true;
            std::thread([&]() {
                nfdchar_t *outPath;
                nfdfilteritem_t filterItem[2] = {{"Text files", "txt"}, {"All files", "*"}};
                nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, nullptr);

                if (result == NFD_OKAY) {
                    models = s.getModels(outPath);
                    NFD_FreePath(outPath);
                } else if (result == NFD_CANCEL) {
                    std::cerr << "INFO: NFD: user pressed cancel" << std::endl;
                } else {
                    std::cerr << "ERROR: " << NFD_GetError() << std::endl;
                }
                fileDialogRunning = false;
            }).detach();
        }

        EndDrawing();
    }

    CloseWindow();
    NFD_Quit();
    return 0;
}
