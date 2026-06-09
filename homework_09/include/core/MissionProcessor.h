#pragma once

#include <map>
#include <memory>
#include <vector>
#include <string>

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

typedef std::map<std::string, AmmoParams> AmmoMap;
typedef std::vector<SimStep> SimSteps;

class MissionProcessor {
    unsigned maxSteps;
    IConfigLoader* config{nullptr};
    std::unique_ptr<IBallisticSolver> solver;
    std::unique_ptr<ITargetProvider> targets;
    AmmoMap ammo;
    SimSteps steps;
    int cTarget{0};

    bool executeConditions();
    bool getBombParams(float& m, float& d, float& l);
public:
    friend class MissionBuilder;
    unsigned getMaxSteps() { return maxSteps; }
    void changeConfig(IConfigLoader* c) { config = c; }
    void changeSolver(std::unique_ptr<IBallisticSolver>& s) { solver = std::move(s); }
    void changeProvider(std::unique_ptr<ITargetProvider>& t) { targets = std::move(t); }
    void readAmmoParams(const std::string fileName);
    void calcFirePosition(Coord dp, Coord tp, float bombDist, float addDist, Coord& fp, Coord& mp, bool& midF);
    float calcTotalTime(Coord dp, Coord tp, float curDir, float angleThreshold, float angularSpeed, float curSpeed, float maxSpeed, float accPath, bool stopf);
    float normalizeAngle(float a);
    void resetTargetIterator() { cTarget = 0; }
    bool hasNextTarget() { return cTarget != targets->getTargetCount(); }
    void nextTargetCalc(SelectedTarget* selTarget, const float& curTime, const float& bombDist, const float& bombTime, const Coord& dp, const float& dir, const float& curSpeed, const DronePhase& curPhase, const int& curTarget);
    int execute();
    void writeResult(const std::string fileName);
};

class MissionBuilder {
    MissionProcessor* m;
public:
    MissionBuilder() { m = new MissionProcessor(); }
    MissionBuilder& setMaxSteps(unsigned s) { m->maxSteps = s; return *this; }
    MissionBuilder& setAmmoParams(const std::string fileName) { m->readAmmoParams(fileName); return *this; }
    MissionBuilder& setDroneConfig(IConfigLoader* c) { m->config = c; return *this; }
    MissionBuilder& setBallisticSolver(std::unique_ptr<IBallisticSolver>& s) { m->solver = std::move(s); return *this; }
    MissionBuilder& setTargetProvider(std::unique_ptr<ITargetProvider>& t) { m->targets = std::move(t); return *this; }
    MissionProcessor* build() { return m; }
};