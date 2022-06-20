/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapTrackingInputDevice.h"

#include "BodyStateBPLibrary.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "IBodyState.h"
#include "IXRTrackingSystem.h"
#include "LeapAsync.h"
#include "LeapComponent.h"
#include "LeapUtility.h"
#include "Skeleton/BodyStateSkeleton.h"
#include "UltraleapTrackingData.h"

DECLARE_STATS_GROUP(TEXT("UltraleapTracking"), STATGROUP_UltraleapTracking, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Leap Game Input and Events"), STAT_LeapInputTick, STATGROUP_UltraleapTracking);
DECLARE_CYCLE_STAT(TEXT("Leap BodyState Tick"), STAT_LeapBodyStateTick, STATGROUP_UltraleapTracking);

#pragma region Utility
// Function call Utility
void FUltraleapTrackingInputDevice::CallFunctionOnComponents(TFunction<void(ULeapComponent*)> InFunction)
{
	// Callback optimization
	if (EventDelegates.Num() <= 0)
	{
		return;
	}

	if (IsInGameThread())
	{
		for (ULeapComponent* EventDelegate : EventDelegates)
		{
			InFunction(EventDelegate);
		}
	}
	else
	{
		FLeapAsync::RunShortLambdaOnGameThread([this, InFunction] {
			for (ULeapComponent* EventDelegate : EventDelegates)
			{
				InFunction(EventDelegate);
			}
		});
	}
}

// UE v4.6 IM event wrappers
bool FUltraleapTrackingInputDevice::EmitKeyUpEventForKey(FKey Key, int32 User = 0, bool Repeat = false)
{
	if (IsInGameThread())
	{
		FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
		return FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
	}
	return false;
}

bool FUltraleapTrackingInputDevice::EmitKeyDownEventForKey(FKey Key, int32 User = 0, bool Repeat = false)
{
	if (IsInGameThread())
	{
		FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
		return FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
	}
	return false;
}

bool FUltraleapTrackingInputDevice::EmitAnalogInputEventForKey(FKey Key, float Value, int32 User = 0, bool Repeat = false)
{
	if (IsInGameThread())
	{
		FAnalogInputEvent AnalogInputEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0, Value);
		return FSlateApplication::Get().ProcessAnalogInputEvent(AnalogInputEvent);
	}
	return false;
}

// comes from service message loop
void FUltraleapTrackingInputDevice::OnConnect()
{
	FLeapAsync::RunShortLambdaOnGameThread([&] {
		UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapService: OnConnect."));

		IsWaitingForConnect = false;

		//SetOptions(Options);

		CallFunctionOnComponents([&](ULeapComponent* Component) { Component->OnLeapServiceConnected.Broadcast(); });
	});
}
// comes from service message loop
void FUltraleapTrackingInputDevice::OnConnectionLost()
{
	FLeapAsync::RunShortLambdaOnGameThread([&] {
		UE_LOG(UltraleapTrackingLog, Warning, TEXT("LeapService: OnConnectionLost."));

		CallFunctionOnComponents([&](ULeapComponent* Component) { Component->OnLeapServiceDisconnected.Broadcast(); });
	});
}
// already proxied onto game thread in wrapper
void FUltraleapTrackingInputDevice::OnDeviceFound(const LEAP_DEVICE_INFO* Props)
{
	/* Stats.DeviceInfo.SetFromLeapDevice((_LEAP_DEVICE_INFO*) Props);
	SetOptions(Options);

	if (LeapImageHandler)
	{
		LeapImageHandler->Reset();
	}*/
	UE_LOG(UltraleapTrackingLog, Log, TEXT("OnDeviceFound %s %s."), *Stats.DeviceInfo.PID, *Stats.DeviceInfo.Serial);

	AttachedDevices.AddUnique(Stats.DeviceInfo.Serial);

	CallFunctionOnComponents(
		[&](ULeapComponent* Component) { Component->OnLeapDeviceAttached.Broadcast(Stats.DeviceInfo.Serial); });
}
// comes from service message loop
void FUltraleapTrackingInputDevice::OnDeviceLost(const char* Serial)
{
	const FString SerialString = FString(ANSI_TO_TCHAR(Serial));

	FLeapAsync::RunShortLambdaOnGameThread([&, SerialString] {
		UE_LOG(UltraleapTrackingLog, Warning, TEXT("OnDeviceLost %s."), *SerialString);

		AttachedDevices.Remove(SerialString);

		CallFunctionOnComponents(
			[SerialString](ULeapComponent* Component) { Component->OnLeapDeviceDetached.Broadcast(SerialString); });
	});
}

