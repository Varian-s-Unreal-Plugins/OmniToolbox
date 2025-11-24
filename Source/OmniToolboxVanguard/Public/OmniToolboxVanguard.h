#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

OMNITOOLBOXVANGUARD_API DECLARE_LOG_CATEGORY_EXTERN(LogVanguard, Log, All);

class FOmniToolboxVanguardModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
