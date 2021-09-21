// /****************************************************************************** * Copyright (C) Ultraleap, Ltd. 2011-2021. * *
// * * Use subject to the terms of the Apache License 2.0 available at            * * http://www.apache.org/licenses/LICENSE-2.0, or
// another agreement           * * between Ultraleap and you, your company or other organization.             *
// ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "LeapWrapper.h"
/**
 *
 */
class FOpenXRToLeapWrapper : FLeapWrapperBase
{
public:
	FOpenXRToLeapWrapper();
	virtual ~FOpenXRToLeapWrapper();

	void UpdateHandState();

	// FLeapWrapperBase overrides (base stubs out old leap calls)
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp) override;
	virtual LEAP_TRACKING_EVENT* GetFrame() override;
	virtual LEAP_DEVICE_INFO* GetDeviceProperties() override;

private:
	class IHandTracker* HandTracker;
	void InitOpenXRHandTrackingModule();
	LEAP_TRACKING_EVENT DummyLeapFrame;
	LEAP_HAND DummyLeapHands[2];
	LEAP_DEVICE_INFO DummyDeviceInfo;

	void ConvertToLeapSpace(LEAP_HAND& LeapHand, const FOccluderVertexArray& Positions, const TArray<FQuat>& Rotations);
};
