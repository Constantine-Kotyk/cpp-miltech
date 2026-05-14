#pragma once

#include <cmath>

#define UNSUCCESSFUL 1
#define SUCCESS 0
#define UNKNOWN_AMMO -1
#define T_MUST_BE_POSITIVE -2
#define H_MUST_BE_POSITIVE -3
#define D_MUST_BE_POSITIVE -4
#define FILE_OPEN_FAILED -5
#define FILE_FIELD_COUNT_MISMATCH -6
#define FILE_FIELD_PARSE_FAILED -7

struct DroneParams {
    float xd, yd, zd; // coordinates of the drone
    float targetX, targetY; // coordinates of the target
    float attackSpeed; // speed of the attack
    float accelerationPath; // additional distance due to acceleration
    char ammo_name[11]; // name of the ammunition
};

struct Solution {
    float fireX, fireY;
    float midX, midY;
    bool hasMidPoint{false};
};

bool getAmmoParams(const char* ammo_name, float& m, float& d, float& l);
int calcBallistics(DroneParams& dP, Solution& solution);
int read_data(char* path, DroneParams& dP);