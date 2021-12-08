/*************************************************************************************************************************************
 *The MIT License(MIT)
 *
 *Copyright(c) 2016 Jan Kaniewski(Getnamo)
 *Modified work Copyright(C) 2019 - 2021 Ultraleap, Inc.
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 *files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
 *merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions :
 *
 *The above copyright notice and this permission notice shall be included in all copies or
 *substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 *FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *************************************************************************************************************************************/

#include "FBodyStateInputDevice.h"

#include "BodyStateBoneComponent.h"
#include "BodyStateDevice.h"
#include "BodyStateHMDSnapshot.h"
#include "BodyStateInputInterface.h"
#include "BodyStateSkeletonStorage.h"
#include "Framework/Application/SlateApplication.h"

// UE v4.6 IM event wrappers
bool FBodyStateInputDevice::EmitKeyUpEventForKey(FKey Key, int32 User, bool Repeat)
{
	FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
	return FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
}

bool FBodyStateInputDevice::EmitKeyDownEventForKey(FKey Key, int32 User, bool Repeat)
{
	FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
	return FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
}

bool FBodyStateInputDevice::EmitAnalogInputEventForKey(FKey Key, float Value, int32 User, bool Repeat)
{
	FAnalogInputEvent AnalogInputEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0, Value);
	return FSlateApplication::Get().ProcessAnalogInputEvent(AnalogInputEvent);
}

FBodyStateInputDevice::FBodyStateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
{
}

FBodyStateInputDevice::~FBodyStateInputDevice()
{
}

void FBodyStateInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FBodyStateInputDevice::AttachMergeAlgorithm(TFunction<void()> InFunction)
{
	return false;
}

void FBodyStateInputDevice::AddBoneSceneListener(UBodyStateBoneComponent* Listener)
{
	BoneSceneListeners.Add(Listener);
}

void FBodyStateInputDevice::RemoveBoneSceneListener(UBodyStateBoneComponent* Listener)
{
	BoneSceneListeners.Remove(Listener);
}

bool FBodyStateInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
	return false;
}

void FBodyStateInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}

void FBodyStateInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}

/************************************************************************/
/* Key Tick flow functions                                              */
/************************************************************************/
void FBodyStateInputDevice::Tick(float DeltaTime)
{
	// Tick - store delta time for BS refence
}

// Main loop event emitter
void FBodyStateInputDevice::SendControllerEvents()
{
	// HMDSamples->AddCurrentHMDSample();

	DispatchInput();
	DispatchEstimators();

	// DispatchRecognizers();

	UpdateSceneListeners();
}

// End key tick flow functions

/************************************************************************/
/* Key Input functions                                                  */
/************************************************************************/
void FBodyStateInputDevice::DispatchInput()
{
	// TODO expand this

	// Fetch input from all attached devices
	SkeletonStorage->CallFunctionOnDevices(
		[this](const FBodyStateDevice& Device) { Device.InputCallbackDelegate->UpdateInput(Device.DeviceId, Device.Skeleton); });
}

void FBodyStateInputDevice::DispatchEstimators()
{
	// Copy results to merged skeleton and obtain estimator data if any
	SkeletonStorage->UpdateMergeSkeletonData();
}

void FBodyStateInputDevice::DispatchRecognizers()
{
}

void FBodyStateInputDevice::UpdateSceneListeners()
{
	// Early exit optimization
	if (BoneSceneListeners.Num() == 0)
	{
		return;
	}

	for (auto Listener : BoneSceneListeners)
	{
		// Get relevant skeleton for listener
		UBodyStateSkeleton* Skeleton = SkeletonStorage->SkeletonForDevice(Listener->SkeletonId);
		UBodyStateBone* FollowedBone = Skeleton->BoneForEnum(Listener->BoneToFollow);

		// Update scene transform for that bone from the bone enum
		Listener->SetRelativeTransform(FollowedBone->Transform());
	}
}
