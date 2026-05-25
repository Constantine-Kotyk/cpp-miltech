#include <iostream>
#include <cmath>
#include <fstream>
#include <cstring>
#include <string>
#include "json.hpp"

#define ENABLE_LOG	  1
#define ENABLE_DEBUG  0

#if ENABLE_LOG
  #define LOG(msg) std::cout << "[LOG] " << msg << std::endl
#else
  #define LOG(msg)
#endif

#if ENABLE_DEBUG
  #define DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
  #define DEBUG(msg)
#endif

#define ERROR(msg) std::cerr << msg << std::endl

enum DronePhase {STOPPED, ACCELERATING, DECELERATING, TURNING, MOVING};

struct Coord {
	float x;
	float y;

	// Додавання координат
	Coord operator+(const Coord& other) const {
    	Coord result;
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
	}

	// Віднімання координат
	Coord operator-(const Coord& other) const {
    	Coord result;
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
	}

	// Множення на скаляр
	Coord operator*(float s) const {
    	Coord result;
        result.x = x * s;
        result.y = y * s;
        return result;
	}

	// Ділення на скаляр
	Coord operator/(float s) const {
    	Coord result;
        result.x = x / s;
        result.y = y / s;
        return result;
	}

	// Порівняння
	bool operator==(const Coord& other) const {
        return x == other.x && y == other.y;
	}

	float length() {
	    return std::hypot(x, y);
	}

	float length(Coord c) {
	    return std::hypot(c.x, c.y);
	}
};

struct AmmoParams {
	std::string name;
	float mass; 	// маса (кг)
	float drag; 	// коефіцієнт опору
	float lift; 	// коефіцієнт підйому
};

struct DroneConfig {
	Coord startPos;         	// початкова позиція (x, y)
	float altitude;     	    // висота
	float initialDir;   	    // початковий напрямок (рад)
	float attackSpeed;      	// швидкість атаки (м/с)
	float accelerationPath;    	// шлях розгону (м)
	std::string ammoName;       	    // обрані боєприпаси
	float arrayTimeStep;	    // крок часу масиву цілей
	float simTimeStep;  	    // крок симуляції
	float hitRadius;    	    // радіус влучення
	float angularSpeed; 	    // кутова швидкість (рад/с)
	float turnThreshold;	    // поріг повороту (рад)
};

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

using json = nlohmann::json;




////////////////////////////////////////////
//             CONFIG LOADER              //
////////////////////////////////////////////

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

enum class ConfigLoaderType {JSON, TEST};

class ConfigLoaderFactory {
public:
    static IConfigLoader* create(ConfigLoaderType type, const std::string param = "") {
        LOG("New config loader created (" << (((int)type==0)?"JSON":"TEST") << ")");
        switch (type) {
            case ConfigLoaderType::JSON: return new ConfigLoaderJSON(param);
            case ConfigLoaderType::TEST: return new ConfigLoaderTest();
            default: return nullptr;
        }
    }
};

////////////////////////////////////////////
//            TARGET PROVIDER             //
////////////////////////////////////////////

class ITargetProvider {
public:
    virtual int getTargetCount() = 0;
    virtual Coord getTargetPosition(int tnum, float curTime, float dt, float arrayTimeStep) = 0;
    virtual ~ITargetProvider() {}
};

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

enum class ProviderType {JSON, TEST};

class TargetProviderFactory {
public:
    static ITargetProvider* create(ProviderType type, const std::string param = "") {
        LOG("New target provider created (" << (((int)type==0)?"JSON":"TEST") << ")");
        switch (type) {
            case ProviderType::JSON: return new TargetProviderJSON(param);
            case ProviderType::TEST: return new TargetProviderTest();
            default: return nullptr;
        }
    }
};

////////////////////////////////////////////
//           BALLISTIC SOLVER             //
////////////////////////////////////////////

class IBallisticSolver {
public:
    virtual void calcBalisticTask(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd, float& bombDist, float& bombTime) = 0;
    virtual ~IBallisticSolver() {}
};

class AnalyticalSolver : public IBallisticSolver {
public:
    void calcBalisticTask(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd, float& bombDist, float& bombTime) override;
};

class TableSolver : public IBallisticSolver {
public:
    void calcBalisticTask(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd, float& bombDist, float& bombTime) override;
};

