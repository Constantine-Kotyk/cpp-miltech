#include <cmath>
#include "solvers/AnalyticalSolver.h"

void AnalyticalSolver::calcBalisticTask(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd, float& bombDist, float& bombTime) {

    float g = 9.81;
    float a = d * g * m - 2 * d * d * l * attackSpeed;
    float b = -3 * g * m * m + 3 * d * l * m * attackSpeed;
    float c = 6 * m * m * zd;
    float p = -(b * b) / (3 * a * a);
    if (p < 0) {
        float q = (2 * b * b * b) / (27 * a * a * a) + (c / a);
        float phi = acos(3 * q / (2 * p) * sqrt(-3 / p));
        bombTime = 2 * sqrt(-p / 3) * cos((phi + 4 * M_PI) / 3) - b / (3 * a);
        if (bombTime <= 0)
            bombTime = sqrt(2.0f * zd / g);
    }
    else
        bombTime = sqrt(2.0f * zd / g);

    bombDist = attackSpeed * bombTime -
        pow(bombTime, 2) * d * attackSpeed / (2 * m) +
        pow(bombTime, 3) * (6 * d * g * l * m - 6 * d * d * (l * l - 1) * attackSpeed) / (36 * m * m) +
        pow(bombTime, 4) * (-6 * d * d * g * l * (1 + l * l + pow(l, 4)) * m + 3 * pow(d, 3) * l * l * (1 + l * l) * attackSpeed + 6 * pow(d, 3) * pow(l, 4) * (1 + l * l) * attackSpeed)  / (36 * pow((1 + l * l), 2) * pow(m, 3)) +
        pow(bombTime, 5) * (3 * pow(d, 3) * g * pow(l, 3) * m - 3 * pow(d, 4) * l * l * (1 + l*l) * attackSpeed) / (36 * (1 + l * l) * pow(m, 4));

}
