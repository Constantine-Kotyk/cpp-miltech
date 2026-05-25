#include "core/MissionProcessor.h"
#include "core/umacro.h"
#include <fstream>
#include <cmath>
#include "json.hpp"

using json = nlohmann::json;

void MissionProcessor::writeResult(const std::string fileName, const int& stepsCount) {
    std::ofstream fout(fileName);
    if (!fout.is_open()) {
        ERROR("Cannot write results file (\"" << fileName << "\")");
        return;
    }

    json out;
    out["totalSteps"] = stepsCount;
    out["steps"] = json::array();
    for (int i = 0; i < stepsCount; i++) {
        json s;
        s["position"]        = {{"x", steps[i].pos.x}, {"y", steps[i].pos.y}};
        s["direction"]       = steps[i].direction;
        s["state"]           = steps[i].state;
        s["targetIndex"]     = steps[i].targetIdx;
        s["dropPoint"]       = {{"x", steps[i].dropPoint.x},
                                {"y", steps[i].dropPoint.y}};
        s["aimPoint"]        = {{"x", steps[i].aimPoint.x},
                                {"y", steps[i].aimPoint.y}};
        s["predictedTarget"] = {{"x", steps[i].predictedTarget.x},
                                {"y", steps[i].predictedTarget.y}};
        out["steps"].push_back(s);
    }
    fout << out.dump(2);  // 2 = відступ для читабельності
}

bool MissionProcessor::executeConditions() {
    if(!ammo) {
        LOG("The execution attempt was unsuccessful: ammo configuration is not defined");
        return false;
    }
    if(!config) {
        LOG("The execution attempt was unsuccessful: drone configuration is not defined");
        return false;
    }
    if(!config->getIsConfigLoaded()) {
        LOG("The execution attempt was unsuccessful: drone configuration is not defined");
        return false;
    }
    if(!targets) {
        LOG("The execution attempt was unsuccessful: TargetProvider is not defined");
        return false;
    }
    if(!solver) {
        LOG("The execution attempt was unsuccessful: Solver is not defined");
        return false;
    }
    return true;
}

