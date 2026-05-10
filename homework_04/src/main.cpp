#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

constexpr int TICKS_PER_REVOLUTION = 1024;
constexpr double WHEEL_RADIUS_M = 0.3;
constexpr double WHEELBASE_M = 1.0;

struct RawData{
    long timestamp;
    long fl_ticks;
    long fr_ticks;
    long bl_ticks;
    long br_ticks;
};

bool readData(const char* filePath, std::vector<RawData>& data){
    std::ifstream inputFile(filePath);

    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open " << filePath << " file" << std::endl;
        return false;
    }
    
    data.clear();

    long timestamp, fl_ticks, fr_ticks, bl_ticks, br_ticks;

    while (inputFile >> timestamp >> fl_ticks >> fr_ticks >> bl_ticks >> br_ticks) {
        data.emplace_back(timestamp, fl_ticks, fr_ticks, bl_ticks, br_ticks);    
    }

    return true;
}

int main(int argc, char** argv) {
    // The program expects exactly one argument: a path to telemetry samples.
    
    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";
        return 1;
    }

    std::vector<RawData> data;

    if (!readData(argv[1], data)) {
        return 1;
    }

    double d_fl, d_fr, d_bl, d_br;
    double d_left, d_right;
    double distance_per_tick, dL, dR, d, dtheta, x{0.0}, y{0.0}, theta{0.0};

    for (unsigned long i = 1; i < data.size(); i += 1) {
        d_fl = data[i].fl_ticks - data[i-1].fl_ticks;
        d_fr = data[i].fr_ticks - data[i-1].fr_ticks;
        d_bl = data[i].bl_ticks - data[i-1].bl_ticks;
        d_br = data[i].br_ticks - data[i-1].br_ticks;

        d_left  = (d_fl + d_bl) / 2;
        d_right = (d_fr + d_br) / 2;

        distance_per_tick = 2 * M_PI * WHEEL_RADIUS_M / TICKS_PER_REVOLUTION;

        dL = d_left  * distance_per_tick;
        dR = d_right * distance_per_tick;

        d = (dL + dR) / 2;
        dtheta = (dR - dL) / WHEELBASE_M;

        x += d * std::cos(theta + dtheta / 2);
        y += d * sin(theta + dtheta / 2);
        theta += dtheta;

        std::cout << data[i].timestamp << " " << x << " " << y << " " << theta << std::endl;
    }

    return 0;
}
