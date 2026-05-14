#include "ballistics.hpp"

#include <iostream>


int main(int argc, char** argv) {

    // The executable expects input file path as an argument
    if (argc != 2) {
        std::cerr << "usage: balistics_check <input_path>" << std::endl ;
        return UNSUCCESSFUL;
    }

    DroneParams dP;

    int result;
    result = read_data(argv[1], dP);
    if (result < SUCCESS) {
        switch (result) {
            case FILE_OPEN_FAILED:
                std::cerr << "Failed to open input file!" << std::endl;
                break;
            case FILE_FIELD_COUNT_MISMATCH:
                std::cerr << "Field count in input file does not match expected!" << std::endl;
                break;
            case FILE_FIELD_PARSE_FAILED:
                std::cerr << "Failed to parse fields in input file!" << std::endl;
                break;
        }
        return UNSUCCESSFUL;
    }

    Solution solution{};

    result = calcBallistics(dP, solution);
    if (result < SUCCESS) {
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
        return UNSUCCESSFUL;
    }

    if (solution.hasMidPoint) {
        std::cout << "Fire at: (" << solution.fireX << ", " << solution.fireY << ")" << std::endl;
        std::cout << "Midpoint: (" << solution.midX << ", " << solution.midY << ")" << std::endl;
    } else {
        std::cout << "Fire at: (" << solution.fireX << ", " << solution.fireY << ")" << std::endl;
    }

    return SUCCESS;
}