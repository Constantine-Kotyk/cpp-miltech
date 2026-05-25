#pragma once

#include "core/umacro.h"
#include "interfaces/IConfigLoader.h"
#include "config/ConfigLoaderJSON.h"
#include "config/ConfigLoaderTest.h"

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
