#include "ballistics.hpp"

#include <cstring>

bool getAmmoParams(const char* ammo_name, float& m, float& d, float& l) {
    if (strcmp(ammo_name, "VOG-17") == 0) {
        m = 0.35f;
        d = 0.07f;
        l = 0.0f;
        return true;
    }
    else if (strcmp(ammo_name, "M67") == 0) {
        m = 0.6f;
        d = 0.10f;
        l = 0.0f;
        return true;
    }
    else if (strcmp(ammo_name, "RKG-3") == 0) {
        m = 1.2f;
        d = 0.10f;
        l = 0.0f;
        return true;
    }
    else if (strcmp(ammo_name, "GLIDING-VOG") == 0) {
        m = 0.45f;
        d = 0.10f;
        l = 1.0f;
        return true;
    }
    else if (strcmp(ammo_name, "GLIDING-RKG") == 0) {
        m = 1.4f;
        d = 0.10f;
        l = 1.0f;
        return true;
    }
    else {
        return false;
    }
}

int calcBallistics(DroneParams& dP, float& h, float& D) {
    float m, d, l;

    if (!getAmmoParams(dP.ammo_name, m, d, l)) 
        return UNKNOWN_AMMO;

    float g = 9.81f;
    float a = d * g * m - 2 * d * d * l * dP.attackSpeed;
    float b = -3 * g * m * m + 3 * d * l * m * dP.attackSpeed;
    float c = 6 * m * m * dP.zd;
    float p = -(b * b) / (3 * a * a);
    float q = (2 * b * b * b) / (27 * a * a * a) + (c / a);
    float phi = acos(3 * q / (2 * p) * sqrt(-3 / p));
    float t = 2 * sqrt(-p / 3) * cos((phi + 4 * M_PI) / 3) - b / (3 * a);

    if (!(t > 0)) 
        return T_MUST_BE_POSITIVE;

    h = dP.attackSpeed * t -
        pow(t, 2) * d * dP.attackSpeed / (2 * m) +
        pow(t, 3) * (6 * d * g * l * m - 6 * d * d * (l * l - 1) * dP.attackSpeed) / (36 * m * m) +
        pow(t, 4) * (-6 * d * d * g * l * (1 + l * l + pow(l, 4)) * m + 3 * pow(d, 3) * l * l * (1 + l * l) * dP.attackSpeed + 6 * pow(d, 3) * pow(l, 4) * (1 + l * l) * dP.attackSpeed)  / (36 * pow((1 + l * l), 2) * pow(m, 3)) +
        pow(t, 5) * (3 * pow(d, 3) * g * pow(l, 3) * m - 3 * pow(d, 4) * l * l * (1 + l*l) * dP.attackSpeed) / (36 * (1 + l * l) * pow(m, 4));

    if (!(h > 0)) 
        return H_MUST_BE_POSITIVE;

    D = sqrt((dP.targetX - dP.xd) * (dP.targetX - dP.xd) + (dP.targetY - dP.yd) * (dP.targetY - dP.yd));

    if (!(D > 0))
        return D_MUST_BE_POSITIVE;

    return 0;
}

