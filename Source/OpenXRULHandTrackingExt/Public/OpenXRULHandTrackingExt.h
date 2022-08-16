// Copyright Epic Games, Inc. All Rights Reserved.

/************************************************************************
 * This plugin is a good example of a very simple OpenXRExtension plugin.
 * 
 * It's all one class which implements both IOpenXRExtensionPlugin and
 * IModuleInterface.
 * 
 * If the XR_MSFT_hand_interaction OpenXR extension is available on the current
 * OpenXR runtime this OpenXRExtensionPlugin will register an Interaction Profile 
 * which makes the left/right hand poses available and will setup the Select and 
 * Grip buttons so they can be bound in UE4.
 ************************************************************************/

#pragma once
#include "IOpenXRExtensionPlugin.h"

#include "Modules/ModuleInterface.h"
#include "OpenXRCore.h"

class FOpenXRULHandTrackingExt : public IModuleInterface, public IOpenXRExtensionPlugin
{
public:
	/************************************************************************/
	/* IModuleInterface                                                     */
	/************************************************************************/
	virtual void StartupModule() override;

	/************************************************************************/
	/* IOpenXRExtensionPlugin                                               */
	/************************************************************************/
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("OpenXRULHandTrackingExt"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetInteractionProfile(XrInstance InInstance, FString& OutKeyPrefix, XrPath& OutPath, bool& OutHasHaptics) override;
};
