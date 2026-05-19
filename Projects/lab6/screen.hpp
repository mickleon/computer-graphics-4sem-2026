#pragma once

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#include "figure.hpp"
#include "matrix.hpp"
#include "raylib.h"
#include "transform.hpp"

struct Screen {
    // расстояния от сторон прямоугольника до границ окна
    const float left = 30, right = 140, top = 20, bottom = 50;
    float minX = left, maxX;
    float minY = top, maxY;
    float Wx, Wy;
    float Wcx = left, Wcy;
    float rectCentX, rectCentY;
    float rectAspect;

    Mat4 T = Mat4(1.f);         // матрица, в которой накапливаются все преобразования
                                // первоначально - единичная матрица
    Vec3 S, P, u;               // координаты точки наблюдения
                                // точки, в которую направлен вектор наблюдения
                                // вектора направления вверх
    float dist;                 // вспомогательная переменная - расстояние между S и P
    float fovy, aspect;         // угол обзора и соотношение сторон окна наблюдения
    float fovyWork, aspectWork; // рабочие переменные для fovy и aspect
    float near, far;            // расстояния до окна наблюдения и до горизонта
    float n, f;                 // рабочие переменные для near и far
    float l, r, t, b;           // рабочие вспомогательные переменные
                                // для значений координат левой, правой,
                                // нижней и верхней координаты в СКН

    enum ProjType { Ortho, Frustum, Perspective } pType = Ortho; // тип трехмерной проекции

    void rectCalc(float screenWidth, float screenHeigth) {
        Wx = screenWidth - left - right;
        Wy = screenHeigth - top - bottom;

        Wcy = maxY;

        rectAspect = Wx / Wy;

        rectCentX = Wx / 2.0f + left;
        rectCentY = Wy / 2.0f + top;

        maxX = screenWidth - right;
        maxY = screenHeigth - bottom;
    }

    void initWorkPars() { // инициализация рабочих параметров камеры
        n = near;
        f = far;
        fovyWork = fovy;
        aspectWork = aspect;
        float Vy = 2 * near * tan(fovy / 2);
        float Vx = aspect * Vy;
        l = -Vx / 2;
        r = Vx / 2;
        b = -Vy / 2;
        t = Vy / 2;
        dist = length(P - S);
        T = lookAt(S, P, u);
    }