void FUltraleapTrackingInputDevice::OnDeviceFailure(const eLeapDeviceStatus FailureCode, const LEAP_DEVICE FailedDevice)
{
	FString ErrorString;
	switch (FailureCode)
	{
		case eLeapDeviceStatus_Streaming:
			ErrorString = TEXT("Streaming");
			break;
		case eLeapDeviceStatus_Paused:
			ErrorString = TEXT("Paused");
			break;
		case eLeapDeviceStatus_Robust:
			ErrorString = TEXT("Robust");
			break;
		case eLeapDeviceStatus_Smudged:
			ErrorString = TEXT("Smudged");
			break;
		case eLeapDeviceStatus_BadCalibration:
			ErrorString = TEXT("Bad Calibration");
			break;
		case eLeapDeviceStatus_BadFirmware:
			ErrorString = TEXT("Bad Firmware");
			break;
		case eLeapDeviceStatus_BadTransport:
			ErrorString = TEXT("Bad Transport");
			break;
		case eLeapDeviceStatus_BadControl:
			ErrorString = TEXT("Bad Control");
			break;
		case eLeapDeviceStatus_UnknownFailure:
		default:
			ErrorString = TEXT("Unknown");
			break;
	}
	UE_LOG(UltraleapTrackingLog, Warning, TEXT("OnDeviceFailure: %s (%d)"), *ErrorString, (int32) FailureCode);
}


#pragma endregion Utility

#pragma region Leap Input Device

#define LOCTEXT_NAMESPACE "UltraleapTracking"
#define START_IN_OPEN_XR_MODE 0
FUltraleapTrackingInputDevice::FUltraleapTrackingInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler), Leap(nullptr), Connector(nullptr)
{
	// Set static stats
	Stats.LeapAPIVersion = FString(TEXT("4.0.1"));

	InitTrackingSource(START_IN_OPEN_XR_MODE);
	

	// Multi-device note: attach multiple devices and get another ID?
	// Origin will be different if mixing vr with desktop/mount

	// Add IM keys
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchL, LOCTEXT("LeapPinchL", "Leap (L) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabL, LOCTEXT("LeapGrabL", "Leap (L) Grab"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchR, LOCTEXT("LeapPinchR", "Leap (R) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabR, LOCTEXT("LeapGrabR", "Leap (R) Grab"), FKeyDetails::GamepadKey));
}

#undef LOCTEXT_NAMESPACE
void FUltraleapTrackingInputDevice::PostEarlyInit()
{
}
FUltraleapTrackingInputDevice::~FUltraleapTrackingInputDevice()
{
	ShutdownLeap();
}

void FUltraleapTrackingInputDevice::Tick(float DeltaTime)
{
	// TODO: tick any attached devices from here
}

// Main loop event emitter
void FUltraleapTrackingInputDevice::SendControllerEvents()
{
	// TODO: proxy to attached device's CaptureAndEvaluate
}

void FUltraleapTrackingInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}


// Multidevice note, this will receive global events such as device add/remove, connected, disconnected to service
// Device specific events will be subscribed to from FUltraleapDevice
void FUltraleapTrackingInputDevice::AddEventDelegate(const ULeapComponent* EventDelegate)
{
	UWorld* ComponentWorld = nullptr;
	if (EventDelegate->GetOwner())
	{
		ComponentWorld = EventDelegate->GetOwner()->GetWorld();
	}
	if (ComponentWorld == nullptr)
	{
		// editor mirror component
		if (EventDelegate != nullptr && EventDelegate->IsValidLowLevel())
		{
			EventDelegates.Add((ULeapComponent*) EventDelegate);
		}
		return;
	}
	// needed for world time
	Leap->SetWorld(ComponentWorld);
	// only add delegates to world
	if (ComponentWorld->WorldType == EWorldType::Game || ComponentWorld->WorldType == EWorldType::GamePreview ||
		ComponentWorld->WorldType == EWorldType::PIE || ComponentWorld->WorldType == EWorldType::EditorPreview)
	{
		if (EventDelegate != nullptr && EventDelegate->IsValidLowLevel())
		{
			EventDelegates.Add((ULeapComponent*) EventDelegate);
		}

		UE_LOG(UltraleapTrackingLog, Log, TEXT("AddEventDelegate (%d)."), EventDelegates.Num());
	}
}

void FUltraleapTrackingInputDevice::RemoveEventDelegate(const ULeapComponent* EventDelegate)
{
	EventDelegates.Remove((ULeapComponent*) EventDelegate);
	// UE_LOG(UltraleapTrackingLog, Log, TEXT("RemoveEventDelegate (%d)."),
	// EventDelegates.Num());
}

