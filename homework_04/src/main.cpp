#include <fstream>
#include <iostream>

const int TICKS_PER_REVOLUTION    = 1024;
const double WHEEL_RADIUS_M       = 0.3;
const double WHEELBASE_M          = 1.0;

struct odometry_data {
    long timestamp_ms, fl_ticks, fr_ticks, bl_ticks, br_ticks;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";
        return 1;
    }

    // Input:  text file with 5 whitespace-separated numbers per line:
    //         timestamp_ms fl_ticks fr_ticks bl_ticks br_ticks
    std::ifstream in(argv[1]);
    if (!in.is_open()) {
        std::cerr << "failed to read data from file: " << argv[1] << "\n";
        return 1;
    }

    odometry_data data[2];
    in >> data[0].timestamp_ms >> data[0].fl_ticks >> data[0].fr_ticks >> data[0].bl_ticks >> data[0].br_ticks;

    std::cout << data[0].timestamp_ms << data[0].fl_ticks << data[0].fr_ticks << data[0].bl_ticks << data[0].br_ticks;

    // TODO: implement wheel odometry for a 4-wheel differential-drive UGV.
    // Output: same tabular format on stdout, starting from the second sample:
    //         timestamp_ms x y theta

    in.close();

    return 0;
}

