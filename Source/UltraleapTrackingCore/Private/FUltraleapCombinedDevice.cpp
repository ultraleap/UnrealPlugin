/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapCombinedDevice.h"

FUltraleapCombinedDevice::FUltraleapCombinedDevice(IHandTrackingWrapper* LeapDeviceWrapper,
	ITrackingDeviceWrapper* TrackingDeviceWrapperIn, TArray<IHandTrackingWrapper*> DevicesToCombineIn) : 
	FUltraleapDevice(LeapDeviceWrapper, TrackingDeviceWrapperIn),
	DevicesToCombine(DevicesToCombineIn)
{
}

FUltraleapCombinedDevice::~FUltraleapCombinedDevice()
{
	FUltraleapDevice::~FUltraleapDevice();
}
// Main loop event emitter and handler
void FUltraleapCombinedDevice::SendControllerEvents()
{
	// Create combined frame here and call parse
	// the parent class will then behave as if it had one device
	CombineFrame();
	ParseEvents();
}
void FUltraleapCombinedDevice::CombineFrame()
{
	TArray<FLeapFrameData> SourceFrames;
	// add combiner logic based on DevicesToCombine List. All devices will have ticked before this is called
	for (auto SourceDevice : DevicesToCombine)
	{
		auto InternalSourceDevice = SourceDevice->GetDevice();
		if (InternalSourceDevice)
		{
			FLeapFrameData SourceFrame;
			InternalSourceDevice->GetLatestFrameData(SourceFrame);
			SourceFrames.Add(SourceFrame);
		}
	}
}