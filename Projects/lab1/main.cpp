#include <raylib.h>

Font InitFont(const char *fontPath, float fontSize) {
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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(600, 480, "Lab1");
    SetWindowMinSize(200, 150);
    SetTargetFPS(60);

    Font font = InitFont("Assets/Fonts/AdwaitaSans-Regular.ttf", 20.0f);

    while (!WindowShouldClose()) {
        const float Wx = static_cast<float>(GetScreenWidth());
        const float Wy = static_cast<float>(GetScreenHeight());

        const float dx = 100.0f;
        const float dy = 100.0f;

        const Vector2 PATH[] = {
            {Wx / 3, 0}, {Wx, Wy / 3}, {Wx / 3 * 2, Wy}, {0, Wy / 3 * 2}, {Wx / 3, 0},
        };

        BeginDrawing();
        ClearBackground(Color{127, 255, 212, 255});
        DrawTextEx(font, "Надпись на форме", {40, 100}, 20.0f, 0.0f, BLACK);
        DrawLineEx({0, 0}, {Wx, Wy}, 6, RED);
        DrawLineEx({90, 50}, {Wx - dx, Wy - dy}, 10, BLUE);

        for (int i = 0; i < 5; ++i) {
            DrawLineEx(PATH[i], PATH[i + 1], 5, DARKGREEN);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
