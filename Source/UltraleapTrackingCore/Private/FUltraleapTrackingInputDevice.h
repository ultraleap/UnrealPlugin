/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "BodyStateDeviceConfig.h"
#include "BodyStateHMDSnapshot.h"
#include "BodyStateInputInterface.h"
#include "IInputDevice.h"
#include "IXRTrackingSystem.h"
#include "LeapC.h"
#include "LeapComponent.h"
#include "LeapImage.h"
#include "LeapLiveLink.h"
#include "LeapUtility.h"
#include "LeapWrapper.h"
#include "OpenXRToLeapWrapper.h"
#include "SceneViewExtension.h"
#include "UltraleapTrackingData.h"
#include "IUltraleapTrackingPlugin.h"
/**
 * Stores raw controller data and custom toggles
 */

class FUltraleapTrackingInputDevice : public IInputDevice, public LeapWrapperCallbackInterface
{
public:
	FUltraleapTrackingInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler);
	virtual ~FUltraleapTrackingInputDevice();

	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime) override;

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override
	{
		return false;
	}

	/**
	 * IForceFeedbackSystem pass through functions
	 */
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override
	{
	}
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override
	{
	}

	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;

	void AddEventDelegate(const ULeapComponent* EventDelegate);
	void RemoveEventDelegate(const ULeapComponent* EventDelegate);
	void ShutdownLeap();
	void AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible);
	void LatestFrame(FLeapFrameData& OutFrame);
	void SetSwizzles(ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW);
	// Policy and toggles
	void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable);
	void SetTrackingMode(ELeapMode Flag);
	virtual void OnDeviceDetach();

	FCriticalSection LeapSection;

	void SetOptions(const FLeapOptions& Options);
	FLeapOptions GetOptions();
	FLeapStats GetStats();
	const TArray<FString>& GetAttachedDevices()
	{
		return AttachedDevices;
	}

	ILeapConnector* GetConnector()
	{
		return Connector;
	}
	void PostEarlyInit();

private:
	// Private UProperties
	
	TArray<ULeapComponent*> EventDelegates;	   // delegate storage

	// Private utility methods
	void CallFunctionOnComponents(TFunction<void(ULeapComponent*)> InFunction);	   // lambda multi-cast convenience wrapper
	bool EmitKeyUpEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitKeyDownEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitAnalogInputEventForKey(FKey Key, float Value, int32 User, bool Repeat);
	
	void InitTrackingSource(const bool UseOpenXRAsSource);

	TArray<FString> AttachedDevices;
	
	
	// Wrapper link
	TSharedPtr<IHandTrackingWrapper> Leap;
	ILeapConnector* Connector;
	FLeapStats Stats;
	// LeapWrapper Callbacks
	// Global
	virtual void OnConnect() override;
	virtual void OnConnectionLost() override;
	virtual void OnDeviceFound(const LEAP_DEVICE_INFO* props) override;
	virtual void OnDeviceLost(const char* serial) override;
	virtual void OnDeviceFailure(const eLeapDeviceStatus failure_code, const LEAP_DEVICE failed_device) override;

	bool IsWaitingForConnect = false;
};