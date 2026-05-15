#pragma once

#include <cmath>
#include <cstdint>

enum ErrorCode : std::int8_t {
  kFailed = 1,
  kSuccess = 0,
  kUnknownAmmo = -1,
  kTMustBePositive = -2,
  kHMustBePositive = -3,
  kDMustBePositive = -4,
  kFileOpenFailed = -5,
  kFileFieldCountMismatch = -6,
  kFileFieldParseFailed = -7
};

struct DroneParams {
  float x_, y_, z_;
  float target_x_, target_y_;
  float attack_speed_;
  float acceleration_path_;
  char ammo_name_[11];  // NOLINT - null-terminated string for ammo name
};

struct Solution {
  float fire_x_{0.0F}, fire_y_{0.0F};
  float mid_x_{0.0F}, mid_y_{0.0F};
  bool has_mid_point_{false};
};

auto get_ammo_params(const char* ammo_name, float& a_m, float& a_d, float& a_l) -> bool;
auto calc_ballistics(DroneParams& d_params, Solution& solution) -> int;
auto read_data(char* path, DroneParams& d_params) -> int;