int MissionProcessor::execute() {
    if(!executeConditions())
        return -1;

    float m, d, l;
    if (!getBombParams(m, d, l)) {
        ERROR("The execution attempt was unsuccessful: Invalid bomb name");
        return -1;
    }

    float acc = (config->getAttackSpeed() * config->getAttackSpeed()) / (2 * config->getAccelerationPath());

    int step = 0;
    float curTime = 0;
    DronePhase curPhase = STOPPED;
    int curTarget = -1;
    float curSpeed = 0;
    float bombTime;
    float bombDist;

	SelectedTarget* selTarget = new SelectedTarget;

    solver->calcBalisticTask(m, d, l, config->getAttackSpeed(), config->getAltitude(), bombDist, bombTime);

    Coord dp = config->getStartPos();
    float dir = config->getInitialDir();

    while (step < (int)maxSteps) {
    	selTarget->num = 0;
    	selTarget->time = 1000000;
    	selTarget->fp = dp;

    	resetTargetIterator();

    	while (hasNextTarget()) {
    		nextTargetCalc(selTarget, curTime, bombDist, bombTime, dp, dir, curSpeed, curPhase, curTarget);
    	}

    	curTarget = selTarget->num;

    	selTarget->dir = atan2(selTarget->fp.y - dp.y, selTarget->fp.x - dp.x);

    	steps[step].pos.x = dp.x;
    	steps[step].pos.y = dp.y;
    	steps[step].direction = dir;
    	steps[step].state = curPhase;
    	steps[step].targetIdx = curTarget;
    	steps[step].dropPoint = selTarget->fp;
    	Coord dc = { (float)cos(dir), (float)sin(dir) };
        steps[step].aimPoint = dp + dc * bombDist;
        steps[step].predictedTarget = selTarget->pp;

        DEBUG("[" << step << "] X: " << dp.x << ", Y: " << dp.y);

    	float addPath = 0;
    	float turnAngle = normalizeAngle(selTarget->dir - dir);

    	switch (curPhase) {
    		case STOPPED: {
    			if (fabs(turnAngle) > config->getTurnThreshold())
    				curPhase = TURNING;
    			else {
    				dir = selTarget->dir;
    				curPhase = ACCELERATING;
    			}
    			break;
            }
    		case ACCELERATING: {
    			float prevSpeed = curSpeed;
    			if (fabs(turnAngle) > config->getTurnThreshold() && curSpeed > 0.01) {
    				curPhase = DECELERATING;
    				curSpeed = curSpeed - acc * config->getSimTimeStep();
    				if (curSpeed <= 0) {
    					curSpeed = 0;
    					curPhase = STOPPED;
    				}
    			}
    			else {
    				if (fabs(turnAngle) <= config->getTurnThreshold())
    					dir = selTarget->dir;

    				curSpeed = curSpeed + acc * config->getSimTimeStep();
    				if (curSpeed >= config->getAttackSpeed()) {
    					curSpeed = config->getAttackSpeed();
    					curPhase = MOVING;
    				}
    			}
   				addPath = (prevSpeed + curSpeed) / 2.0 * config->getSimTimeStep();
   				break;
            }
   			case DECELERATING: {
   				float prevSpeed = curSpeed;
   				curSpeed = curSpeed - acc * config->getSimTimeStep();
   				if (curSpeed <= 0) {
   					curSpeed = 0;
   					curPhase = STOPPED;
   				}
   				addPath = (prevSpeed + curSpeed) / 2.0 * config->getSimTimeStep();
   				break;
            }
   			case TURNING: {
   				float da = normalizeAngle(selTarget->dir - dir);
   				if (fabs(da) <= config->getAngularSpeed() * config->getSimTimeStep()) {
   					dir = selTarget->dir;
   					curPhase = ACCELERATING;
   				}
   				else {
                    if (da > 0)
                        da = 1;
                    else
                        da = -1;
   					dir = normalizeAngle(dir + da * config->getAngularSpeed() * config->getSimTimeStep());
   				}
   				break;
            }
   			case MOVING: {
   				if (fabs(turnAngle) > config->getTurnThreshold()) {
   					curPhase = DECELERATING;
   					float prevSpeed = curSpeed;
   					curSpeed = curSpeed - acc * config->getSimTimeStep();
   					if (curSpeed <= 0) {
   						curSpeed = 0;
   						curPhase = STOPPED;
   					}
	   				addPath = (prevSpeed + curSpeed) / 2 * config->getSimTimeStep();
   				}
   				else {
    				if (fabs(turnAngle) <= config->getTurnThreshold()) {
    					dir = selTarget->dir;
                    }    
   					addPath = curSpeed * config->getSimTimeStep();
   				}
   				break;
            }
    	}

    	dp.x = dp.x + cos(dir) * addPath;
    	dp.y = dp.y + sin(dir) * addPath;

    	step++;
    	curTime = curTime + config->getSimTimeStep();

    	if (curPhase == MOVING && dp.length(dp - selTarget->fp) <= config->getHitRadius() * 0.2)
    		break;
    }

    delete selTarget;

    return step;
}

void MissionProcessor::nextTargetCalc(SelectedTarget* selTarget, const float& curTime, const float& bombDist, const float& bombTime, const Coord& dp, const float& dir, const float& curSpeed, const DronePhase& curPhase, const int& curTarget) {
    Coord p{0, 0};
    float totalTime;
    Coord fireP, midP;
	bool midF;

    for (int i = 0; i < 10; i++) {
        totalTime = 0;

        p = targets->getTargetPosition(cTarget, curTime, totalTime + bombTime, config->getArrayTimeStep());

        calcFirePosition(dp, p, bombDist, config->getAccelerationPath(), fireP, midP, midF);

        if (midF && cTarget == curTarget && (curPhase == MOVING || curPhase == ACCELERATING))
            midF = false;

        if (midF) {
            totalTime = totalTime + calcTotalTime(dp, midP, dir, config->getTurnThreshold(), config->getAngularSpeed(), curSpeed, config->getAttackSpeed(), config->getAccelerationPath(), true);
            float fdir = atan2(fireP.y - midP.y, fireP.x - midP.y);
            totalTime = totalTime + fabs(normalizeAngle(fdir - dir)) / config->getAngularSpeed();
            totalTime = totalTime + calcTotalTime(midP, fireP, fdir, config->getTurnThreshold(), config->getAngularSpeed(), config->getAttackSpeed(), config->getAttackSpeed(), config->getAccelerationPath(), false);
        }
        else
            totalTime = totalTime + calcTotalTime(dp, fireP, dir, config->getTurnThreshold(), config->getAngularSpeed(), curSpeed, config->getAttackSpeed(), config->getAccelerationPath(), false);
    }

    if (totalTime < selTarget->time) {
        selTarget->num = cTarget;
        selTarget->time = totalTime;
        selTarget->fp = fireP;
        selTarget->pp = p;
    }

    cTarget++;
}


