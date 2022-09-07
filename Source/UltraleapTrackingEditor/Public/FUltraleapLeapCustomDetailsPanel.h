/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"
#include "LeapComponent.h"
/**
 * Helper class to enable live updates of the detail tab on the leap component from device events
 * This is needed as by default, component parameters are not bound 'upwards' to the device panel when changed from 
 * C++ (for example when a device is plugged or unplugged, the device chooser drop down needs updating)
 */
class FUltraleapLeapCustomDetailsPanel : public IDetailCustomization
{
public:
	FUltraleapLeapCustomDetailsPanel();
	~FUltraleapLeapCustomDetailsPanel();

private:
	
	bool PassDetailsToLeapComponent(IDetailLayoutBuilder& DetailBuilder);

public:
	/* Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/* IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	virtual void PendingDelete() override;

	TArray<ULeapComponent*> ComponentsReferenced;
};
