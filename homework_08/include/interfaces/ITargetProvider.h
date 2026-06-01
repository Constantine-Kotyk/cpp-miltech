#pragma once

#include "dto/Coord.h"

class ITargetProvider {
public:
    virtual int getTargetCount() = 0;
    virtual Coord getTargetPosition(int tnum, float curTime, float dt, float arrayTimeStep) = 0;
    virtual ~ITargetProvider() {}
};