void MissionProcessor::calcFirePosition(Coord dp, Coord tp, float bombDist, float addDist, Coord& fp, Coord& mp, bool& midF) {
    Coord delta = tp - dp;
	float D = delta.length();
	midF = bombDist + addDist > D;

	if (midF) {
		if (fabs(D) < 0) {
			dp.x = tp.x - (bombDist + addDist);
			dp.y = tp.y;
			D = bombDist + addDist;
		}
		else {
			dp.x = tp.x - (tp.x - dp.x) * (bombDist + addDist) / D;
			dp.y = tp.y - (tp.y - dp.y) * (bombDist + addDist) / D;
			delta = tp - dp;
			D = delta.length();
		}
		mp = dp;
	}

	float ratio = (D - bombDist) / D;
    fp.x = dp.x + (tp.x - dp.x) * ratio;
    fp.y = dp.y + (tp.y - dp.y) * ratio;
}

float MissionProcessor::calcTotalTime(Coord dp, Coord tp, float curDir, float angleThreshold, float angularSpeed, float curSpeed, float maxSpeed, float accPath, bool stopf) {
	float totalTime = 0;
	float dir = atan2(tp.y - dp.y, tp.x - dp.x);
	float a = maxSpeed * maxSpeed / (2 * accPath);

	if (fabs(normalizeAngle(dir - curDir)) > angleThreshold) {
		float pathToStop = (curSpeed * curSpeed) / (2 * a);

		dp.x = dp.x + cos(curDir) * pathToStop;
		dp.y = dp.y + sin(curDir) * pathToStop;

		totalTime = (curSpeed / a) + fabs(normalizeAngle(dir - curDir)) / angularSpeed;

		curSpeed = 0;
		curDir = dir;
	}

	float maxAccDist = (maxSpeed * maxSpeed) / (2 * a);
	float curAccDist = (curSpeed * curSpeed) / (2 * a);
	Coord delta = tp - dp;
	float D = delta.length();
    float addDist = 0;
	if (stopf) addDist = maxAccDist;
	if (addDist > D) addDist = D;
	D = D - addDist;
	totalTime = totalTime + sqrt(2 * addDist / a);

	addDist = maxAccDist - curAccDist;
	if (addDist > D) addDist = D;
	totalTime = totalTime + sqrt(2 * accPath / a);
	D = D - addDist;
	totalTime = totalTime + D / maxSpeed;

	return totalTime;
}

float MissionProcessor::normalizeAngle(float a) {
	while (a > M_PI) a = a - 2 * M_PI;
	while (a < -M_PI) a = a + 2 * M_PI;
	return a;
}

AmmoParams* MissionProcessor::readAmmoParams(const std::string fileName, int& ammoCount) {
    if (this->ammo) delete[] this->ammo; // на той випадок, якщо буде повторний виклик читання параметрів боєприпасів
    std::ifstream f(fileName);
    if (!f.is_open())
        return nullptr;
    json j;
    f >> j;
    f.close();

    ammoCount = (int)j.size();
    AmmoParams* ammo = new AmmoParams[ammoCount];

    for (int i = 0; i < ammoCount; i++) {
        ammo[i].name = j[i]["name"];
        ammo[i].mass = j[i]["mass"];
        ammo[i].drag = j[i]["drag"];
        ammo[i].lift = j[i]["lift"];
    }

    return ammo;
}

bool MissionProcessor::getBombParams(float& m, float& d, float& l) {
    bool result = false;
    for(int i = 0; i < ammoParamsCount; ++i) {
        if (config->getAmmoName()==ammo[i].name) {
            m = ammo[i].mass;
            d = ammo[i].drag;
            l = ammo[i].lift;
            result = true;
            break;
        }
    }
    return result;
}
