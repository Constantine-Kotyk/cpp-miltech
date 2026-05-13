#pragma once

#include <cmath>

#define UNKNOWN_AMMO -1
#define T_MUST_BE_POSITIVE -2
#define H_MUST_BE_POSITIVE -3
#define D_MUST_BE_POSITIVE -4

struct DroneParams {
    float xd, yd, zd; // coordinates of the drone
    float targetX, targetY; // coordinates of the target
    float attackSpeed; // speed of the attack
    float accelerationPath; // additional distance due to acceleration
    char ammo_name[11]; // name of the ammunition
};

bool getAmmoParams(const char* ammo_name, float& m, float& d, float& l);
int calcBallistics(DroneParams& dP, float& h, float& D);