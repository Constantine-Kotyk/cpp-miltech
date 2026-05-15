#include "ballistics.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>

const int EXPECTED_FIELD_COUNT = 8;
const int MAX_LINE_LENGTH = 256;

auto get_ammo_params(const char* ammo_name, float& a_m, float& a_d, float& a_l) -> bool
{
  if (strcmp(ammo_name, "VOG-17") == 0) {
    a_m = 0.35f;  // NOLINT
    a_d = 0.07f;  // NOLINT
    a_l = 0.0f;   // NOLINT
    return true;
  }
  else if (strcmp(ammo_name, "M67") == 0) {
    a_m = 0.6f;   // NOLINT
    a_d = 0.10f;  // NOLINT
    a_l = 0.0f;   // NOLINT
    return true;
  }
  else if (strcmp(ammo_name, "RKG-3") == 0) {
    a_m = 1.2f;   // NOLINT
    a_d = 0.10f;  // NOLINT
    a_l = 0.0f;   // NOLINT
    return true;
  }
  else if (strcmp(ammo_name, "GLIDING-VOG") == 0) {
    a_m = 0.45f;  // NOLINT
    a_d = 0.10f;  // NOLINT
    a_l = 1.0f;   // NOLINT
    return true;
  }
  else if (strcmp(ammo_name, "GLIDING-RKG") == 0) {
    a_m = 1.4f;   // NOLINT
    a_d = 0.10f;  // NOLINT
    a_l = 1.0f;   // NOLINT
    return true;
  }
  else {
    return false;
  }
}

auto calc_ballistics(DroneParams& d_params, Solution& solution) -> int
{
  float a_m, a_d, a_l, h_p, D;

  if (!get_ammo_params(d_params.ammo_name_, a_m, a_d, a_l)) {
    return kUnknownAmmo;
  }

  float g_c = 9.81F;
  float a_c = a_d * g_c * a_m - 2 * a_d * a_d * a_l * d_params.attack_speed_;
  float b_c = -3 * g_c * a_m * a_m + 3 * a_d * a_l * a_m * d_params.attack_speed_;
  float c_c = 6 * a_m * a_m * d_params.z_;
  float p_c = -(b_c * b_c) / (3 * a_c * a_c);
  float q_c = (2 * b_c * b_c * b_c) / (27 * a_c * a_c * a_c) + (c_c / a_c);
  float phi = acos(3 * q_c / (2 * p_c) * std::sqrt(-3 / p_c));
  float time = 2 * std::sqrt(-p_c / 3) * cos((phi + 4 * M_PI) / 3) - b_c / (3 * a_c);

  if (!(time > 0)) {
    return kTMustBePositive;
  }

  h_p = d_params.attack_speed_ * time - pow(time, 2) * a_d * d_params.attack_speed_ / (2 * a_m) +
        pow(time, 3) * (6 * a_d * g_c * a_l * a_m - 6 * a_d * a_d * (a_l * a_l - 1) * d_params.attack_speed_) / (36 * a_m * a_m) +
        pow(time, 4) *
          (-6 * a_d * a_d * g_c * a_l * (1 + a_l * a_l + pow(a_l, 4)) * a_m +
           3 * pow(a_d, 3) * a_l * a_l * (1 + a_l * a_l) * d_params.attack_speed_ +
           6 * pow(a_d, 3) * pow(a_l, 4) * (1 + a_l * a_l) * d_params.attack_speed_) /
          (36 * pow((1 + a_l * a_l), 2) * pow(a_m, 3)) +
        pow(time, 5) *
          (3 * pow(a_d, 3) * g_c * pow(a_l, 3) * a_m - 3 * pow(a_d, 4) * a_l * a_l * (1 + a_l * a_l) * d_params.attack_speed_) /
          (36 * (1 + a_l * a_l) * pow(a_m, 4));

  if (!(h_p > 0)) {
    return kHMustBePositive;
  }

  D = std::sqrt((d_params.target_x_ - d_params.x_) * (d_params.target_x_ - d_params.x_) +
                (d_params.target_y_ - d_params.y_) * (d_params.target_y_ - d_params.y_));

  if (!(D > 0)) {
    return kDMustBePositive;
  }

  if (h_p + d_params.acceleration_path_ > D) {
    d_params.x_ = d_params.target_x_ - (d_params.target_x_ - d_params.x_) * (h_p + d_params.acceleration_path_) / D;
    d_params.y_ = d_params.target_y_ - (d_params.target_y_ - d_params.y_) * (h_p + d_params.acceleration_path_) / D;

    solution.mid_x_ = d_params.x_;
    solution.mid_y_ = d_params.y_;
    solution.has_mid_point_ = true;

    D = std::sqrt((d_params.target_x_ - d_params.x_) * (d_params.target_x_ - d_params.x_) +
                  (d_params.target_y_ - d_params.y_) * (d_params.target_y_ - d_params.y_));

    if (!(D > 0)) {
      return kDMustBePositive;
    }
  }
  double ratio = (D - h_p) / D;
  solution.fire_x_ = d_params.x_ + (d_params.target_x_ - d_params.x_) * ratio;
  solution.fire_y_ = d_params.y_ + (d_params.target_y_ - d_params.y_) * ratio;

  return kSuccess;
}

int split_line(char line[], char* fields[], int max_fields)
{
  int count = 0;
  char* cursor = line;

  while (*cursor != '\0' && count < max_fields) {
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
      *cursor = '\0';
      ++cursor;
    }

    if (*cursor == '\0') {
      break;
    }

    fields[count] = cursor;
    ++count;

    while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\r') {
      ++cursor;
    }
  }

  return count;
}

float parse_float(const char* text, bool* success)
{
  char* end = nullptr;
  float value = std::strtof(text, &end);

  if (end == text) {
    *success = false;
  }

  return value;
}

auto read_data(char* path, DroneParams& dParams) -> int
{
  std::ifstream input{path};
  if (!input) {
    return kFileOpenFailed;
  }

  char line[MAX_LINE_LENGTH];
  input.getline(line, MAX_LINE_LENGTH);
  input.close();

  char* fields[EXPECTED_FIELD_COUNT] = {};  // NOLINT
  const int kFieldCount = split_line(line, fields, EXPECTED_FIELD_COUNT);

  if (kFieldCount != EXPECTED_FIELD_COUNT) {
    return kFileFieldCountMismatch;
  }

  bool field_success = true;
  dParams.x_ = parse_float(fields[0], &field_success);
  dParams.y_ = parse_float(fields[1], &field_success);
  dParams.z_ = parse_float(fields[2], &field_success);
  dParams.target_x_ = parse_float(fields[3], &field_success);
  dParams.target_y_ = parse_float(fields[4], &field_success);
  dParams.attack_speed_ = parse_float(fields[5], &field_success);
  dParams.acceleration_path_ = parse_float(fields[6], &field_success);
  // NOLINTNEXTLINE - old method for string copy because of C-style string in struct
  strncpy(dParams.ammo_name_, fields[7], sizeof(dParams.ammo_name_) - 1);
  dParams.ammo_name_[sizeof(dParams.ammo_name_) - 1] = '\0';
  if (!field_success) {
    return kFileFieldParseFailed;
  }
  return kSuccess;
}
