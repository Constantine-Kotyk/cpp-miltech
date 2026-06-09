#pragma once

#include <string>

#include "dto/Coord.h"

class IConfigLoader {
public:
	virtual Coord getStartPos() = 0;
	virtual float getAltitude() = 0;
	virtual float getInitialDir() = 0;
	virtual float getAttackSpeed() = 0;
	virtual float getAccelerationPath() = 0;
	virtual std::string getAmmoName() = 0;
	virtual float getArrayTimeStep() = 0;
	virtual float getSimTimeStep() = 0;
	virtual float getHitRadius() = 0;
	virtual float getAngularSpeed() = 0;
	virtual float getTurnThreshold() = 0;
    virtual bool getIsConfigLoaded() = 0;
    virtual ~IConfigLoader() {}
};
