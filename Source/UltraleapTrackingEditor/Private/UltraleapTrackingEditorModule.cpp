

#include "UltraleapTrackingEditorModule.h"

#include "BodystateAnimInstance.h"
#include "FUltraleapAnimCustomDetailsPanel.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

IMPLEMENT_MODULE(FUltraleapTrackingEditorModule, UltraleapTrackingEditor);

void FUltraleapTrackingEditorModule::StartupModule()
{
	// Get the property module
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	// Register the custom details panel we have created
	PropertyModule.RegisterCustomClassLayout(UAnimGraphNode_ModifyBodyStateMappedBones::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FUltraleapAnimCustomDetailsPanel::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(UBodyStateAnimInstance::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FUltraleapAnimCustomDetailsPanel::MakeInstance));
}