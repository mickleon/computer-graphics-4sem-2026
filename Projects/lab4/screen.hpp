#pragma once

#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>

#include "figure.hpp"
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

    Mat3 initT;         // матрица начального преобразования
    Mat3 T = Mat3(1.f); // матрица, в которой накапливаются все преобразования
                        // первоначально - единичная матрица

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

    std::vector<ssu::ModelFig> getModels(const char *fileName) {
        std::vector<ssu::ModelFig> models;
        std::ifstream in;
        in.open(fileName);

        if (in.is_open()) {
            Mat3 M = Mat3(1.f);            // матрица для получения модельной матрицы
            Mat3 initM;                    // матрица для начального преобразования каждого рисунка
            std::vector<Mat3> transforms;  // стек матриц преобразований
            std::vector<ssu::Path> figure; // список ломаных очередного рисунка
            float thickness = 2;           // толщина со значением по умолчанию 2
            int r, g, b;                   // составляющие цвета
            r = g = b = 0;                 // значение составляющих цвета по умолчанию (черный)
            std::string cmd;               // строка для считывания имени команды

            float Vx, Vy;

            std::string str; // строка, в которую считываем строки файла
            getline(in, str);

            while (in) {
                if ((str.find_first_not_of(" \t\r\n") != std::string::npos) && (str[0] != '#')) {
                    std::stringstream s(str);
                    s >> cmd;
                    if (cmd == "frame") { // размеры изображения
                        s >> Vx >> Vy;
                        float aspectFig = Vx / Vy; // обновление соотношения сторон
                        // смещение центра рисунка с началом координат
                        Mat3 T1 = translate(-Vx / 2, -Vy / 2);
                        // масштабирование остается прежним, меняется только привязка
                        // коэффициент увеличения при сохранении исходного соотношения сторон
                        float S = aspectFig < rectAspect ? Wy / Vy : Wx / Vx;
                        Mat3 S1 = scale(S, -S);
                        // сдвиг точки привязки из начала координат в нужную позицию
                        Mat3 T2 = translate(Wx / 2 + Wcx, Wcy - Wy / 2);
                        // В initT совмещаем эти три преобразования (справа налево)
                        initT = T2 * (S1 * T1);
                        T = initT;
                    } else if (cmd == "color") {     // цвет линии
                        s >> r >> g >> b;            // считываем три составляющие цвета
                    } else if (cmd == "thickness") { // толщина линии
                        s >> thickness;              // считываем значение толщины
                    } else if (cmd == "path") {      // набор точек
                        std::vector<Vec2> vertices;  // список точек ломаной
                        int N;                       // количество точек
                        s >> N;
                        std::string str1; // дополнительная строка для чтения из файла
                        while (N > 0) {   // пока не все точки считали
                            getline(in, str1);
                            if ((str1.find_first_not_of(" \t\r\n") != std::string::npos) &&
                                (str1[0] != '#')) {
                                // прочитанная строка не пуста и не комментарий
                                // значит в ней пара координат
                                float x, y; // переменные для считывания
                                std::stringstream s1(
                                    str1
                                ); // еще один строковый поток из строки str1
                                s1 >> x >> y;
                                vertices.push_back(Vec2(x, y)); // добавляем точку в список
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
                    } else if (cmd == "model") {         // начало описания нового рисунка
                        float mVcx, mVcy, mVx, mVy;      // параметры команды model
                        s >> mVcx >> mVcy >> mVx >> mVy; // считываем значения переменных
                        float S = mVx / mVy < 1 ? 2.f / mVy : 2.f / mVx;
                        // сдвиг точки привязки из начала координат в нужную позицию
                        // после которого проводим масштабирование
                        initM = scale(S) * translate(-mVcx, -mVcy);
                        figure.clear();
                    } else if (cmd == "figure") { // формирование новой модели
                        models.push_back(ssu::ModelFig(figure, M * initM, Vx, Vy));
                    } else if (cmd == "translate") { // перенос
                        float Tx, Ty;                // параметры преобразования переноса
                        s >> Tx >> Ty;               // считываем параметры
                        M = translate(Tx, Ty) * M;   // добавляем перенос к общему преобразованию
                    } else if (cmd == "scale") {     // масштабирование
                        float S;                     // параметр масштабирования
                        s >> S;                      // считываем параметр
                        M = scale(S) * M; // добавляем масштабирование к общему преобразованию
                    } else if (cmd == "rotate") { // поворот
                        float theta;              // угол поворота в градусах
                        s >> theta;               // считываем параметр
                        M = rotate(-theta / 180.f * PI) *
                            M; // добавляем поворот к общему преобразованию
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
        // Поворот по часовой стрелке
        if (IsKeyDown(KEY_Q)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = rotate(0.01f) * T;
            T = translate(rectCentX, rectCentY) * T;
        }
        // Поворот против часовой стрелки
        if (IsKeyDown(KEY_E)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = rotate(-0.01f) * T;
            T = translate(rectCentX, rectCentY) * T;
        }

        // Поворот по часовой стрелке (ускорение)
        if (IsKeyDown(KEY_Y)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = rotate(0.05f) * T;
            T = translate(rectCentX, rectCentY) * T;
        }
        // Поворот против часовой стрелки (ускорени
        if (IsKeyDown(KEY_R)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = rotate(-0.05f) * T;
            T = translate(rectCentX, rectCentY) * T;
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

        // Сдвиг вверх (ускорение)
        if (IsKeyDown(KEY_T)) {
            T = translate(0, -10) * T;
        } // Сдвиг вниз (ускорение)
        if (IsKeyDown(KEY_G)) {
            T = translate(0, 10) * T;
        } // Сдвиг влево (ускорение)
        if (IsKeyDown(KEY_F)) {
            T = translate(-10, 0) * T;
        } // Сдвиг вправо (ускорение)
        if (IsKeyDown(KEY_H)) {
            T = translate(10, 0) * T;
        }

        // Увеличение
        if (IsKeyDown(KEY_Z)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = scale(1.1) * T;
            T = translate(rectCentX, rectCentY) * T;
        } // Уменьшение
        if (IsKeyDown(KEY_X)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = scale(1 / 1.1) * T;
            T = translate(rectCentX, rectCentY) * T;
        }

        // Отразить по горизонтали
        if (IsKeyPressed(KEY_U)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = mirrorX() * T;
            T = translate(rectCentX, rectCentY) * T;
        } // Сжатие по горизонтали
        if (IsKeyPressed(KEY_J)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = mirrorY() * T;
            T = translate(rectCentX, rectCentY) * T;
        }

        // Растяжение по горизонтали
        if (IsKeyDown(KEY_I)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = scale(1.1, 1) * T;
            T = translate(rectCentX, rectCentY) * T;
        } // Сжатие по горизонтали
        if (IsKeyDown(KEY_K)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = scale(1 / 1.1, 1) * T;
            T = translate(rectCentX, rectCentY) * T;
        }

        // Растяжение по вертикали
        if (IsKeyDown(KEY_O)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = scale(1, 1.1) * T;
            T = translate(rectCentX, rectCentY) * T;
        } // Сжатие по вертикали
        if (IsKeyDown(KEY_L)) {
            T = translate(-rectCentX, -rectCentY) * T;
            T = scale(1, 1 / 1.1) * T;
            T = translate(rectCentX, rectCentY) * T;
        }

        // Сброс преобразований
        if (IsKeyPressed(KEY_ESCAPE)) {
            T = initT;
        }
    }
};
