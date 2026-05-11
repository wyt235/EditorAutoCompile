#include "EditorAssetAutoCompileModule.h"
#include "Async/Async.h"
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/MessageDialog.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/ObjectSaveContext.h"

static bool IsLikelyAutoSaveFileName(const FString& PackageFileName)
{
    return PackageFileName.Contains(TEXT("/AutoSaves/"), ESearchCase::IgnoreCase)
        || PackageFileName.Contains(TEXT("\\AutoSaves\\"), ESearchCase::IgnoreCase)
        || PackageFileName.Contains(TEXT("/Saved/AutoSaves/"), ESearchCase::IgnoreCase)
        || PackageFileName.Contains(TEXT("\\Saved\\AutoSaves\\"), ESearchCase::IgnoreCase);
}

static bool IsEditorInPIEOrSimulate()
{
    if (GEditor == nullptr)
    {
        return false;
    }

#if WITH_EDITOR
    if (GEditor->IsPlaySessionInProgress() || GEditor->PlayWorld != nullptr || GEditor->bIsSimulatingInEditor)
    {
        return true;
    }
#endif

    return false;
}

void FEditorAssetAutoCompileModule::StartupModule()
{
#if WITH_EDITOR
    if (!bBound)
    {
        UPackage::PackageSavedWithContextEvent.AddRaw(this, &FEditorAssetAutoCompileModule::HandlePackageSaved);
        bBound = true;
    }
#endif
}

void FEditorAssetAutoCompileModule::ShutdownModule()
{
#if WITH_EDITOR
    if (bBound)
    {
        UPackage::PackageSavedWithContextEvent.RemoveAll(this);
        bBound = false;
    }
#endif
}

void FEditorAssetAutoCompileModule::HandlePackageSaved(const FString& PackageFileName, UPackage* Package, FObjectPostSaveContext ObjectSaveContext)
{
#if WITH_EDITOR
    if (Package == nullptr) return;

    // 跳过AutoSave
    if (IsLikelyAutoSaveFileName(PackageFileName)) return;
	
	// 在主线程执行
	TWeakObjectPtr WeakPackage = Package;
    AsyncTask(ENamedThreads::GameThread, [WeakPackage]()
    {
    	if (WeakPackage.Get() == nullptr) return;

        if (IsEditorInPIEOrSimulate())  // PIE给出弹窗提醒
        {
            FMessageDialog::Open(
                EAppMsgType::Ok,
                FText::FromString(TEXT("当前编辑器正在运行，为避免crash，已跳过保存时编译流程。\n请停止运行后手动编译蓝图。"))
            );
            return;
        }
    	TArray<UBlueprint*> Blueprints;
    	TArray<UObject*> ObjectsInPackage;

    	// 先收集需要编译的蓝图，避免在编译过程中修改包内对象导致迭代器失效
        GetObjectsWithOuter(WeakPackage.Get(), ObjectsInPackage, true);
        for (UObject* ObjectInPackage : ObjectsInPackage)
        {
	        if (!IsValid(ObjectInPackage)) continue;
			if (UBlueprint* Blueprint = Cast<UBlueprint>(ObjectInPackage); IsValid(Blueprint))
			{
				Blueprints.Add(Blueprint);
			}
        }

    	// 编译蓝图
    	for (UBlueprint* Blueprint : Blueprints)
    	{
            FKismetEditorUtilities::CompileBlueprint(Blueprint);
    	}
    });
#endif
}


IMPLEMENT_MODULE(FEditorAssetAutoCompileModule, EditorAssetAutoCompile)
