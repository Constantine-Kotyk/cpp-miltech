#pragma once

#include <fstream>
#include <string>
#include "json.hpp"

#include "interfaces/ITargetProvider.h"

using json = nlohmann::json;

class TargetProviderJSON : public ITargetProvider {
    int targetsCount{0};
    int targetsTimeSteps{0};
    bool targetsLoaded{false};
    json j;
public:
    TargetProviderJSON(const std::string name) {
        std::ifstream f(name);
        if (f.is_open()) {
            f >> j;
            f.close();
            targetsCount = j["targetCount"];
            targetsTimeSteps = j["timeSteps"];
            targetsLoaded = true;
        }
        else
            targetsLoaded = false;
    }
    int getTargetCount() override { return targetsCount; }
    Coord getTargetPosition(int tnum, float curTime, float dt, float arrayTimeStep) override {
        int i = (int)floor(curTime / arrayTimeStep) % targetsTimeSteps;
        int n = (i + 1) % targetsTimeSteps;

        Coord dtc;
        dtc.x = (float)j["targets"][tnum]["positions"][n]["x"] - (float)j["targets"][tnum]["positions"][i]["x"];
        dtc.y = (float)j["targets"][tnum]["positions"][n]["y"] - (float)j["targets"][tnum]["positions"][i]["y"];

        float r = (curTime / arrayTimeStep) - floor(curTime / arrayTimeStep);

        dtc.x = (float)j["targets"][tnum]["positions"][i]["x"] + dtc.x * r + (dtc.x / arrayTimeStep) * dt;
        dtc.y = (float)j["targets"][tnum]["positions"][i]["y"] + dtc.y * r + (dtc.y / arrayTimeStep) * dt;
        return dtc;
    }
};
