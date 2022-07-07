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
	virtual void CombineFrame(const TArray<FLeapFrameData>& SourceFrames) = 0;


	// the combined devices
	TArray<IHandTrackingWrapper*> DevicesToCombine;

	// Helpers ported from VectorHand.cs, used from multiple Combiners
	// Equivalent of VectorHand.Encode
	void CreateLocalLinearJointList(const FLeapHandData& Hand, TArray<FVector>& JointsPositions);
	// Equivalent of VectorHand.Decode
	void ConvertToWorldSpaceHand(FLeapHandData& Hand,
		const bool IsLeft, const FVector& PalmPos, const FQuat& PalmRot, const TArray<FVector>& JointPositions);

	void CreateLinearJointListInterp(const FLeapHandData& Hand, TArray<FVector>& Joints);

	// Based on VectorHand.NUM_JOINT_POSITIONS
	static const int NumJointPositions = 25;
	static int HandID;
	
	FTransform GetSourceDeviceOrigin(const int ProviderIndex);
	

private:
};
