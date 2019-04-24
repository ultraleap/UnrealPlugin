// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "IInputDevice.h"
#include "BodyStateDevice.h"

class UBodyStateBoneComponent;
class FBodyStateSkeletonStorage;

class FBodyStateInputDevice : public IInputDevice
{
public:
	FBodyStateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& MessageHandler);

	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime) override;

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;

	/** Actual Tick functions, abstracted so we have control over flow*/
	void DispatchInput();		//raw input
	void DispatchEstimators();	//merge and estimation
	void DispatchRecognizers();	//recognition ( heavy parts may run off-thread)
	void UpdateSceneListeners();

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	/** IForceFeedbackSystem pass through functions **/
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) override;

	virtual ~FBodyStateInputDevice();

	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;
	TSharedPtr<FBodyStateSkeletonStorage> SkeletonStorage;

	//Define mixing and update interfaces - maybe change () to (USkeletons) so the algorithm can loop through the skeletons for merging
	bool AttachMergeAlgorithm(TFunction< void()> InFunction);

	//Scene Component Listeners
	void AddBoneSceneListener(UBodyStateBoneComponent* Listener);
	void RemoveBoneSceneListener(UBodyStateBoneComponent* Listener);

private:
	class BSHMDSnapshotHandler* HMDSamples;			//Time-warp

	//Private Body State variables
	TMap<IBodyStateInputRawInterface*, FBodyStateDevice> Devices;
	TMap<int32, IBodyStateInputRawInterface*> DeviceKeyMap;

	TArray<UBodyStateBoneComponent*> BoneSceneListeners;

	//Private utility methods
	bool EmitKeyUpEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitKeyDownEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitAnalogInputEventForKey(FKey Key, float Value, int32 User, bool Repeat);
};