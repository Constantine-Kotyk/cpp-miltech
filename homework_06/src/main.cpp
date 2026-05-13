#include "ballistics.hpp"

#include <iostream>
#include <fstream>


int main(int argc, char** argv) {

    // The executable expects input file path as an argument
    if (argc != 2) {
        std::cerr << "usage: balistics_check <input_path>\n";
        return 1;
    }

    DroneParams dP;

    std::ifstream in(argv[1]);
    in >> dP.xd >> dP.yd >> dP.zd >> dP.targetX >> dP.targetY >> dP.attackSpeed >> dP.accelerationPath >> dP.ammo_name;
    in.close();

    float h, D;
    int result = calcBallistics(dP, h, D);
    if (result < 0) {
        std::cerr << "Ballistics calculation failed: ";

        switch (result) {
            case UNKNOWN_AMMO:
                std::cerr << "Unknown ammunition!" << std::endl;
                break;
            case T_MUST_BE_POSITIVE:
                std::cerr << "t must be positive!" << std::endl;
                break;
            case H_MUST_BE_POSITIVE:
                std::cerr << "h must be positive!" << std::endl;
                break;
            case D_MUST_BE_POSITIVE:
                std::cerr << "D must be positive!" << std::endl;
                break;
        }

        return 1;
    }

    std::ofstream out("output.txt");

    if (h + dP.accelerationPath > D) {
        dP.xd = dP.targetX - (dP.targetX - dP.xd) * (h + dP.accelerationPath) / D;
        dP.yd = dP.targetY - (dP.targetY - dP.yd) * (h + dP.accelerationPath) / D;

        out << dP.xd << " " << dP.yd << " ";

        D = sqrt((dP.targetX - dP.xd) * (dP.targetX - dP.xd) + (dP.targetY - dP.yd) * (dP.targetY - dP.yd));

        if (!(D > 0)) {
            std::cerr << "D must be positive!" << std::endl;
            return 1;
        }
    }

    double ratio = (D - h) / D;
    double fireX = dP.xd + (dP.targetX - dP.xd) * ratio;
    double fireY = dP.yd + (dP.targetY - dP.yd) * ratio;

    out << fireX << " " << fireY;
    out.close();

    return 0;
}