/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once
#include "FUltraleapCombinedDevice.h"

class FUltraleapCombinedDeviceAngular : public FUltraleapCombinedDevice
{
public:
	FUltraleapCombinedDeviceAngular(IHandTrackingWrapper* LeapDeviceWrapperIn, ITrackingDeviceWrapper* TrackingDeviceWrapperIn,
		TArray<IHandTrackingWrapper*> DevicesToCombineIn)
		: FUltraleapCombinedDevice(LeapDeviceWrapperIn, TrackingDeviceWrapperIn, DevicesToCombineIn)
	{
	}
	
protected:
	virtual void CombineFrame(const TArray<FLeapFrameData>& SourceFrames) override
	{
	}

private:
};