    std::vector<ssu::Model> getModels(const char *fileName) {
        std::vector<ssu::Model> models;
        std::ifstream in;
        in.open(fileName);

        if (in.is_open()) {
            Mat4 M = Mat4(1.f);            // матрица для получения модельной матрицы
            Mat4 initM;                    // матрица для начального преобразования каждого рисунка
            std::vector<Mat4> transforms;  // стек матриц преобразований
            std::vector<ssu::Path> figure; // список ломаных очередного рисунка
            float thickness = 2;           // толщина со значением по умолчанию 2
            int r, g, b;                   // составляющие цвета
            r = g = b = 0;                 // значение составляющих цвета по умолчанию (черный)
            std::string cmd;               // строка для считывания имени команды

            std::string str; // строка, в которую считываем строки файла
            getline(in, str);

            while (in) {
                if ((str.find_first_not_of(" \t\r\n") != std::string::npos) && (str[0] != '#')) {
                    std::stringstream s(str);
                    s >> cmd;
                    if (cmd == "camera") {        // положение камеры
                        s >> S.x >> S.y >> S.z;   // координаты точки наблюдения
                        s >> P.x >> P.y >> P.z;   // точка, в которую направлен вектор наблюдения
                        s >> u.x >> u.y >> u.z;   // вектор направления вверх
                    } else if (cmd == "screen") { // положение окна наблюдения
                        s >> fovyWork >> aspect >> near >> far; // параметры команды
                        fovy = fovyWork / 180.f * PI; // перевод угла из градусов в радианты
                    } else if (cmd == "color") {      // цвет линии
                        s >> r >> g >> b;             // считываем три составляющие цвета
                    } else if (cmd == "thickness") {  // толщина линии
                        s >> thickness;               // считываем значение толщины
                    } else if (cmd == "path") {       // набор точек
                        std::vector<Vec3> vertices;   // список точек ломаной
                        int N;                        // количество точек
                        s >> N;
                        std::string str1; // дополнительная строка для чтения из файла
                        while (N > 0) {   // пока не все точки считали
                            getline(in, str1);
                            if ((str1.find_first_not_of(" \t\r\n") != std::string::npos) &&
                                (str1[0] != '#')) {
                                // прочитанная строка не пуста и не комментарий
                                // значит в ней пара координат
                                float x, y, z; // переменные для считывания
                                std::stringstream s1(
                                    str1
                                ); // еще один строковый поток из строки str1
                                s1 >> x >> y >> z;
                                vertices.push_back(Vec3(x, y, z)); // добавляем точку в список
                                N--; // уменьшаем счетчик после успешного считывания точки
                            }
                        }

                        figure.push_back(
                            ssu::Path(
                                vertices,
                                Color{
                                    static_cast<uint8_t>(r), static_cast<uint8_t>(g),
                                    static_cast<uint8_t>(b), 255
                                },
                                thickness
                            )
                        );
                    } else if (cmd == "model") {               // начало описания нового рисунка
                        float mVcx, mVcy, mVcz, mVx, mVy, mVz; // параметры команды model
                        s >> mVcx >> mVcy >> mVcz >> mVx >> mVy >>
                            mVz; // считываем значения переменных
                        float S = mVx / mVy < 1 ? 2.f / mVy : 2.f / mVx;
                        // сдвиг точки привязки из начала координат в нужную позицию
                        // после которого проводим масштабирование
                        initM = scale(S, S, S) * translate(-mVcx, -mVcy, -mVcz);
                        figure.clear();
                    } else if (cmd == "figure") { // формирование новой модели
                        models.push_back(ssu::Model(figure, M * initM));
                    } else if (cmd == "translate") {   // перенос
                        float Tx, Ty, Tz;              // параметры преобразования переноса
                        s >> Tx >> Ty >> Tz;           // считываем параметры
                        M = translate(Tx, Ty, Tz) * M; // добавляем перенос к общему преобразованию
                    } else if (cmd == "scale") {       // масштабирование
                        float S;                       // параметр масштабирования
                        s >> S;                        // считываем параметр
                        M = scale(S, S, S) * M; // добавляем масштабирование к общему преобразованию
                    } else if (cmd == "rotate") { // поворот
                        float theta;              // угол поворота в градусах
                        float nx, ny, nz;         // координаты направляющего вектора оси вращения
                        s >> theta >> nx >> ny >> nz; // считываем параметры
                        // добавляем вращение к общему преобразованию
                        M = rotate(theta / 180.f * PI, Vec3(nx, ny, nz)) * M;
                    } else if (cmd == "pushTransform") { // сохранение матрицы в стек
                        transforms.push_back(M);         // сохраняем матрицу в стек
                    } else if (cmd == "popTransform") {  // откат к матрице из стека
                        M = transforms.back();           // получаем верхний элемент стека
                        transforms.pop_back();           // выкидываем матрицу из стека
                    }
                }
                getline(in, str);
            }
        }
        return models;
    }