void FUltraleapTrackingInputDevice::ShutdownLeap()
{
	if (Leap != nullptr)
	{
		// This will kill the leap thread
		Leap->CloseConnection();
	}
}

void FUltraleapTrackingInputDevice::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible)
{
	//TODO: add by device ID or first device
//	LeftHandIsVisible = CurrentFrame.LeftHandVisible;
//	RightHandIsVisible = CurrentFrame.RightHandVisible;
}

void FUltraleapTrackingInputDevice::LatestFrame(FLeapFrameData& OutFrame)
{
	// TODO: add by device ID or first device
	//OutFrame = CurrentFrame;
}
void FUltraleapTrackingInputDevice::SetSwizzles(
	ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW)
{
	// TODO: add by device ID or first device
	Leap->SetSwizzles(ToX, ToY, ToZ, ToW);
}
// Policies
void FUltraleapTrackingInputDevice::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	// TODO: add by device ID or first device
	switch (Flag)
	{
		case LEAP_POLICY_BACKGROUND_FRAMES:
			Leap->SetPolicyFlagFromBoolean(eLeapPolicyFlag_BackgroundFrames, Enable);
			break;
		case LEAP_POLICY_IMAGES:
			Leap->SetPolicyFlagFromBoolean(eLeapPolicyFlag_Images, Enable);
			break;
		// legacy 3.0 implementation superseded by SetTrackingMode
		case LEAP_POLICY_OPTIMIZE_HMD:
			Leap->SetPolicyFlagFromBoolean(eLeapPolicyFlag_OptimizeHMD, Enable);
			break;
		case LEAP_POLICY_ALLOW_PAUSE_RESUME:
			Leap->SetPolicyFlagFromBoolean(eLeapPolicyFlag_AllowPauseResume, Enable);
			break;
		case LEAP_POLICY_MAP_POINTS:
			Leap->SetPolicyFlagFromBoolean(eLeapPolicyFlag_MapPoints, Enable);
		default:
			break;
	}
}
// v5 implementation of tracking mode
void FUltraleapTrackingInputDevice::SetTrackingMode(ELeapMode Flag)
{
	// TODO: add by device ID or first device
	switch (Flag)
	{
		case LEAP_MODE_DESKTOP:
			Leap->SetTrackingMode(eLeapTrackingMode_Desktop);
			break;
		case LEAP_MODE_VR:
			Leap->SetTrackingMode(eLeapTrackingMode_HMD);
			break;
		case LEAP_MODE_SCREENTOP:
			Leap->SetTrackingMode(eLeapTrackingMode_ScreenTop);
			break;
	}
}
#pragma endregion Leap Input Device

#pragma region BodyState


void FUltraleapTrackingInputDevice::OnDeviceDetach()
{
	ShutdownLeap();
	UE_LOG(UltraleapTrackingLog, Log, TEXT("OnDeviceDetach call from BodyState."));
}

#pragma endregion BodyState
void FUltraleapTrackingInputDevice::InitTrackingSource(const bool UseOpenXRAsSource)
{
	if (IsWaitingForConnect)
	{
		UE_LOG(UltraleapTrackingLog, Warning,
			TEXT("FUltraleapTrackingInputDevice::SwitchTrackingSource switch attempted whilst async connect in progress"));
	}
	if (Leap != nullptr)
	{
		Leap->CloseConnection();
	}

	if (UseOpenXRAsSource)
	{
		Leap = TSharedPtr<IHandTrackingWrapper>(new FOpenXRToLeapWrapper);
	}
	else
	{
		FLeapWrapper* Wrapper = new FLeapWrapper;
		Connector = dynamic_cast<ILeapConnector*>(Wrapper);
		Leap = TSharedPtr<IHandTrackingWrapper>(Wrapper);
	}
	if (!UseOpenXRAsSource)
	{
		IsWaitingForConnect = true;
	}
	static const bool UseMultiDevice = false;
	Leap->OpenConnection(this, UseMultiDevice);
}
void FUltraleapTrackingInputDevice::SetOptions(const FLeapOptions& InOptions)
{
	// TODO: proxy by device ID
}
FLeapOptions FUltraleapTrackingInputDevice::GetOptions()
{
	//TODO: proxy by device ID
	FLeapOptions Options;
	return Options;
}

FLeapStats FUltraleapTrackingInputDevice::GetStats()
{
	// TODO: proxy by device ID
	return Stats;
}

#pragma endregion Leap Input Device
