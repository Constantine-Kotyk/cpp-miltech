#pragma once

#include <string>

#include "dto/Coord.h"
#include "interfaces/IConfigLoader.h"

class ConfigLoaderTest : public IConfigLoader {
public:
    ConfigLoaderTest() {}
	Coord getStartPos() override            { return {50.0f, 250.0f}; }
	float getAltitude() override            { return 100.0f; }
	float getInitialDir() override          { return 0.0f; }
	float getAttackSpeed() override         { return 10.0f; }
	float getAccelerationPath() override    { return 10.0f; }
	std::string getAmmoName() override      { return "VOG-17"; }
	float getArrayTimeStep() override       { return 1.0f; }
	float getSimTimeStep() override         { return 0.1f; }
	float getHitRadius() override           { return 3.0f; }
	float getAngularSpeed() override        { return 1.0f; }
	float getTurnThreshold() override       { return 0.1f; }
    bool getIsConfigLoaded() override       { return true; }
};
