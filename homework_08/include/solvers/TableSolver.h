#pragma once

#include "interfaces/IBallisticSolver.h"

class TableSolver : public IBallisticSolver {
public:
    void calcBalisticTask(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd, float& bombDist, float& bombTime) override;
};