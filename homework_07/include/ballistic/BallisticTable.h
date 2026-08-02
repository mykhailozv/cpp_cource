#pragma once

#include <vector>


struct BallisticTable{
    // 5 осей — кожна зі своїм набором вузлів (нерівномірний крок)
    std::vector<float> axisZ0;  // висота
    std::vector<float> axisV0;  // швидкість
    std::vector<float> axisM;   // маса
    std::vector<float> axisD;   // опір
    std::vector<float> axisL;   // підйомна сила
 
    // Результат в кожному вузлі сітки
    struct Result {
        float t;      // час польоту
        float hDist;  // горизонтальна дистанція
    };
 
    // Плоский масив розміром |Z0| * |V0| * |M| * |D| * |L|
    std::vector<Result> data;
 
    size_t index(int iz, int iv, int im, int id, int il) const;
 
    const Result& at(int iz, int iv, int im, int id, int il) const;

    // 5-вимірна лінійна інтерполяція (вкладені lerp)
    Result lookup(float Z0, float V0, float m, float d, float l) const;
 
    // Завантаження з текстового файлу
    bool load(const char* path);
};