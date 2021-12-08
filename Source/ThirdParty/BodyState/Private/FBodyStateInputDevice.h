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

#pragma once

#include "BodyStateDevice.h"
#include "CoreMinimal.h"
#include "IInputDevice.h"
#include "InputCoreTypes.h"

class UBodyStateBoneComponent;
class FBodyStateSkeletonStorage;

class FBodyStateInputDevice : public IInputDevice
{
public:
	FBodyStateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler);

	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime) override;

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;

	/** Actual Tick functions, abstracted so we have control over flow*/
	void DispatchInput();		   // raw input
	void DispatchEstimators();	   // merge and estimation
	void DispatchRecognizers();	   // recognition ( heavy parts may run off-thread)
	void UpdateSceneListeners();

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	/** IForceFeedbackSystem pass through functions **/
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override;

	virtual ~FBodyStateInputDevice();

	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;
	TSharedPtr<FBodyStateSkeletonStorage> SkeletonStorage;

	// Define mixing and update interfaces - maybe change () to (USkeletons) so the algorithm can loop through the skeletons for
	// merging
	bool AttachMergeAlgorithm(TFunction<void()> InFunction);

	// Scene Component Listeners
	void AddBoneSceneListener(UBodyStateBoneComponent* Listener);
	void RemoveBoneSceneListener(UBodyStateBoneComponent* Listener);

private:
	class BSHMDSnapshotHandler* HMDSamples;	   // Time-warp

	// Private Body State variables
	TMap<IBodyStateInputRawInterface*, FBodyStateDevice> Devices;
	TMap<int32, IBodyStateInputRawInterface*> DeviceKeyMap;

	TArray<UBodyStateBoneComponent*> BoneSceneListeners;

	// Private utility methods
	bool EmitKeyUpEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitKeyDownEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitAnalogInputEventForKey(FKey Key, float Value, int32 User, bool Repeat);
};