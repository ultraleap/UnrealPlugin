// /****************************************************************************** * Copyright (C) Ultraleap, Ltd. 2011-2021. * *
// * * Use subject to the terms of the Apache License 2.0 available at            * * http://www.apache.org/licenses/LICENSE-2.0, or
// another agreement           * * between Ultraleap and you, your company or other organization.             *
// ******************************************************************************/

#include "OpenXRToLeapWrapper.h"

#include "IHandTracker.h"

FOpenXRToLeapWrapper::FOpenXRToLeapWrapper() : HandTracker(nullptr)
{
}

FOpenXRToLeapWrapper::~FOpenXRToLeapWrapper()
{
}
void FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule()
{
	IModuleInterface* ModuleInterface = FModuleManager::Get().LoadModule("OpenXRHandTracking");
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
	{
		HandTracker = &IModularFeatures::Get().GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());
	}
}
void FOpenXRToLeapWrapper::UpdateHandState()
{
	if (HandTracker == nullptr)
	{
		return;
	}
	{
		FOccluderVertexArray OutPositions;
		TArray<FQuat> OutRotations;
		TArray<float> OutRadii;

		bool Status = HandTracker->GetAllKeypointStates(EControllerHand::Left, OutPositions, OutRotations, OutRadii);
	}
}