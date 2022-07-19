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
	virtual void CombineFrame(const TArray<FLeapFrameData>& SourceFrames) override;

public:
	float Cam1Alpha;
	float Cam2Alpha;

	float LeftAngle;
	float RightAngle;

	float MaxInterpolationAngle = 60;


	FTransform MidpointDevices;	  // used to calculate relative angle and weight hands accordingly. Transform should face direction
								  // that bisects FOV of devices

private:
	
	FVector MidDevicePointPosition;
	FVector MidDevicePointForward;
	FVector MidDevicePointUp;

	void MergeHands(
		const TArray<FLeapFrameData>& SourceFrames, TArray<FLeapHandData>& Hands, bool& LeftHandVisible, bool& RightHandVisible);
	static float AngleSigned(const FVector& V1, const FVector& V2, const FVector& N);
	bool AngularInterpolate(const TArray<const FLeapHandData*>& HandList, float& Alpha, float& Angle, FLeapHandData& Hand);
};
