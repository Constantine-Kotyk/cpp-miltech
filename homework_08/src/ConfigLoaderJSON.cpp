#include "core/ConfigLoaderJSON.h"
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

bool ConfigLoaderJSON::readDroneConfig(const std::string fileName) {
    std::ifstream f(fileName);
    if (!f.is_open()) 
        return false;
    json j;
    f >> j;
    f.close();

    config.startPos.x       = j["drone"]["position"]["x"];
    config.startPos.y       = j["drone"]["position"]["y"];
    config.altitude         = j["drone"]["altitude"];
    config.initialDir       = j["drone"]["initialDirection"];
    config.attackSpeed      = j["drone"]["attackSpeed"];
    config.accelerationPath = j["drone"]["accelerationPath"];
    config.angularSpeed     = j["drone"]["angularSpeed"];
    config.turnThreshold    = j["drone"]["turnThreshold"];
    config.simTimeStep      = j["simulation"]["timeStep"];
    config.hitRadius        = j["simulation"]["hitRadius"];
    config.arrayTimeStep    = j["targetArrayTimeStep"];
    config.ammoName         = j["ammo"];
    return true;
}
