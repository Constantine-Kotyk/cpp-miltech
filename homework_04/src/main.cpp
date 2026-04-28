#include <fstream>
#include <iostream>
#include <cmath>

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

    std::ifstream in(argv[1]);
    if (!in.is_open()) {
        std::cerr << "failed to read data from file: " << argv[1] << "\n";
        return 1;
    }

    odometry_data data[2];
    in >> data[0].timestamp_ms >> data[0].fl_ticks >> data[0].fr_ticks >> data[0].bl_ticks >> data[0].br_ticks;

    double x = 0.0, y = 0.0, theta = 0.0; 

    while (!in.eof()) {
        in >> data[1].timestamp_ms >> data[1].fl_ticks >> data[1].fr_ticks >> data[1].bl_ticks >> data[1].br_ticks;

        if(in.fail()) {
            break; // Exit the loop if nothing was read (end of file)
        }

        // Calculate the change in ticks for each wheel
        long d_fl = data[1].fl_ticks - data[0].fl_ticks;
        long d_fr = data[1].fr_ticks - data[0].fr_ticks;
        long d_bl = data[1].bl_ticks - data[0].bl_ticks;
        long d_br = data[1].br_ticks - data[0].br_ticks;

        // Calculate the average distance traveled by the left and right wheels
        double d_left = (d_fl + d_bl) / 2.0;
        double d_right = (d_fr + d_br) / 2.0;

        // Convert ticks to distance (in meters)
        double distance_per_tick = (2 * M_PI * WHEEL_RADIUS_M) / static_cast<double>(TICKS_PER_REVOLUTION);

        double dL = d_left * distance_per_tick;
        double dR = d_right * distance_per_tick;

        // Calculate the average distance traveled
        double d = (dL + dR) / 2.0; 

        // Calculate the change in orientation (theta)
        double dtheta = (dR - dL) / WHEELBASE_M;

        // Update position and orientation
        x += d * cos(theta + dtheta / 2.0);
        y += d * sin(theta + dtheta / 2.0);
        theta += dtheta;

        // Output the timestamp, x, y, and theta
        std::cout << data[1].timestamp_ms << " " << x << " " << y << " " << theta << std::endl;

        data[0] = data[1]; // Update the previous data for the next iteration
    }

    in.close();

    return 0;
}

