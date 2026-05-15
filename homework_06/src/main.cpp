#include "ballistics.hpp"

#include <iostream>
// #include <span>

auto main(int argc, char** argv) -> int
{
  // The executable expects input file path as an argument
  if (argc != 2) {
    std::cerr << "usage: balistics_check <input_path>\n";
    return kFailed;
  }

  // auto args = std::span(argv, static_cast<size_t>(argc));  // NOLINT does not work

  DroneParams d_p{};

  int result{0};
  result = read_data(argv[1], d_p);  // NOLINT - argv[1] is the input file path
  if (result < kSuccess) {
    switch (result) {
      case kFileOpenFailed:
        std::cerr << "Failed to open input file!\n";
        break;
      case kFileFieldCountMismatch:
        std::cerr << "Field count in input file does not match expected!\n";
        break;
      case kFileFieldParseFailed:
        std::cerr << "Failed to parse fields in input file!\n";
        break;
      default:
        std::cerr << "Unknown error while reading input file!\n";
    }
    return kFailed;
  }

  Solution solution{};

  result = calc_ballistics(d_p, solution);
  if (result < kSuccess) {
    std::cerr << "Ballistics calculation failed: ";

    switch (result) {
      case kUnknownAmmo:
        std::cerr << "Unknown ammunition!\n";
        break;
      case kTMustBePositive:
        std::cerr << "t must be positive!\n";
        break;
      case kHMustBePositive:
        std::cerr << "h must be positive!\n";
        break;
      case kDMustBePositive:
        std::cerr << "D must be positive!\n";
        break;
      default:
        std::cerr << "Unknown error!\n";
    }
    return kFailed;
  }

  if (solution.has_mid_point_) {
    std::cout << "Fire at: (" << solution.fire_x_ << ", " << solution.fire_y_ << ")\n";
    std::cout << "Midpoint: (" << solution.mid_x_ << ", " << solution.mid_y_ << ")\n";
  }
  else {
    std::cout << "Fire at: (" << solution.fire_x_ << ", " << solution.fire_y_ << ")\n";
  }

  return kSuccess;
}