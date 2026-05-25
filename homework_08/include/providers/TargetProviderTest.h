#pragma once

#include "dto/Coord.h"
#include "interfaces/ITargetProvider.h"

class TargetProviderTest : public ITargetProvider { // нерухомі цілі
    int targetsCount{5};
    [[maybe_unused]]int targetsTimeSteps{1};
public:
    TargetProviderTest() {}
    int getTargetCount() override { return targetsCount; }
    Coord getTargetPosition(int tnum, [[maybe_unused]]float curTime, [[maybe_unused]]float dt, [[maybe_unused]]float arrayTimeStep) override {
        switch (tnum) {
            case 0: return {300.0f, 300.0f};
            case 1: return {336.8f, 295.0f};
            case 2: return {290.0f, 310.0f};
            case 3: return {334.0f, 305.0f};
            case 4: return {295.0f, 290.0f};
        }
        return {0.0f, 0.0f};
    }
};
