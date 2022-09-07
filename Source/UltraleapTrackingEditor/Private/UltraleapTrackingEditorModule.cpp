/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "UltraleapTrackingEditorModule.h"
#include "BodyStateAnimInstance.h"
#include "LeapComponent.h"
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

	PropertyModule.RegisterCustomClassLayout(ULeapComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FUltraleapLeapCustomDetailsPanel::MakeInstance));
}