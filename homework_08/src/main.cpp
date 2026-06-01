#define ENABLE_LOG	  1
#define ENABLE_DEBUG  0

#include <string>

#include "config/ConfigLoader.h"
#include "providers/TargetProvider.h"
#include "solvers/AnalyticalSolver.h"
#include "core/MissionProcessor.h"
#include "core/umacro.h"

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
                                    .setMaxSteps(10000)
                                    .setAmmoParams(dataDir + "ammo.json")
                                    .setDroneConfig(fileConfigLoader)
                                    .setBallisticSolver(analytical)
                                    .setTargetProvider(jsonProvider)
                                    .build();
    int stepCnt;
    stepCnt = mission->execute();
    if (stepCnt > 0) {
        LOG("Steps count with JSON file provider: " << stepCnt);
        mission->writeResult(outputDir + "simulation.json");
    }
    mission->changeProvider(testProvider);
    stepCnt = mission->execute();
    if (stepCnt > 0) {
        LOG("Steps count with test provider: " << stepCnt);
        mission->writeResult(outputDir + "simulation_test.json");
    }

    delete mission;
    delete fileConfigLoader;
    delete jsonProvider;
    delete testProvider;
    delete analytical;

    return 0;
}
