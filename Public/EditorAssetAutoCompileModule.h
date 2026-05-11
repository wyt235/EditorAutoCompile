#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FEditorAssetAutoCompileModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void HandlePackageSaved(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext);

private:
    bool bBound = false;
};
