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

#define START_IN_OPENXR 0

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
	
	UE_LOG(UltraleapTrackingLog, Log, TEXT("OnDeviceFound PID %x DeviceSerial %s."), (int32)Props->pid, *FString(Props->serial));

	AttachedDevices.AddUnique(FString(Props->serial));

	CallFunctionOnComponents(
		[&](ULeapComponent* Component) { Component->OnLeapDeviceAttached.Broadcast(FString(Props->serial)); });
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

FUltraleapTrackingInputDevice::FUltraleapTrackingInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler), Leap(nullptr), Connector(nullptr)
{
	InitTrackingSource();
	

	// Multi-device note: attach multiple devices and get another ID?
	// Origin will be different if mixing vr with desktop/mount

	// Add IM keys
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchL, LOCTEXT("LeapPinchL", "Leap (L) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabL, LOCTEXT("LeapGrabL", "Leap (L) Grab"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchR, LOCTEXT("LeapPinchR", "Leap (R) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabR, LOCTEXT("LeapGrabR", "Leap (R) Grab"), FKeyDetails::GamepadKey));

	IBodyState::Get().SetupGlobalDeviceManager(this);
	FLeapUtility::InitLeapStatics();

	// fallback device for non multileap defaults to 
	// open XR based on this device
	// this can also be switched at runtime
	IsInOpenXRMode = START_IN_OPENXR;
}

#undef LOCTEXT_NAMESPACE
void FUltraleapTrackingInputDevice::PostEarlyInit()
{
	if (Connector)
	{
		Connector->PostEarlyInit();
	}
}
FUltraleapTrackingInputDevice::~FUltraleapTrackingInputDevice()
{
	IBodyState::Get().SetupGlobalDeviceManager(nullptr);
	ShutdownLeap();
}

void FUltraleapTrackingInputDevice::Tick(float DeltaTime)
{
	if (Connector)
	{
		Connector->TickDevices(DeltaTime);
	}
}

