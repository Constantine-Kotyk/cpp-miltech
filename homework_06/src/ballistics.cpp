#include "ballistics.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>


const int EXPECTED_FIELD_COUNT = 8;
const int MAX_LINE_LENGTH = 256;


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

int calcBallistics(DroneParams& dP, Solution& solution) {
    float m, d, l, h, D;

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

    if (h + dP.accelerationPath > D) {
        dP.xd = dP.targetX - (dP.targetX - dP.xd) * (h + dP.accelerationPath) / D;
        dP.yd = dP.targetY - (dP.targetY - dP.yd) * (h + dP.accelerationPath) / D;

        solution.midX = dP.xd;
        solution.midY = dP.yd;
        solution.hasMidPoint = true;

        D = sqrt((dP.targetX - dP.xd) * (dP.targetX - dP.xd) + (dP.targetY - dP.yd) * (dP.targetY - dP.yd));

        if (!(D > 0)) 
            return D_MUST_BE_POSITIVE;
    }

    double ratio = (D - h) / D;
    solution.fireX = dP.xd + (dP.targetX - dP.xd) * ratio;
    solution.fireY = dP.yd + (dP.targetY - dP.yd) * ratio;

    return SUCCESS;
}


int split_line(char line[], char* fields[], int max_fields) {
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

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' &&
               *cursor != '\r') {
            ++cursor;
        }
    }

    return count;
}

float parse_float(const char* text, bool* success) {
    char* end = nullptr;
    float value = std::strtof(text, &end);

    if (end == text)
        *success = false;
    
    return value;
}

int read_data(char* path, DroneParams& dP) {
    std::ifstream input{path};
    if (!input) 
        return FILE_OPEN_FAILED;
    

    char line[MAX_LINE_LENGTH];
    input.getline(line, MAX_LINE_LENGTH);
    input.close();

    char* fields[EXPECTED_FIELD_COUNT] = {};
    const int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT);
    
    if (field_count != EXPECTED_FIELD_COUNT) 
        return FILE_FIELD_COUNT_MISMATCH;

    bool field_success = true;
    dP.xd = parse_float(fields[0], &field_success);
    dP.yd = parse_float(fields[1], &field_success);
    dP.zd = parse_float(fields[2], &field_success);
    dP.targetX = parse_float(fields[3], &field_success);
    dP.targetY = parse_float(fields[4], &field_success);
    dP.attackSpeed = parse_float(fields[5], &field_success);
    dP.accelerationPath = parse_float(fields[6], &field_success);
    strncpy(dP.ammo_name, fields[7], sizeof(dP.ammo_name) - 1);
    dP.ammo_name[sizeof(dP.ammo_name) - 1] = '\0';
    if (!field_success) 
        return FILE_FIELD_PARSE_FAILED;
    
    return SUCCESS;
}
