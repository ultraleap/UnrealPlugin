// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenXRULHandTrackingExt.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"

#define LOCTEXT_NAMESPACE "OpenXRULHandTrackingExt"

IMPLEMENT_MODULE(FOpenXRULHandTrackingExt, OpenXRULHandTrackingExt);

void FOpenXRULHandTrackingExt::StartupModule()
{
	RegisterOpenXRExtensionModularFeature();
}

bool FOpenXRULHandTrackingExt::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add("XR_ULTRALEAP_hand_tracking_forearm");
	return true;
}

bool FOpenXRULHandTrackingExt::GetInteractionProfile(
	XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics)
{
	OutKeyPrefix = "OpenXRULHandTrackingExt";
	OutHasHaptics = false;
	return xrStringToPath(InInstance, "/interaction_profiles/ultraleap/hand_interaction", &OutPath) == XR_SUCCESS;
}

#undef LOCTEXT_NAMESPACE