// Main loop event emitter
void FUltraleapTrackingInputDevice::SendControllerEvents()
{
	if (Connector)
	{
		Connector->TickSendControllerEventsOnDevices();
	}
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
			EventDelegates.AddUnique((ULeapComponent*) EventDelegate);
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
			EventDelegates.AddUnique((ULeapComponent*) EventDelegate);
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
IHandTrackingDevice* FUltraleapTrackingInputDevice::GetDeviceBySerial(const FString& DeviceSerial)
{
	if (!Connector)
	{
		return nullptr;
	}

	TArray<FString> DeviceList;

	// leave list empty if no device serial (backwards compatibilty fallback to default device)
	if (!DeviceSerial.IsEmpty())
	{
		DeviceList.Add(DeviceSerial);
	}
	auto DeviceWrapper = Connector->GetDevice(DeviceList, ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_UNKNOWN, IsInOpenXRMode);
	if (DeviceWrapper)
	{
		auto InternalDevice = DeviceWrapper->GetDevice();
		if (InternalDevice)
		{
			return InternalDevice;
		}
	}
	return nullptr;
}
IHandTrackingWrapper* FUltraleapTrackingInputDevice::GetDeviceWrapperBySerial(const FString& DeviceSerial)
{
	if (!Connector)
	{
		return nullptr;
	}
	TArray<FString> DeviceList;
	// leave list empty if no device serial (backwards compatibilty fallback to default device)
	if (!DeviceSerial.IsEmpty())
	{
		DeviceList.Add(DeviceSerial);
	}

	return Connector->GetDevice(DeviceList, ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_UNKNOWN, IsInOpenXRMode);
}
// get default device for backwards compatibility
IHandTrackingWrapper* FUltraleapTrackingInputDevice::GetFallbackDeviceWrapper()
{
	if (!Connector)
	{
		return nullptr;
	}
	// empty device list means 'give me the default'
	TArray<FString> DeviceList;
	return Connector->GetDevice(DeviceList, ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_UNKNOWN, IsInOpenXRMode);
}
void FUltraleapTrackingInputDevice::AreHandsVisible(
		bool& LeftHandIsVisible, bool& RightHandIsVisible, const FString& DeviceSerial)
{
	auto InternalDevice = GetDeviceBySerial(DeviceSerial);
	if (InternalDevice)
	{
		InternalDevice->AreHandsVisible(LeftHandIsVisible, RightHandIsVisible);
	}
}

void FUltraleapTrackingInputDevice::LatestFrame(FLeapFrameData& OutFrame, const FString& DeviceSerial)
{
	auto InternalDevice = GetDeviceBySerial(DeviceSerial);
	if (InternalDevice)
	{
		InternalDevice->GetLatestFrameData(OutFrame);
	}
}
void FUltraleapTrackingInputDevice::SetSwizzles(ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ,
	ELeapQuatSwizzleAxisB ToW, const TArray<FString>& DeviceSerials)
{
	auto DeviceWrapper =
		Connector->GetDevice(DeviceSerials, ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_UNKNOWN, IsInOpenXRMode);
	if (DeviceWrapper)
	{
		DeviceWrapper->SetSwizzles(ToX, ToY, ToZ, ToW);
	}
}
// Policies
void FUltraleapTrackingInputDevice::SetLeapPolicyBySerial(ELeapPolicyFlag Flag, bool Enable, const FString& DeviceSerial)
{
	IHandTrackingWrapper* DeviceWrapper = nullptr;

	if (DeviceSerial.IsEmpty())
	{
		DeviceWrapper = GetFallbackDeviceWrapper();
	}
	else
	{
		DeviceWrapper = GetDeviceWrapperBySerial(DeviceSerial);
	}
	if (DeviceWrapper)
	{
		switch (Flag)
		{
			case LEAP_POLICY_BACKGROUND_FRAMES:
				DeviceWrapper->SetPolicyFlagFromBoolean(eLeapPolicyFlag_BackgroundFrames, Enable);
				break;
			case LEAP_POLICY_IMAGES:
				DeviceWrapper->SetPolicyFlagFromBoolean(eLeapPolicyFlag_Images, Enable);
				break;
			// legacy 3.0 implementation superseded by SetTrackingMode
			case LEAP_POLICY_OPTIMIZE_HMD:
				DeviceWrapper->SetPolicyFlagFromBoolean(eLeapPolicyFlag_OptimizeHMD, Enable);
				break;
			case LEAP_POLICY_ALLOW_PAUSE_RESUME:
				DeviceWrapper->SetPolicyFlagFromBoolean(eLeapPolicyFlag_AllowPauseResume, Enable);
				break;
			case LEAP_POLICY_MAP_POINTS:
				DeviceWrapper->SetPolicyFlagFromBoolean(eLeapPolicyFlag_MapPoints, Enable);
			default:
				break;
		}
	}
}
void FUltraleapTrackingInputDevice::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable, const TArray<FString>& DeviceSerials)
{
	if (DeviceSerials.Num() == 0)
	{
		SetLeapPolicyBySerial(Flag, Enable, "");
	}
	for (auto DeviceSerial : DeviceSerials)
	{
		SetLeapPolicyBySerial(Flag, Enable, DeviceSerial);
	}

}
void FUltraleapTrackingInputDevice::SetTrackingModeBySerial(ELeapMode Flag, const FString& DeviceSerial)
{
	IHandTrackingWrapper* DeviceWrapper = nullptr;
	
	if (DeviceSerial.IsEmpty())
	{
		DeviceWrapper = GetFallbackDeviceWrapper();
	}
	else
	{
		DeviceWrapper = GetDeviceWrapperBySerial(DeviceSerial);
	}
	if (DeviceWrapper)
	{
		switch (Flag)
		{
			case LEAP_MODE_DESKTOP:
				DeviceWrapper->SetTrackingMode(eLeapTrackingMode_Desktop);
				break;
			case LEAP_MODE_VR:
				DeviceWrapper->SetTrackingMode(eLeapTrackingMode_HMD);
				break;
			case LEAP_MODE_SCREENTOP:
				DeviceWrapper->SetTrackingMode(eLeapTrackingMode_ScreenTop);
				break;
		}
	}
}
	// v5 implementation of tracking mode
void FUltraleapTrackingInputDevice::SetTrackingMode(ELeapMode Flag, const TArray<FString>& DeviceSerials)
{
	TArray<FString> DeviceSerialsToSet = DeviceSerials;
	// backwards compatibility
	if (DeviceSerials.Num() == 0)
	{
		SetTrackingModeBySerial(Flag, FString(""));
	}
	for (auto DeviceSerial : DeviceSerials)
	{
		SetTrackingModeBySerial(Flag,DeviceSerial);
	}
}
#pragma endregion Leap Input Device

