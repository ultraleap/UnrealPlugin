// /****************************************************************************** * Copyright (C) Ultraleap, Ltd. 2011-2021. * *
// * * Use subject to the terms of the Apache License 2.0 available at            * * http://www.apache.org/licenses/LICENSE-2.0, or
// another agreement           * * between Ultraleap and you, your company or other organization.             *
// ******************************************************************************/

#include "OpenXRToLeapWrapper.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "IHandTracker.h"

FOpenXRToLeapWrapper::FOpenXRToLeapWrapper() : HandTracker(nullptr)
{
	DummyLeapHands[0] = {0};
	DummyLeapHands[1] = {0};
	DummyLeapFrame = {0};

	DummyLeapFrame.framerate = 90;
	DummyLeapFrame.pHands = DummyLeapHands;
	InitOpenXRHandTrackingModule();
}

FOpenXRToLeapWrapper::~FOpenXRToLeapWrapper()
{
}
void FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule()
{
	IModuleInterface* ModuleInterface = FModuleManager::Get().LoadModule("OpenXRHandTracking");
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
	{
		HandTracker = &IModularFeatures::Get().GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());
	}
}
void FOpenXRToLeapWrapper::ConvertToLeapSpace(LEAP_HAND& LeapHand, const FOccluderVertexArray& Positions,const TArray<FQuat>& Rotations)
{

}

LEAP_TRACKING_EVENT* FOpenXRToLeapWrapper::GetInterpolatedFrameAtTime(int64 TimeStamp)
{
	
	if (HandTracker == nullptr)
	{
		return &DummyLeapFrame;
	}
	FOccluderVertexArray OutPositions[2];
	TArray<FQuat> OutRotations[2];
	TArray<float> OutRadii[2];

	// status only true when the hand is being tracked/visible to the tracking device
	
	bool StatusLeft = HandTracker->GetAllKeypointStates(EControllerHand::Left, OutPositions[0], OutRotations[0], OutRadii[0]);
	bool StatusRight = HandTracker->GetAllKeypointStates(EControllerHand::Right, OutPositions[1], OutRotations[1], OutRadii[1]);

	DummyLeapFrame.nHands = StatusLeft + StatusRight;
	DummyLeapFrame.info.frame_id++;
	UWorld* World = nullptr;
	if (GEngine)
	{
		World = GEngine->GetWorld();
	}
	// time in microseconds
	if (World)
	{
		DummyLeapFrame.info.timestamp = World->GetTimeSeconds() * 1000000.0f;
	}
	DummyLeapFrame.tracking_frame_id++;

	if (!StatusLeft)
	{
		DummyLeapFrame.pHands = &DummyLeapHands[1];
	}
	else
	{
		DummyLeapFrame.pHands = &DummyLeapHands[0];
	}

	if (StatusLeft)
	{
		ConvertToLeapSpace(DummyLeapHands[0], OutPositions[0], OutRotations[0]);
	}
	if (StatusRight)
	{
		ConvertToLeapSpace(DummyLeapHands[1], OutPositions[1], OutRotations[1]);
	}
	return &DummyLeapFrame;
}
void FOpenXRToLeapWrapper::UpdateHandState()
{

}