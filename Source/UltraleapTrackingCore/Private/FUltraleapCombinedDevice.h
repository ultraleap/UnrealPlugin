/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once
#include "FUltraleapDevice.h"

class FUltraleapCombinedDevice : public FUltraleapDevice
{
public:
	FUltraleapCombinedDevice(IHandTrackingWrapper* LeapDeviceWrapperIn, ITrackingDeviceWrapper* TrackingDeviceWrapperIn, TArray<IHandTrackingWrapper*> DevicesToCombineIn);
	virtual ~FUltraleapCombinedDevice();

	

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;
		
protected:
	// override this in any custom combiners
	virtual void CombineFrame();

private:
	// the combined devices
	TArray<IHandTrackingWrapper*> DevicesToCombine;
};