#pragma region BodyState

int32 FUltraleapTrackingInputDevice::RequestCombinedDevice(
	const TArray<FString>& DeviceSerials, const EBSDeviceCombinerClass CombinerClass)
{
	if (Connector == nullptr)
	{
		return -1;
	}
	ELeapDeviceCombinerClass LeapCombinerClass = ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_UNKNOWN;
	switch (CombinerClass)
	{
		case EBSDeviceCombinerClass::BS_DEVICE_COMBINER_CONFIDENCE:
			LeapCombinerClass = ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_CONFIDENCE;
			break;
		case EBSDeviceCombinerClass::BS_DEVICE_COMBINER_ANGULAR:
			LeapCombinerClass = ELeapDeviceCombinerClass::LEAP_DEVICE_COMBINER_ANGULAR;
			break;
	}
	auto DeviceWrapper = Connector->GetDevice(DeviceSerials, LeapCombinerClass, IsInOpenXRMode);
	if (DeviceWrapper)
	{
		auto InternalDevice = DeviceWrapper->GetDevice();
		if (InternalDevice)
		{
			return InternalDevice->GetBodyStateDeviceID();
		}
	}
	return -1;
}
int32 FUltraleapTrackingInputDevice::GetDefaultDeviceID()
{
	// this could be either the first device found
	// or the OpenXR device depending on global options
	auto DeviceWrapper = GetFallbackDeviceWrapper();
	if (DeviceWrapper)
	{
		auto Device = DeviceWrapper->GetDevice();
		if (Device)
		{
			return Device->GetBodyStateDeviceID();
		}
	}
	return 0;
}
void FUltraleapTrackingInputDevice::OnDeviceDetach()
{
	ShutdownLeap();
	UE_LOG(UltraleapTrackingLog, Log, TEXT("OnDeviceDetach call from BodyState."));
}

#pragma endregion BodyState
void FUltraleapTrackingInputDevice::InitTrackingSource()
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

	
	FLeapWrapper* Wrapper = new FLeapWrapper;
	Connector = dynamic_cast<ILeapConnector*>(Wrapper);
	Leap = TSharedPtr<IHandTrackingWrapper>(Wrapper);
	
	IsWaitingForConnect = true;
	
	static const bool UseMultiDevice = true;
	Leap->OpenConnection(this, UseMultiDevice);
}
void FUltraleapTrackingInputDevice::SetOptions(const FLeapOptions& InOptions, const TArray<FString>& DeviceSerials)
{
	// backwards compatibility
	if (DeviceSerials.Num() == 0)
	{
		// if setting global options, check for switch to OpenXR
		// as in this case the fallback device will change
		const bool OpenXRModeChanged = InOptions.bUseOpenXRAsSource != IsInOpenXRMode;

		IsInOpenXRMode = InOptions.bUseOpenXRAsSource;

		auto DeviceWrapper = GetFallbackDeviceWrapper();
		if (DeviceWrapper)
		{
			auto Device = DeviceWrapper->GetDevice();
			if (Device)
			{
				Device->SetOptions(InOptions);
			}
		}
		// notify as the default device has changed
		// so bodystate needs to refresh which skeleton it's listening to
		if (OpenXRModeChanged)
		{
			UBodyStateBPLibrary::OnDefaultDeviceChanged();
		}
	}
	for (auto DeviceSerial : DeviceSerials)
	{
		IHandTrackingDevice* Device = GetDeviceBySerial(DeviceSerial);
		Device->SetOptions(InOptions);
	}
}
FLeapOptions FUltraleapTrackingInputDevice::GetOptions(const FString& DeviceSerial)
{
	IHandTrackingDevice* Device = GetDeviceBySerial(DeviceSerial);
	if (Device)
	{
		return Device->GetOptions();
	}
	FLeapOptions Options;
	return Options;
}

FLeapStats FUltraleapTrackingInputDevice::GetStats(const FString& DeviceSerial)
{
	IHandTrackingDevice* Device = GetDeviceBySerial(DeviceSerial);
	if (Device)
	{
		return Device->GetStats();
	}
	FLeapStats Stats;
	return Stats;
}

#pragma endregion Leap Input Device
