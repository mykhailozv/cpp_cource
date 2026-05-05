#include <iostream>
#include <fstream>
#include <vector>

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
    // TODO: implement wheel odometry for a 4-wheel differential-drive UGV.
    //
    // Model parameters:
    //   ticks_per_revolution = 1024
    //   wheel_radius_m       = 0.3
    //   wheelbase_m          = 1.0
    //
    // Input: a text file with 5 whitespace-separated values per line:
    //         timestamp_ms fl_ticks fr_ticks bl_ticks br_ticks
    // Output: a table on stdout, starting from the second sample:
    //         timestamp_ms x y theta

    return 0;
}
