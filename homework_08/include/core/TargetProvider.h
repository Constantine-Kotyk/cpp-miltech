#pragma once

#include "interfaces/ITargetProvider.h"
#include "core/TargetProviderJSON.h"
#include "core/TargetProviderTest.h"
#include "core/umacro.h"

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