////////////////////////////////////////////
//           MISSION PROCESSOR            //
////////////////////////////////////////////

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

////////////////////////////////////////////
//                  MAIN                  //
////////////////////////////////////////////

int main(int argc, char* argv[])
{
    std::string dataDir = "";
    std::string outputDir = "";

    if (argc < 3) {
        LOG("Usage for specified data and output directory: " << argv[0] << " [data_directory_path output_directory_path]");
    }
    else {
        dataDir = argv[1];
        if (!dataDir.empty() && dataDir.back() != '/') dataDir += '/';
        outputDir = argv[2];
        if (!outputDir.empty() && outputDir.back() != '/') outputDir += '/';
    }

    AnalyticalSolver* analytical = new AnalyticalSolver;
    auto* jsonProvider = TargetProviderFactory::create(ProviderType::JSON, dataDir + "targets.json");
    auto* testProvider = TargetProviderFactory::create(ProviderType::TEST);

    auto* fileConfigLoader = ConfigLoaderFactory::create(ConfigLoaderType::JSON, dataDir + "config.json");

    MissionProcessor* mission = MissionBuilder()
                                    .setAmmoParams(dataDir + "ammo.json")
                                    .setDroneConfig(fileConfigLoader)
                                    .setBallisticSolver(analytical)
                                    .setTargetProvider(jsonProvider)
                                    .build();
    int stepCnt;
    stepCnt = mission->execute();
    if (stepCnt > 0) {
        LOG("Steps count with JSON file provider: " << stepCnt);
        mission->writeResult(outputDir + "simulation.json", stepCnt);
    }
    mission->changeProvider(testProvider);
    stepCnt = mission->execute();
    if (stepCnt > 0) {
        LOG("Steps count with test provider: " << stepCnt);
        mission->writeResult(outputDir + "simulation_test.json", stepCnt);
    }

    delete mission;
    delete fileConfigLoader;
    delete jsonProvider;
    delete testProvider;
    delete analytical;

    return 0;
}

////////////////////////////////////////////
//             IMPLEMENTATION             //
////////////////////////////////////////////

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

void AnalyticalSolver::calcBalisticTask(const float& m, const float& d, const float& l, const float& attackSpeed, const float& zd, float& bombDist, float& bombTime) {

    float g = 9.81;
    float a = d * g * m - 2 * d * d * l * attackSpeed;
    float b = -3 * g * m * m + 3 * d * l * m * attackSpeed;
    float c = 6 * m * m * zd;
    float p = -(b * b) / (3 * a * a);
    if (p < 0) {
        float q = (2 * b * b * b) / (27 * a * a * a) + (c / a);
        float phi = acos(3 * q / (2 * p) * sqrt(-3 / p));
        bombTime = 2 * sqrt(-p / 3) * cos((phi + 4 * M_PI) / 3) - b / (3 * a);
        if (bombTime <= 0)
            bombTime = sqrt(2.0f * zd / g);
    }
    else
        bombTime = sqrt(2.0f * zd / g);

    bombDist = attackSpeed * bombTime -
        pow(bombTime, 2) * d * attackSpeed / (2 * m) +
        pow(bombTime, 3) * (6 * d * g * l * m - 6 * d * d * (l * l - 1) * attackSpeed) / (36 * m * m) +
        pow(bombTime, 4) * (-6 * d * d * g * l * (1 + l * l + pow(l, 4)) * m + 3 * pow(d, 3) * l * l * (1 + l * l) * attackSpeed + 6 * pow(d, 3) * pow(l, 4) * (1 + l * l) * attackSpeed)  / (36 * pow((1 + l * l), 2) * pow(m, 3)) +
        pow(bombTime, 5) * (3 * pow(d, 3) * g * pow(l, 3) * m - 3 * pow(d, 4) * l * l * (1 + l*l) * attackSpeed) / (36 * (1 + l * l) * pow(m, 4));

}

void TableSolver::calcBalisticTask([[maybe_unused]]const float& m, 
    [[maybe_unused]]const float& d, 
    [[maybe_unused]]const float& l, 
    [[maybe_unused]]const float& attackSpeed, 
    [[maybe_unused]]const float& zd, 
    [[maybe_unused]]float& bombDist, 
    [[maybe_unused]]float& bombTime) {

    //solver not implemented yet

}
