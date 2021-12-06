

#pragma once

#include "BodyStateDeviceConfig.h"
#include "BodyStateInputInterface.h"

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