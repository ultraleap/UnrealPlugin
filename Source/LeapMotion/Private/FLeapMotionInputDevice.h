// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IXRTrackingSystem.h"
#include "LeapUtility.h"
#include "LeapComponent.h"
#include "LeapMotionData.h"
#include "BodyStateInputInterface.h"
#include "BodyStateHMDSnapshot.h"
#include "BodyStateDeviceConfig.h"
#include "IInputDevice.h"
#include "SceneViewExtension.h"
#include "LeapC.h"
#include "LeapWrapper.h"
#include "LeapLiveLink.h"
#include "LeapImage.h"


/**
* Stores raw controller data and custom toggles
*/

struct EKeysLeap
{
	static const FKey LeapPinchL;
	static const FKey LeapGrabL;

	static const FKey LeapPinchR;
	static const FKey LeapGrabR;
};

class FLeapMotionInputDevice :	public IInputDevice, 
								public LeapWrapperCallbackInterface, 
								public IBodyStateInputRawInterface,
								public ISceneViewExtension
{
public:
	FLeapMotionInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& MessageHandler);
	virtual ~FLeapMotionInputDevice();

	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime) override;

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;

	/** Main input capture and event parsing 'tick' */
	void CaptureAndEvaluateInput();
	void ParseEvents();

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	/** IForceFeedbackSystem pass through functions **/
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) override;

	TSharedRef< FGenericApplicationMessageHandler > MessageHandler;
	
	void AddEventDelegate(const ULeapComponent* EventDelegate);
	void RemoveEventDelegate(const ULeapComponent* EventDelegate);
	void ShutdownLeap();
	void AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible);
	void LatestFrame(FLeapFrameData& OutFrame);

	//Policy and toggles
	void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable);

	//BodyState
	virtual void UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton) override;
	virtual void OnDeviceDetach();

	FCriticalSection LeapSection;

	/** ISceneViewExtension interface */
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override {}
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;
	virtual int32 GetPriority() const override { return -10; }

	void SetOptions(const FLeapOptions& Options);
	FLeapOptions GetOptions();
	FLeapStats GetStats();

private:
	bool UseTimeBasedVisibilityCheck = false;
	bool UseTimeBasedGestureCheck = false;
	//Time Based Variables
	//Pinch And Grab Thresholds
	float StartGrabThreshold = .8f;
	float EndGrabThreshold = .5f;
	float StartPinchThreshold = .8f;
	float EndPinchThreshold = .5f;
	double TimeSinceLastLeftPinch = 0;
	double TimeSinceLastRightPinch = 0;
	double TimeSinceLastLeftGrab = 0;
	double TimeSinceLastRightGrab = 0;
	double PinchTimeout = 100000; //.1 Second
	double GrabTimeout = 100000; //.1 Second
	bool IsLeftPinching = false;
	bool IsRightPinching = false;
	bool IsLeftGrabbing = false;
	bool IsRightGrabbing = false;
	//Visibility Tracking and Thresholds
	bool IsLeftVisible = false;
	bool IsRightVisible = false;
	int64_t TimeSinceLastLeftVisible = 10000;
	int64_t TimeSinceLastRightVisible = 10000;
	int64_t VisibilityTimeout = 1000000; //1 Second
	int64_t LastLeapTime = 0;
	FLeapHandData LastLeftHand;
	FLeapHandData LastRightHand;

	//Private UProperties
	void ClearReferences(UObject* object);

	TArray<ULeapComponent*> EventDelegates;		//delegate storage

	//Private utility methods
	void CallFunctionOnComponents(TFunction< void(ULeapComponent*)> InFunction);	//lambda multi-cast convenience wrapper
	bool EmitKeyUpEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitKeyDownEventForKey(FKey Key, int32 User, bool Repeat);
	bool EmitAnalogInputEventForKey(FKey Key, float Value, int32 User, bool Repeat);
	bool HandClosed(float Strength);
	bool HandPinched(float Strength);
	void CheckHandVisibility();
	void CheckPinchGesture();
	void CheckGrabGesture();

	int64 GetInterpolatedNow();

	//Internal states
	FLeapOptions Options;
	FLeapStats Stats;

	//Interpolation time offsets
	int64 HandInterpolationTimeOffset;		//in microseconds
	int64 FingerInterpolationTimeOffset;	//in microseconds

	//HMD
	FName HMDType;

	//Timewarp
	float TimewarpTween;
	int64 TimeWarpTimeStamp;
	float SlerpTween;

	//Frame timing
	LeapUtilityTimer FrameTimer;
	double GameTimeInSec;
	int64 FrameTimeInMicros;
	
	//Game thread Data
	FLeapFrameData CurrentFrame;
	FLeapFrameData PastFrame;
	
	TArray<FString> AttachedDevices;
	TArray<int32> VisibleHands;
	TArray<int32> PastVisibleHands;

	//Time warp support
	BSHMDSnapshotHandler SnapshotHandler;

	//Image handling
	TSharedPtr<FLeapImage> LeapImageHandler;
	void OnImageCallback(UTexture2D* LeftCapturedTexture, UTexture2D* RightCapturedTexture);

	//SVE
	bool bSceneViewExtensionSet;

	//Wrapper link
	FLeapWrapper Leap;

	//LeapWrapper Callbacks
	virtual void OnConnect() override;
	virtual void OnConnectionLost() override;
	virtual void OnDeviceFound(const LEAP_DEVICE_INFO *props) override;
	virtual void OnDeviceLost(const char* serial) override;
	virtual void OnDeviceFailure(
		const eLeapDeviceStatus failure_code,
		const LEAP_DEVICE failed_device) override;
	virtual void OnFrame(const LEAP_TRACKING_EVENT *frame) override;
	virtual void OnImage(const LEAP_IMAGE_EVENT *image_event) override;
	virtual void OnPolicy(const uint32_t current_policies) override;
	virtual void OnLog(
		const eLeapLogSeverity severity,
		const int64_t timestamp,
		const char* message) override;

	//Bodystate link
	int32 BodyStateDeviceId;
	FBodyStateDeviceConfig Config;

#if !UE_BUILD_SHIPPING
	//LiveLink
	TSharedPtr<FLeapLiveLinkProducer> LiveLink;
#endif

	//Convenience Converters - Todo: wrap into separate class?
	void SetBSFingerFromLeapDigit(class UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit);
	void SetBSThumbFromLeapThumb(class UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit);
	void SetBSHandFromLeapHand(class UBodyStateHand* Hand, const FLeapHandData& LeapHand);
};