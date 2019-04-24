// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BodyStateInputInterface.h"
#include "BodyStateDeviceConfig.h"


class FBodyStateHMDDevice : public IBodyStateInputRawInterface
{
public:
	FBodyStateHMDDevice();
	virtual ~FBodyStateHMDDevice();
	int32 HMDDeviceIndex;
	bool bShouldTrackMotionControllers;
	float MotionControllerInertialConfidence;
	float MotionControllerTrackedConfidence;

	FBodyStateDeviceConfig Config;

	virtual void UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton) override;
	virtual void OnDeviceDetach() override;
};