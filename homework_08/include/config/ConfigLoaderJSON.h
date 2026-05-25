#pragma once

#include <string>

#include "dto/Coord.h"
#include "dto/DroneParams.h"
#include "interfaces/IConfigLoader.h"

class ConfigLoaderJSON : public IConfigLoader {
    DroneConfig config;
    bool configLoaded{false};
public:
    ConfigLoaderJSON(const std::string name) {
        configLoaded = readDroneConfig(name);
    }
    bool readDroneConfig(const std::string fileName);
	Coord getStartPos() override { return config.startPos; }
	float getAltitude() override { return config.altitude; }
	float getInitialDir() override { return config.initialDir; }
	float getAttackSpeed() override { return config.attackSpeed; }
	float getAccelerationPath() override { return config.accelerationPath; }
	std::string getAmmoName() override { return config.ammoName; }
	float getArrayTimeStep() override { return config.arrayTimeStep; }
	float getSimTimeStep() override { return config.simTimeStep; }
	float getHitRadius() override { return config.hitRadius; }
	float getAngularSpeed() override { return config.angularSpeed; }
	float getTurnThreshold() override { return config.turnThreshold; }
    bool getIsConfigLoaded() override { return configLoaded; }
};
