#pragma once

#include "interfaces/IConfigLoader.h"
#include "interfaces/IBallisticSolver.h"
#include "interfaces/ITargetProvider.h"
#include "dto/Coord.h"
#include "dto/AmmoParams.h"

enum DronePhase {STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING};

struct SimStep {
	Coord      pos;          	 // позиція дрона
	float      direction;    	 // напрямок (рад)
	DronePhase state;        	 // стан автомата (0-4)
	int        targetIdx;    	 // індекс поточної цілі
	Coord      dropPoint;    	 // точка скиду (куди летить дрон)
	Coord      aimPoint;     	 // куди впаде бомба (якщо скинути зараз)
	Coord      predictedTarget;  // прогнозована позиція цілі
};

struct SelectedTarget {
    Coord fp, pp;
    float time, dir;
    int num;
};

class MissionProcessor {
    const unsigned maxSteps;
    IConfigLoader* config{nullptr};
    IBallisticSolver* solver{nullptr};
    ITargetProvider* targets{nullptr};
    int ammoParamsCount;
    AmmoParams* ammo{nullptr};
    SimStep* steps;
    int cTarget{0};

    bool executeConditions();
    bool getBombParams(float& m, float& d, float& l);
public:
    MissionProcessor() : maxSteps(10000) {
        steps = new SimStep[maxSteps];
    }
    friend class MissionBuilder;
    unsigned getMaxSteps() { return maxSteps; }
    void changeConfig(IConfigLoader* c) { config = c; }
    void changeSolver(IBallisticSolver* s) { solver = s; }
    void changeProvider(ITargetProvider* t) { targets = t; }
    AmmoParams* readAmmoParams(const std::string fileName, int& ammoCount);
    void calcFirePosition(Coord dp, Coord tp, float bombDist, float addDist, Coord& fp, Coord& mp, bool& midF);
    float calcTotalTime(Coord dp, Coord tp, float curDir, float angleThreshold, float angularSpeed, float curSpeed, float maxSpeed, float accPath, bool stopf);
    float normalizeAngle(float a);
    void resetTargetIterator() { cTarget = 0; }
    bool hasNextTarget() { return cTarget != targets->getTargetCount(); }
    void nextTargetCalc(SelectedTarget* selTarget, const float& curTime, const float& bombDist, const float& bombTime, const Coord& dp, const float& dir, const float& curSpeed, const DronePhase& curPhase, const int& curTarget);
    int execute();
    void writeResult(const std::string fileName, const int& stepsCount);
    ~MissionProcessor() {
        if (ammo) delete[] ammo;
        delete steps;
    }
};

class MissionBuilder {
    MissionProcessor* m;
public:
    MissionBuilder() { m = new MissionProcessor(); }
    MissionBuilder& setAmmoParams(const std::string fileName) { m->ammo = m->readAmmoParams(fileName, m->ammoParamsCount); return *this; }
    MissionBuilder& setDroneConfig(IConfigLoader* c) { m->config = c; return *this; }
    MissionBuilder& setBallisticSolver(IBallisticSolver* s) { m->solver = s; return *this; }
    MissionBuilder& setTargetProvider(ITargetProvider* t) { m->targets = t; return *this; }
    MissionProcessor* build() { return m; }
};