    void keyPressHandle() {
        // Сброс преобразований
        if (IsKeyPressed(KEY_ESCAPE)) {
            initWorkPars();
        }

        // Прямоугольная проекция
        if (IsKeyDown(KEY_ONE)) {
            pType = Ortho;
        }
        // Перспективная проекция
        if (IsKeyDown(KEY_TWO)) {
            pType = Frustum;
        }
        if (IsKeyPressed(KEY_THREE)) {
            pType = Perspective;
        }

        // Вперед
        if (IsKeyDown(KEY_W)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                T = lookAt(Vec3(0, 0, -0.1), Vec3(0, 0, -0.2), Vec3(0, 1, 0)) * T;
            } else {
                T = lookAt(Vec3(0, 0, -1), Vec3(0, 0, -2), Vec3(0, 1, 0)) * T;
            }
        }
        // Назад
        if (IsKeyDown(KEY_S)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                T = lookAt(Vec3(0, 0, 0.1), Vec3(0, 0, 0), Vec3(0, 1, 0)) * T;
            } else {
                T = lookAt(Vec3(0, 0, 1), Vec3(0, 0, 0), Vec3(0, 1, 0)) * T;
            }
        }
        // Влево
        if (IsKeyDown(KEY_A)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                T = lookAt(Vec3(-0.1, 0, 0), Vec3(-0.1, 0, -0.1), Vec3(0, 1, 0)) * T;
            } else {
                T = lookAt(Vec3(-1, 0, 0), Vec3(-1, 0, -1), Vec3(0, 1, 0)) * T;
            }
        }
        // Вправо
        if (IsKeyDown(KEY_D)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                T = lookAt(Vec3(0.1, 0, 0), Vec3(0.1, 0, -0.1), Vec3(0, 1, 0)) * T;
            } else {
                T = lookAt(Vec3(1, 0, 0), Vec3(1, 0, -1), Vec3(0, 1, 0)) * T;
            }
        }
        // поворот относительно оси Oz
        if (IsKeyDown(KEY_R)) {
            Vec3 u_new = Mat3(rotate(-0.01, Vec3(0, 0, 1))) * Vec3(0, 1, 0);
            T = lookAt(Vec3(0, 0, 0), Vec3(0, 0, -1), u_new) * T;
        }
        if (IsKeyDown(KEY_Y)) {
            Vec3 u_new = Mat3(rotate(0.01, Vec3(0, 0, 1))) * Vec3(0, 1, 0);
            T = lookAt(Vec3(0, 0, 0), Vec3(0, 0, -1), u_new) * T;
        }

        // Поворот вверх
        if (IsKeyDown(KEY_T)) {
            // вращение камеры относительно оси вокруг точки P отстоящей от начала координат в
            // отрицательном направлении оси Oz на расстояние dist
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                Mat4 M = rotateP(
                    0.01, Vec3(1, 0, 0), Vec3(0, 0, -dist)
                );                                    // матрица вращения относительно точки P
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0); // вращение направления вверх
                Vec3 S_new = normalize(M * Vec4(0, 0, 0, 1)); // вращение начала координат
                // переход к СКН в которой начало координат в новой точке, а направление
                // наблюдения - в точку P
                T = lookAt(S_new, Vec3(0, 0, -dist), u_new) * T;
            } else {
                // разворот камеры по вертикали
                Mat4 M = rotate(0.01, Vec3(1, 0, 0)); // матрица вращения относительно Ox
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0); // вращение направления вверх
                Vec3 P_new = normalize(
                    M * Vec4(0, 0, -1, 1)
                ); // вращение точки, в которую смотрит наблюдатель
                T = lookAt(Vec3(0, 0, 0), P_new, u_new) * T;
            }
        }
        // Поворот вниз
        if (IsKeyDown(KEY_G)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                Mat4 M = rotateP(
                    -0.01, Vec3(1, 0, 0), Vec3(0, 0, -dist)
                );                                    // матрица вращения относительно точки P
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0); // вращение направления вверх
                Vec3 S_new = normalize(M * Vec4(0, 0, 0, 1)); // вращение начала координат
                T = lookAt(S_new, Vec3(0, 0, -dist), u_new) * T;
            } else {
                // разворот камеры по вертикали
                Mat4 M = rotate(-0.01, Vec3(1, 0, 0)); // матрица вращения относительно Ox
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0);  // вращение направления вверх
                Vec3 P_new = normalize(
                    M * Vec4(0, 0, -1, 1)
                ); // вращение точки, в которую смотрит наблюдатель
                T = lookAt(Vec3(0, 0, 0), P_new, u_new) * T;
            }
        }
        // Поворот вправо
        if (IsKeyDown(KEY_H)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                Mat4 M = rotateP(
                    0.01, Vec3(0, 1, 0), Vec3(0, 0, -dist)
                );                                    // матрица вращения относительно точки P
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0); // вращение направления вверх
                Vec3 S_new = normalize(M * Vec4(0, 0, 0, 1)); // вращение начала координат
                T = lookAt(S_new, Vec3(0, 0, -dist), u_new) * T;
            } else {
                // разворот камеры по горизонтали
                Mat4 M = rotate(0.01, Vec3(0, 1, 0)); // матрица вращения относительно Oy
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0); // вращение направления вверх
                Vec3 P_new = normalize(
                    M * Vec4(0, 0, -1, 1)
                ); // вращение точки, в которую смотрит наблюдатель
                T = lookAt(Vec3(0, 0, 0), P_new, u_new) * T;
            }
        }

        // Поворот влево
        if (IsKeyDown(KEY_F)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                Mat4 M = rotateP(
                    -0.01, Vec3(0, 1, 0), Vec3(0, 0, -dist)
                );                                    // матрица вращения относительно точки P
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0); // вращение направления вверх
                Vec3 S_new = normalize(M * Vec4(0, 0, 0, 1)); // вращение начала координат
                T = lookAt(S_new, Vec3(0, 0, -dist), u_new) * T;
            } else {
                // разворот камеры по горизонтали
                Mat4 M = rotate(-0.01, Vec3(0, 1, 0)); // матрица вращения относительно Oy
                Vec3 u_new = Mat3(M) * Vec3(0, 1, 0);  // вращение направления вверх
                Vec3 P_new = normalize(
                    M * Vec4(0, 0, -1, 1)
                ); // вращение точки, в которую смотрит наблюдатель
                T = lookAt(Vec3(0, 0, 0), P_new, u_new) * T;
            }
        }

        Vector2 mousePosition = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && minX <= mousePosition.x &&
            mousePosition.x <= maxX && minY <= mousePosition.y && mousePosition.y <= maxY) {
            float sensitivity = 0.007f;
            Vector2 mouseDelta = GetMouseDelta();
            float dx = -mouseDelta.x * sensitivity;
            float dy = -mouseDelta.y * sensitivity;
            Vec3 u_new = T[1];
            Vec3 forward = Vec3(-1, -1, -1) * T[2];

            if (fabs(dx) > 0.0f) {
                Mat3 M = Mat3(rotate(dx, u));
                forward = M * forward;
                u_new = M * u_new;
            }

            Vec3 right = norm(cross(forward, u_new));

            if (fabs(dy) > 0.0f) {
                Mat3 M = rotate(dy, right);
                forward = M * forward;
                u_new = M * u_new;
            }
            Vec3 t = Vec3(T[0][3], T[1][3], T[2][3]);
            Vec3 S = Vec3(-1, -1, -1) * (Mat3(T).transpose() * t);
            T = lookAt(S, S + forward, u_new);
        }

        // уменьшение/увеличение t
        if (IsKeyDown(KEY_I)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                t -= 1;
            } else {
                t += 1;
            }
        }
        // уменьшение/увеличение l
        if (IsKeyDown(KEY_J)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                l -= 1;
            } else {
                l += 1;
            }
        }
        // уменьшение/увеличение b
        if (IsKeyDown(KEY_K)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                b -= 1;
            } else {
                b += 1;
            }
        }
        // уменьшение/увеличение r
        if (IsKeyDown(KEY_L)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                r -= 1;
            } else {
                r += 1;
            }
        }

        // уменьшение/увеличение n
        if (IsKeyDown(KEY_U)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                if (n > 0.1 + 0.2) {
                    n -= 0.2;
                }
            } else if (n < f - 0.1 - 0.2) {
                n += 0.2;
            }
        }
        // уменьшение/увеличение f
        if (IsKeyDown(KEY_O)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                if (f > n + 0.1 + 0.2) {
                    f -= 0.2;
                }
            } else {
                f += 0.2;
            }
        }
        // уменьшение/увеличение dist
        if (IsKeyDown(KEY_B)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                if (dist > 0.1 + 0.2) {
                    dist -= 0.2;
                }
            } else {
                dist += 0.2;
            }
        }
        // уменьшение/увеличение fovyWork
        if (IsKeyDown(KEY_Z)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                if (fovyWork > 0.3 + 0.05) {
                    fovyWork -= 0.05;
                }
            } else if (fovyWork < 3 - 0.05) {
                fovyWork += 0.05;
            }
        }
        // уменьшение/увеличение aspectWork
        if (IsKeyDown(KEY_X)) {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                if (aspectWork > 0.01 + 0.05) {
                    aspectWork -= 0.05;
                }
            } else {
                aspectWork += 0.05;
            }
        }
    }
};
