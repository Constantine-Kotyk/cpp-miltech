#pragma once

#include "interfaces/ITargetProvider.h"
#include "providers/TargetProviderJSON.h"
#include "providers/TargetProviderTest.h"
#include "core/umacro.h"

enum class ProviderType {JSON, TEST};

class TargetProviderFactory {
public:
    static std::unique_ptr<ITargetProvider> create(ProviderType type, const std::string param = "") {
        LOG("New target provider created (" << (((int)type==0)?"JSON":"TEST") << ")");
        switch (type) {
            case ProviderType::JSON: return std::make_unique<TargetProviderJSON>(param);
            case ProviderType::TEST: return std::make_unique<TargetProviderTest>();
            default: return nullptr;
        }
    }
};
