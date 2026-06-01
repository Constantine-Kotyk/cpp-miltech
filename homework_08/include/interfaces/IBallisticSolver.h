#pragma once

class IBallisticSolver {
public:
    virtual void calcBalisticTask(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd, float& bombDist, float& bombTime) = 0;
    virtual ~IBallisticSolver() {}
};

