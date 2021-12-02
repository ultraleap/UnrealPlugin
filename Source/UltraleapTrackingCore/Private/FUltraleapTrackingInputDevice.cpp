

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
bool FUltraleapTrackingInputDevice::bUseNewTrackingModeAPI = true;
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

// Utility

const FKey EKeysLeap::LeapPinchL("LeapPinchL");
const FKey EKeysLeap::LeapGrabL("LeapGrabL");
const FKey EKeysLeap::LeapPinchR("LeapPinchR");
const FKey EKeysLeap::LeapGrabR("LeapGrabR");

bool FUltraleapTrackingInputDevice::HandClosed(float Strength)
{
	return (Strength == 1.f);
}

bool FUltraleapTrackingInputDevice::HandPinched(float Strength)
{
	return (Strength > 0.8);
}

int64 FUltraleapTrackingInputDevice::GetInterpolatedNow()
{
	return Leap->GetNow() + HandInterpolationTimeOffset;
}

// comes from service message loop
void FUltraleapTrackingInputDevice::OnConnect()
{
	FLeapAsync::RunShortLambdaOnGameThread([&] {
		UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapService: OnConnect."));

		IsWaitingForConnect = false;

		SetOptions(Options);

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
	Stats.DeviceInfo.SetFromLeapDevice((_LEAP_DEVICE_INFO*) Props);
	SetOptions(Options);

	if (LeapImageHandler)
	{
		LeapImageHandler->Reset();
	}
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
			[SerialString](ULeapComponent* Component) { Component->OnLeapDeviceDetatched.Broadcast(SerialString); });
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

void FUltraleapTrackingInputDevice::OnFrame(const LEAP_TRACKING_EVENT* Frame)
{
	// Not used. Polled on separate thread for lower latency.
}

void FUltraleapTrackingInputDevice::OnImage(const LEAP_IMAGE_EVENT* ImageEvent)
{
	// Forward it to the handler
	LeapImageHandler->OnImage(ImageEvent);
}

void FUltraleapTrackingInputDevice::OnImageCallback(UTexture2D* LeftCapturedTexture, UTexture2D* RightCapturedTexture)
{
	// Handler has returned with batched easy-to-parse results, forward callback
	// on game thread
	CallFunctionOnComponents([LeftCapturedTexture, RightCapturedTexture](ULeapComponent* Component) {
		Component->OnImageEvent.Broadcast(LeftCapturedTexture, ELeapImageType::LEAP_IMAGE_LEFT);
		Component->OnImageEvent.Broadcast(RightCapturedTexture, ELeapImageType::LEAP_IMAGE_RIGHT);
	});
}

void FUltraleapTrackingInputDevice::OnPolicy(const uint32_t CurrentPolicies)
{
	TArray<TEnumAsByte<ELeapPolicyFlag>> Flags;
	ELeapMode UpdatedMode = Options.Mode;
	if (CurrentPolicies & eLeapPolicyFlag_BackgroundFrames)
	{
		Flags.Add(ELeapPolicyFlag::LEAP_POLICY_BACKGROUND_FRAMES);
	}
	if (CurrentPolicies & eLeapPolicyFlag_OptimizeHMD)
	{
		UpdatedMode = ELeapMode::LEAP_MODE_VR;
		Flags.Add(ELeapPolicyFlag::LEAP_POLICY_OPTIMIZE_HMD);
	}
	if (CurrentPolicies & eLeapPolicyFlag_AllowPauseResume)
	{
		Flags.Add(ELeapPolicyFlag::LEAP_POLICY_ALLOW_PAUSE_RESUME);
	}

	Options.Mode = UpdatedMode;

	// Update mode for each component and broadcast current policies
	CallFunctionOnComponents([&, UpdatedMode, Flags](ULeapComponent* Component) {
		Component->TrackingMode = UpdatedMode;
		Component->OnLeapPoliciesUpdated.Broadcast(Flags);
	});
}
void FUltraleapTrackingInputDevice::OnTrackingMode(const eLeapTrackingMode CurrentMode)
{
	switch (CurrentMode)
	{
		case eLeapTrackingMode_Desktop:
			Options.Mode = LEAP_MODE_DESKTOP;
			break;
		case eLeapTrackingMode_HMD:
			Options.Mode = LEAP_MODE_VR;
			break;
		case eLeapTrackingMode_ScreenTop:
			Options.Mode = LEAP_MODE_SCREENTOP;
			break;
	}
	ELeapMode UpdatedMode = Options.Mode;
	// Update mode for each component and broadcast current policies
	CallFunctionOnComponents([&, UpdatedMode](ULeapComponent* Component) {
		Component->TrackingMode = UpdatedMode;
		Component->OnLeapTrackingModeUpdated.Broadcast(UpdatedMode);
	});
}
void FUltraleapTrackingInputDevice::OnLog(const eLeapLogSeverity Severity, const int64_t Timestamp, const char* Message)
{
	if (!Message)
	{
		return;
	}

	switch (Severity)
	{
		case eLeapLogSeverity_Unknown:
			break;
		case eLeapLogSeverity_Critical:
			if (Options.LeapServiceLogLevel > ELeapServiceLogLevel::LEAP_LOG_NONE)
			{
				UE_LOG(UltraleapTrackingLog, Error, TEXT("LeapServiceError: %s"), UTF8_TO_TCHAR(Message));
			}
			break;
		case eLeapLogSeverity_Warning:
			if (Options.LeapServiceLogLevel > ELeapServiceLogLevel::LEAP_LOG_ERROR)
			{
				UE_LOG(UltraleapTrackingLog, Warning, TEXT("LeapServiceWarning: %s"), UTF8_TO_TCHAR(Message));
			}
			break;
		case eLeapLogSeverity_Information:
			if (Options.LeapServiceLogLevel > ELeapServiceLogLevel::LEAP_LOG_WARNING)
			{
				UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapServiceLog: %s"), UTF8_TO_TCHAR(Message));
			}
			break;
		default:
			break;
	}
}

#pragma endregion Utility

#pragma region Leap Input Device

#define LOCTEXT_NAMESPACE "UltraleapTracking"
#define START_IN_OPEN_XR_MODE 0
FUltraleapTrackingInputDevice::FUltraleapTrackingInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler), Leap(nullptr)
{
	// Link callbacks

	// Open the connection
	TimewarpTween = 0.5f;
	SlerpTween = 0.5f;
	GameTimeInSec = 0.f;
	HMDType = TEXT("SteamVR");
	FrameTimeInMicros = 0;	  // default

	// Set static stats
	Stats.LeapAPIVersion = FString(TEXT("4.0.1"));

	SwitchTrackingSource(START_IN_OPEN_XR_MODE);
	Options.bUseOpenXRAsSource = START_IN_OPEN_XR_MODE;

	// Attach to bodystate
	Config.DeviceName = "Leap Motion";
	Config.InputType = EBodyStateDeviceInputType::HMD_MOUNTED_INPUT_TYPE;
	Config.TrackingTags.Add("Hands");
	Config.TrackingTags.Add("Fingers");
	BodyStateDeviceId = UBodyStateBPLibrary::AttachDeviceNative(Config, this);

	// Multi-device note: attach multiple devices and get another ID?
	// Origin will be different if mixing vr with desktop/mount

	// Add IM keys
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchL, LOCTEXT("LeapPinchL", "Leap (L) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabL, LOCTEXT("LeapGrabL", "Leap (L) Grab"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchR, LOCTEXT("LeapPinchR", "Leap (R) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabR, LOCTEXT("LeapGrabR", "Leap (R) Grab"), FKeyDetails::GamepadKey));

#if WITH_EDITOR
	// LiveLink startup
	LiveLink = MakeShareable(new FLeapLiveLinkProducer());
	LiveLink->Startup();
	LiveLink->SyncSubjectToSkeleton(IBodyState::Get().SkeletonForDevice(BodyStateDeviceId));
#endif

	// Image support
	LeapImageHandler = MakeShareable(new FLeapImage);
	LeapImageHandler->OnImageCallback.AddRaw(this, &FUltraleapTrackingInputDevice::OnImageCallback);
}

#undef LOCTEXT_NAMESPACE

FUltraleapTrackingInputDevice::~FUltraleapTrackingInputDevice()
{
#if WITH_EDITOR
	// LiveLink cleanup
	LiveLink->ShutDown();
#endif

	ShutdownLeap();
}

void FUltraleapTrackingInputDevice::Tick(float DeltaTime)
{
	GameTimeInSec += DeltaTime;
	FrameTimeInMicros = DeltaTime * 1000000;
}

// Main loop event emitter
void FUltraleapTrackingInputDevice::SendControllerEvents()
{
	CaptureAndEvaluateInput();
}

void FUltraleapTrackingInputDevice::CaptureAndEvaluateInput()
{
	SCOPE_CYCLE_COUNTER(STAT_LeapInputTick);
	// Did a device connect?
	if (!Leap->IsConnected() || !Leap->GetDeviceProperties())
	{
		return;
	}

	// Todo: get frame and parse for each device

	_LEAP_TRACKING_EVENT* Frame = Leap->GetFrame();

	// Is the frame valid?
	if (!Frame)
	{
		return;
	}
	if (!Options.bUseOpenXRAsSource)
	{
		TimeWarpTimeStamp = Frame->info.timestamp;
		int64 LeapTimeNow = 0;
		LeapTimeNow = Leap->GetNow();
		SnapshotHandler.AddCurrentHMDSample(LeapTimeNow);

		HandInterpolationTimeOffset = Options.HandInterpFactor * FrameTimeInMicros;
		FingerInterpolationTimeOffset = Options.FingerInterpFactor * FrameTimeInMicros;

		// interpolation not supported in OpenXR
		if (Options.bUseInterpolation)
		{
			// Let's interpolate the frame using leap function

			// Get the future interpolated finger frame
			Frame = Leap->GetInterpolatedFrameAtTime(LeapTimeNow + FingerInterpolationTimeOffset);
			CurrentFrame.SetFromLeapFrame(Frame);

			// Get the future interpolated hand frame, farther than fingers to provide
			// lower latency
			Frame = Leap->GetInterpolatedFrameAtTime(LeapTimeNow + HandInterpolationTimeOffset);
			CurrentFrame.SetInterpolationPartialFromLeapFrame(Frame);

			// Track our extrapolation time in stats
			Stats.FrameExtrapolationInMS = (CurrentFrame.TimeStamp - TimeWarpTimeStamp) / 1000.f;
		}
		else
		{
			CurrentFrame.SetFromLeapFrame(Frame);
			Stats.FrameExtrapolationInMS = 0;
		}
	}
	else
	{
		CurrentFrame.SetFromLeapFrame(Frame);
		Stats.FrameExtrapolationInMS = 0;
	}

	ParseEvents();
}

void FUltraleapTrackingInputDevice::ParseEvents()
{
	// Early exit: no device attached that produces data
	if (AttachedDevices.Num() < 1)
	{
		return;
	}

	// Are we in HMD mode? add our HMD snapshot
	// Note with Open XR, the data is already transformed for the HMD/player camera
	if (Options.Mode == LEAP_MODE_VR && Options.bTransformOriginToHMD && !Options.bUseOpenXRAsSource)
	{
		// Correction for HMD offset and rotation has already been applied in call
		// to CaptureAndEvaluateInput through CurrentFrame->SetFromLeapFrame()

		BodyStateHMDSnapshot SnapshotNow = SnapshotHandler.LastHMDSample();

		if (IsInGameThread())
		{
			SnapshotNow = BSHMDSnapshotHandler::CurrentHMDSample(Leap->GetNow());
		}

		FRotator FinalHMDRotation = SnapshotNow.Orientation.Rotator();
		FVector FinalHMDTranslation = SnapshotNow.Position;

		// Determine time-warp, only relevant for VR
		if (Options.bUseTimeWarp)
		{
			// We use fixed timewarp offsets so then is a fixed amount away from now
			// (negative). Positive numbers are invalid for TimewarpOffset
			BodyStateHMDSnapshot SnapshotThen =
				SnapshotHandler.HMDSampleClosestToTimestamp(SnapshotNow.Timestamp - Options.TimewarpOffset);

			BodyStateHMDSnapshot SnapshotDifference = SnapshotNow.Difference(SnapshotThen);

			FRotator WarpRotation = SnapshotDifference.Orientation.Rotator() * Options.TimewarpFactor;
			FVector WarpTranslation = SnapshotDifference.Position * Options.TimewarpFactor;

			FinalHMDTranslation += WarpTranslation;

			FinalHMDRotation = FLeapUtility::CombineRotators(WarpRotation, FinalHMDRotation);
			CurrentFrame.FinalRotationAdjustment = FinalHMDRotation;
		}

		// Rotate our frame by time warp difference
		CurrentFrame.RotateFrame(FinalHMDRotation);
		CurrentFrame.TranslateFrame(FinalHMDTranslation);
	}

	if (LastLeapTime == 0)
		LastLeapTime = Leap->GetNow();

	CheckHandVisibility();
	CheckGrabGesture();
	CheckPinchGesture();

	// Emit tracking data if it is being captured
	CallFunctionOnComponents([this](ULeapComponent* Component) {
		// Scale input?
		// FinalFrameData.ScaleByWorldScale(Component->GetWorld()->GetWorldSettings()->WorldToMeters
		// / 100.f);
		Component->OnLeapTrackingData.Broadcast(CurrentFrame);
	});

	// It's now the past data
	PastFrame = CurrentFrame;
	LastLeapTime = Leap->GetNow();
}

void FUltraleapTrackingInputDevice::CheckHandVisibility()
{
	if (UseTimeBasedVisibilityCheck)
	{
		// Update visible hand list, must happen first
		if (IsLeftVisible)
		{
			TimeSinceLastLeftVisible = TimeSinceLastLeftVisible + (Leap->GetNow() - LastLeapTime);
		}
		if (IsRightVisible)
		{
			TimeSinceLastRightVisible = TimeSinceLastRightVisible + (Leap->GetNow() - LastLeapTime);
		}
		for (auto& Hand : CurrentFrame.Hands)
		{
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				if (CurrentFrame.LeftHandVisible)
				{
					TimeSinceLastLeftVisible = 0;
					LastLeftHand = Hand;
					if (!IsLeftVisible)
					{
						IsLeftVisible = true;
						const bool LeftVisible = true;
						CallFunctionOnComponents([this, LeftVisible](ULeapComponent* Component) {
							Component->OnLeftHandVisibilityChanged.Broadcast(LeftVisible);
						});
						CallFunctionOnComponents(
							[Hand](ULeapComponent* Component) { Component->OnHandBeginTracking.Broadcast(Hand); });
					}
				}
			}
			else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				if (CurrentFrame.RightHandVisible)
				{
					TimeSinceLastRightVisible = 0;
					LastRightHand = Hand;
					if (!IsRightVisible)
					{
						IsRightVisible = true;
						const bool RightVisible = true;
						CallFunctionOnComponents([this, RightVisible](ULeapComponent* Component) {
							Component->OnRightHandVisibilityChanged.Broadcast(RightVisible);
						});
						CallFunctionOnComponents(
							[Hand](ULeapComponent* Component) { Component->OnHandBeginTracking.Broadcast(Hand); });
					}
				}
			}
		}

		// Check if hands should no longer be visible
		if (IsLeftVisible && TimeSinceLastLeftVisible > VisibilityTimeout)
		{
			IsLeftVisible = false;
			const FLeapHandData EndHand = LastLeftHand;
			CallFunctionOnComponents([EndHand](ULeapComponent* Component) { Component->OnHandEndTracking.Broadcast(EndHand); });
			const bool LeftVisible = false;
			CallFunctionOnComponents(
				[this, LeftVisible](ULeapComponent* Component) { Component->OnLeftHandVisibilityChanged.Broadcast(LeftVisible); });
		}
		if (IsRightVisible && TimeSinceLastRightVisible > VisibilityTimeout)
		{
			IsRightVisible = false;
			const FLeapHandData EndHand = LastRightHand;
			CallFunctionOnComponents([EndHand](ULeapComponent* Component) { Component->OnHandEndTracking.Broadcast(EndHand); });
			const bool RightVisible = false;
			CallFunctionOnComponents([this, RightVisible](ULeapComponent* Component) {
				Component->OnRightHandVisibilityChanged.Broadcast(RightVisible);
			});
		}
	}
	else
	{
		// Use old, frame based checking
		// Compare past to present visible hands to determine hand enums.
		//== change can happen when chirality is incorrect and changes
		// Hand end tracking must be called first before we call begin tracking
		// Add each hand to visible hands
		// CurrentFrame.Hands;
		TArray<int32> VisibleHands;
		for (auto& Hand : CurrentFrame.Hands)
		{
			VisibleHands.Add(Hand.Id);
		}
		if (VisibleHands.Num() <= PastVisibleHands.Num())
		{
			for (auto HandId : PastVisibleHands)
			{
				// Not visible anymore? lost hand
				if (!VisibleHands.Contains(HandId))
				{
					const FLeapHandData Hand = PastFrame.HandForId(HandId);
					CallFunctionOnComponents([Hand](ULeapComponent* Component) { Component->OnHandEndTracking.Broadcast(Hand); });
				}
			}
		}

		// Check for hand visibility changes
		if (PastFrame.LeftHandVisible != CurrentFrame.LeftHandVisible)
		{
			const bool LeftVisible = CurrentFrame.LeftHandVisible;
			CallFunctionOnComponents(
				[this, LeftVisible](ULeapComponent* Component) { Component->OnLeftHandVisibilityChanged.Broadcast(LeftVisible); });
		}
		if (PastFrame.RightHandVisible != CurrentFrame.RightHandVisible)
		{
			const bool RightVisible = CurrentFrame.RightHandVisible;
			CallFunctionOnComponents([this, RightVisible](ULeapComponent* Component) {
				Component->OnRightHandVisibilityChanged.Broadcast(RightVisible);
			});
		}

		for (auto& Hand : CurrentFrame.Hands)
		{
			FLeapHandData PastHand;

			// Hand list is tiny, typically 1-3, just enum until you find the matching
			// one
			for (auto& EnumPastHand : PastFrame.Hands)
			{
				if (Hand.Id == EnumPastHand.Id)
				{
					// Same id? same hand
					PastHand = EnumPastHand;
				}
			}

			if (!PastVisibleHands.Contains(Hand.Id))	// or if the hand changed type?
			{
				// New hand
				CallFunctionOnComponents([Hand](ULeapComponent* Component) { Component->OnHandBeginTracking.Broadcast(Hand); });
			}
		}
		PastVisibleHands = VisibleHands;
	}
}

void FUltraleapTrackingInputDevice::CheckPinchGesture()
{
	if (UseTimeBasedGestureCheck)
	{
		if (IsLeftPinching)
		{
			TimeSinceLastLeftPinch = TimeSinceLastLeftPinch + (Leap->GetNow() - LastLeapTime);
		}
		if (IsRightPinching)
		{
			TimeSinceLastRightPinch = TimeSinceLastRightPinch + (Leap->GetNow() - LastLeapTime);
		}
		for (auto& Hand : CurrentFrame.Hands)
		{
			const FLeapHandData& FinalHandData = Hand;
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				if (!IsLeftGrabbing && (!IsLeftPinching && (Hand.PinchStrength > StartPinchThreshold)) ||
					(IsLeftPinching && (Hand.PinchStrength > EndPinchThreshold)))
				{
					TimeSinceLastLeftPinch = 0;
					if (!IsLeftPinching)
					{
						IsLeftPinching = true;
						EmitKeyDownEventForKey(EKeysLeap::LeapPinchL);
						CallFunctionOnComponents(
							[FinalHandData](ULeapComponent* Component) { Component->OnHandPinched.Broadcast(FinalHandData); });
					}
				}
				else if (IsLeftPinching && (TimeSinceLastLeftPinch > PinchTimeout))
				{
					IsLeftPinching = false;
					EmitKeyUpEventForKey(EKeysLeap::LeapPinchL);
					CallFunctionOnComponents(
						[FinalHandData](ULeapComponent* Component) { Component->OnHandUnpinched.Broadcast(FinalHandData); });
				}
			}
			else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				if (!IsRightGrabbing && (!IsRightPinching && (Hand.PinchStrength > StartPinchThreshold)) ||
					(IsRightPinching && (Hand.PinchStrength > EndPinchThreshold)))
				{
					TimeSinceLastRightPinch = 0;
					if (!IsRightPinching)
					{
						IsRightPinching = true;
						EmitKeyDownEventForKey(EKeysLeap::LeapPinchR);
						CallFunctionOnComponents(
							[FinalHandData](ULeapComponent* Component) { Component->OnHandPinched.Broadcast(FinalHandData); });
					}
				}
				else if (IsRightPinching && (TimeSinceLastRightPinch > PinchTimeout))
				{
					IsRightPinching = false;
					EmitKeyUpEventForKey(EKeysLeap::LeapPinchR);
					CallFunctionOnComponents(
						[FinalHandData](ULeapComponent* Component) { Component->OnHandUnpinched.Broadcast(FinalHandData); });
				}
			}
		}
	}
	else
	{
		for (auto& Hand : CurrentFrame.Hands)
		{
			FLeapHandData PastHand;

			// Hand list is tiny, typically 1-3, just enum until you find the matching
			// one
			for (auto& EnumPastHand : PastFrame.Hands)
			{
				if (Hand.Id == EnumPastHand.Id)
				{
					// Same id? same hand
					PastHand = EnumPastHand;
				}
			}

			const FLeapHandData& FinalHandData = Hand;
			// Pinch
			if (Hand.PinchStrength > StartPinchThreshold && PastHand.PinchStrength <= StartPinchThreshold)
			{
				if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
				{
					EmitKeyDownEventForKey(EKeysLeap::LeapPinchL);
				}
				else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
				{
					EmitKeyDownEventForKey(EKeysLeap::LeapPinchR);
				}
				CallFunctionOnComponents(
					[FinalHandData](ULeapComponent* Component) { Component->OnHandPinched.Broadcast(FinalHandData); });
			}
			// Unpinch (TODO: Adjust values)
			else if (Hand.PinchStrength <= EndPinchThreshold && PastHand.PinchStrength > EndPinchThreshold)
			{
				if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
				{
					EmitKeyUpEventForKey(EKeysLeap::LeapPinchL);
				}
				else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
				{
					EmitKeyUpEventForKey(EKeysLeap::LeapPinchR);
				}
				CallFunctionOnComponents(
					[FinalHandData](ULeapComponent* Component) { Component->OnHandUnpinched.Broadcast(FinalHandData); });
			}
		}
	}
}

void FUltraleapTrackingInputDevice::CheckGrabGesture()
{
	if (UseTimeBasedGestureCheck)
	{
		if (IsLeftGrabbing)
		{
			TimeSinceLastLeftGrab = TimeSinceLastLeftGrab + (Leap->GetNow() - LastLeapTime);
		}
		if (IsRightGrabbing)
		{
			TimeSinceLastRightGrab = TimeSinceLastRightGrab + (Leap->GetNow() - LastLeapTime);
		}
		for (auto& Hand : CurrentFrame.Hands)
		{
			const FLeapHandData& FinalHandData = Hand;
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				if ((!IsLeftGrabbing && (Hand.GrabStrength > StartGrabThreshold)) ||
					(IsLeftGrabbing && (Hand.GrabStrength > EndGrabThreshold)))
				{
					TimeSinceLastLeftGrab = 0;
					if (!IsLeftGrabbing)
					{
						IsLeftGrabbing = true;
						EmitKeyDownEventForKey(EKeysLeap::LeapGrabL);
						CallFunctionOnComponents(
							[FinalHandData](ULeapComponent* Component) { Component->OnHandGrabbed.Broadcast(FinalHandData); });
					}
				}
				else if (IsLeftGrabbing && (TimeSinceLastLeftGrab > GrabTimeout))
				{
					IsLeftGrabbing = false;
					EmitKeyUpEventForKey(EKeysLeap::LeapGrabL);
					CallFunctionOnComponents(
						[FinalHandData](ULeapComponent* Component) { Component->OnHandReleased.Broadcast(FinalHandData); });
				}
			}
			else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				if ((!IsRightGrabbing && (Hand.GrabStrength > StartGrabThreshold)) ||
					(IsRightGrabbing && (Hand.GrabStrength > EndGrabThreshold)))
				{
					TimeSinceLastRightGrab = 0;
					if (!IsRightGrabbing)
					{
						IsRightGrabbing = true;
						EmitKeyDownEventForKey(EKeysLeap::LeapGrabR);
						CallFunctionOnComponents(
							[FinalHandData](ULeapComponent* Component) { Component->OnHandGrabbed.Broadcast(FinalHandData); });
					}
				}
				else if (IsRightGrabbing && (TimeSinceLastRightGrab > GrabTimeout))
				{
					IsRightGrabbing = false;
					EmitKeyUpEventForKey(EKeysLeap::LeapGrabR);
					CallFunctionOnComponents(
						[FinalHandData](ULeapComponent* Component) { Component->OnHandReleased.Broadcast(FinalHandData); });
				}
			}
		}
	}
	else
	{
		for (auto& Hand : CurrentFrame.Hands)
		{
			FLeapHandData PastHand;

			// Hand list is tiny, typically 1-3, just enum until you find the matching
			// one
			for (auto& EnumPastHand : PastFrame.Hands)
			{
				if (Hand.Id == EnumPastHand.Id)
				{
					// Same id? same hand
					PastHand = EnumPastHand;
				}
			}

			const FLeapHandData& FinalHandData = Hand;

			if (Hand.GrabStrength > StartGrabThreshold && PastHand.GrabStrength <= StartGrabThreshold)
			{
				if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
				{
					EmitKeyDownEventForKey(EKeysLeap::LeapGrabL);
				}
				else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
				{
					EmitKeyDownEventForKey(EKeysLeap::LeapGrabR);
				}
				CallFunctionOnComponents(
					[FinalHandData](ULeapComponent* Component) { Component->OnHandGrabbed.Broadcast(FinalHandData); });
			}
			// Release
			else if (Hand.GrabStrength <= EndGrabThreshold && PastHand.GrabStrength > EndGrabThreshold)
			{
				if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
				{
					EmitKeyUpEventForKey(EKeysLeap::LeapGrabL);
				}
				else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
				{
					EmitKeyUpEventForKey(EKeysLeap::LeapGrabR);
				}
				CallFunctionOnComponents(
					[FinalHandData](ULeapComponent* Component) { Component->OnHandReleased.Broadcast(FinalHandData); });
			}
		}
	}
}

void FUltraleapTrackingInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FUltraleapTrackingInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
	return false;
}

void FUltraleapTrackingInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}

void FUltraleapTrackingInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}

void FUltraleapTrackingInputDevice::ClearReferences(UObject* object)
{
	if (object != nullptr)
	{
		if (object->IsValidLowLevel())
		{
			object->RemoveFromRoot();
			object->MarkPendingKill();
		}
	}
}

void FUltraleapTrackingInputDevice::AddEventDelegate(const ULeapComponent* EventDelegate)
{
	UWorld* ComponentWorld = EventDelegate->GetOwner()->GetWorld();
	if (ComponentWorld == nullptr)
	{
		return;
	}
	// needed for world time
	Leap->SetWorld(ComponentWorld);
	// only add delegates to world
	if (ComponentWorld->WorldType == EWorldType::Game || ComponentWorld->WorldType == EWorldType::GamePreview ||
		ComponentWorld->WorldType == EWorldType::PIE)
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
	// Detach from body state
	UBodyStateBPLibrary::DetachDevice(BodyStateDeviceId);

	// This will kill the leap thread
	Leap->CloseConnection();

	LeapImageHandler->CleanupImageData();
}

void FUltraleapTrackingInputDevice::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible)
{
	LeftHandIsVisible = CurrentFrame.LeftHandVisible;
	RightHandIsVisible = CurrentFrame.RightHandVisible;
}

void FUltraleapTrackingInputDevice::LatestFrame(FLeapFrameData& OutFrame)
{
	OutFrame = CurrentFrame;
}
void FUltraleapTrackingInputDevice::SetSwizzles(
	ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW)
{
	Leap->SetSwizzles(ToX, ToY, ToZ, ToW);
}
// Policies
void FUltraleapTrackingInputDevice::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
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

void FUltraleapTrackingInputDevice::UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton)
{
	SCOPE_CYCLE_COUNTER(STAT_LeapBodyStateTick);
	// UE_LOG(UltraleapTrackingLog, Log, TEXT("Update requested for %d"),
	// DeviceID);
	bool bLeftIsTracking = false;
	bool bRightIsTracking = false;

	{
		FScopeLock ScopeLock(&Skeleton->BoneDataLock);

		// Update our skeleton with new data
		for (auto LeapHand : CurrentFrame.Hands)
		{
			if (LeapHand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				UBodyStateArm* LeftArm = Skeleton->LeftArm();

				LeftArm->LowerArm->SetPosition(LeapHand.Arm.PrevJoint);
				LeftArm->LowerArm->SetOrientation(LeapHand.Arm.Rotation);

				// Set hand data
				SetBSHandFromLeapHand(LeftArm->Hand, LeapHand);

				// We're tracking that hand, show it. If we haven't updated tracking,
				// update it.
				bLeftIsTracking = true;
			}
			else if (LeapHand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				UBodyStateArm* RightArm = Skeleton->RightArm();

				RightArm->LowerArm->SetPosition(LeapHand.Arm.PrevJoint);
				RightArm->LowerArm->SetOrientation(LeapHand.Arm.Rotation);

				// Set hand data
				SetBSHandFromLeapHand(RightArm->Hand, LeapHand);

				// We're tracking that hand, show it. If we haven't updated tracking,
				// update it.
				bRightIsTracking = true;
			}
		}
	}

	// if the number or type of bones that are tracked changed
	bool bTrackedBonesChanged = false;

	UBodyStateArm* Arm = Skeleton->LeftArm();

	// Did left hand tracking state change? propagate it
	if (bLeftIsTracking != Arm->LowerArm->IsTracked())
	{
		bTrackedBonesChanged = true;

		if (bLeftIsTracking)
		{
			Arm->LowerArm->Meta.TrackingType = Config.DeviceName;
			Arm->LowerArm->Meta.ParentDistinctMeta = true;
			Arm->LowerArm->SetTrackingConfidenceRecursively(1.f);
			Arm->LowerArm->Meta.TrackingTags = Config.TrackingTags;
		}
		else
		{
			Arm->LowerArm->SetTrackingConfidenceRecursively(0.f);
			Arm->LowerArm->Meta.ParentDistinctMeta = false;
			Arm->LowerArm->Meta.TrackingTags.Empty();
		}
	}

	Arm = Skeleton->RightArm();

	// Did right hand tracking state change? propagate it
	if (bRightIsTracking != Arm->LowerArm->IsTracked())
	{
		bTrackedBonesChanged = true;

		if (bRightIsTracking)
		{
			Arm->LowerArm->Meta.TrackingType = Config.DeviceName;
			Arm->LowerArm->Meta.ParentDistinctMeta = true;
			Arm->LowerArm->SetTrackingConfidenceRecursively(1.f);
			Arm->LowerArm->Meta.TrackingTags = Config.TrackingTags;
		}
		else
		{
			Arm->LowerArm->SetTrackingConfidenceRecursively(0.f);
			Arm->LowerArm->Meta.ParentDistinctMeta = false;
			Arm->LowerArm->Meta.TrackingTags.Empty();
		}
	}

// Livelink is an editor only thing
#if WITH_EDITOR
	// LiveLink logic
	if (LiveLink->HasConnection())
	{
		if (bTrackedBonesChanged)
		{
			LiveLink->SyncSubjectToSkeleton(Skeleton);
		}
		LiveLink->UpdateFromBodyState(Skeleton);
	}
#endif
}

void FUltraleapTrackingInputDevice::OnDeviceDetach()
{
	ShutdownLeap();
	UE_LOG(UltraleapTrackingLog, Log, TEXT("OnDeviceDetach call from BodyState."));
}

void FUltraleapTrackingInputDevice::SetBSFingerFromLeapDigit(UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit)
{
	Finger->Metacarpal->SetPosition(LeapDigit.Metacarpal.PrevJoint);
	Finger->Metacarpal->SetOrientation(LeapDigit.Metacarpal.Rotation);

	Finger->Proximal->SetPosition(LeapDigit.Proximal.PrevJoint);
	Finger->Proximal->SetOrientation(LeapDigit.Proximal.Rotation);

	Finger->Intermediate->SetPosition(LeapDigit.Intermediate.PrevJoint);
	Finger->Intermediate->SetOrientation(LeapDigit.Intermediate.Rotation);

	Finger->Distal->SetPosition(LeapDigit.Distal.PrevJoint);
	Finger->Distal->SetOrientation(LeapDigit.Distal.Rotation);

	Finger->bIsExtended = LeapDigit.IsExtended;
}

void FUltraleapTrackingInputDevice::SetBSThumbFromLeapThumb(UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit)
{
	Finger->Metacarpal->SetPosition(LeapDigit.Proximal.PrevJoint);
	Finger->Metacarpal->SetOrientation(LeapDigit.Proximal.Rotation);

	Finger->Proximal->SetPosition(LeapDigit.Intermediate.PrevJoint);
	Finger->Proximal->SetOrientation(LeapDigit.Intermediate.Rotation);

	Finger->Distal->SetPosition(LeapDigit.Distal.PrevJoint);
	Finger->Distal->SetOrientation(LeapDigit.Distal.Rotation);
}

void FUltraleapTrackingInputDevice::SetBSHandFromLeapHand(UBodyStateHand* Hand, const FLeapHandData& LeapHand)
{
	SetBSThumbFromLeapThumb(Hand->ThumbFinger(), LeapHand.Thumb);
	SetBSFingerFromLeapDigit(Hand->IndexFinger(), LeapHand.Index);
	SetBSFingerFromLeapDigit(Hand->MiddleFinger(), LeapHand.Middle);
	SetBSFingerFromLeapDigit(Hand->RingFinger(), LeapHand.Ring);
	SetBSFingerFromLeapDigit(Hand->PinkyFinger(), LeapHand.Pinky);

	Hand->Wrist->SetPosition(LeapHand.Arm.NextJoint);
	Hand->Wrist->SetOrientation(LeapHand.Palm.Orientation);

	// did our confidence change? set it recursively
	/* 4.0 breaks this as confidence isn't set!
	if (Hand->Wrist->Meta.Confidence != LeapHand.Confidence)
	{
			Hand->Wrist->SetTrackingConfidenceRecursively(LeapHand.Confidence);
	}*/
}

#pragma endregion BodyState
void FUltraleapTrackingInputDevice::SwitchTrackingSource(const bool UseOpenXRAsSource)
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
		Leap = TSharedPtr<IHandTrackingWrapper>(new FLeapWrapper);
	}
	if (!UseOpenXRAsSource)
	{
		IsWaitingForConnect = true;
	}
	Leap->OpenConnection(this);
}
void FUltraleapTrackingInputDevice::SetOptions(const FLeapOptions& InOptions)
{
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		HMDType = GEngine->XRSystem->GetSystemName();
	}

	// Did we change the data source
	if (Options.bUseOpenXRAsSource != InOptions.bUseOpenXRAsSource)
	{
		Options = InOptions;
		SwitchTrackingSource(InOptions.bUseOpenXRAsSource);
		// set this here as set options is re-called on connect
		Options.bUseOpenXRAsSource = InOptions.bUseOpenXRAsSource;
	}
	// Did we change the mode?
	if (Options.Mode != InOptions.Mode)
	{
		if (bUseNewTrackingModeAPI)
		{
			SetTrackingMode(InOptions.Mode);
		}
		else
		{
			bool bOptimizeForHMd = InOptions.Mode == ELeapMode::LEAP_MODE_VR;

			SetLeapPolicy(LEAP_POLICY_OPTIMIZE_HMD, bOptimizeForHMd);
		}
	}

	// Set main options
	Options = InOptions;

	// Cache device type
	FString DeviceType = Stats.DeviceInfo.PID;

	// If our tracking fidelity is not custom, set the parameters to good defaults
	// for each platform
	if (InOptions.TrackingFidelity == ELeapTrackingFidelity::LEAP_CUSTOM)
	{
		// We don't enforce any settings if the user is passing in custom options
	}
	else
	{
		// Vive
		// NOTE: even when not in VR, HMDType is initialized to "SteamVR" so will
		// pass through here (is it supposed to?)
		if (HMDType == TEXT("SteamVR") || HMDType == TEXT("GearVR"))
		{
			switch (InOptions.TrackingFidelity)
			{
				case ELeapTrackingFidelity::LEAP_LOW_LATENCY:
					Options.bUseTimeWarp = true;
					Options.bUseInterpolation = true;
					Options.TimewarpOffset = 9000;
					Options.TimewarpFactor = 1.f;
					Options.HandInterpFactor = 0.5f;
					Options.FingerInterpFactor = 0.5f;
					break;
				case ELeapTrackingFidelity::LEAP_NORMAL:
					Options.bUseTimeWarp = true;
					Options.bUseInterpolation = true;
					Options.TimewarpOffset = 2750;
					Options.TimewarpFactor = 1.f;
					Options.HandInterpFactor = 0.f;
					Options.FingerInterpFactor = 0.f;
					break;
				case ELeapTrackingFidelity::LEAP_SMOOTH:
					Options.bUseTimeWarp = false;
					Options.bUseInterpolation = true;
					Options.TimewarpOffset = 500;
					Options.TimewarpFactor = -1.f;
					Options.HandInterpFactor = -1.f;
					Options.FingerInterpFactor = -1.f;
					break;
				case ELeapTrackingFidelity::LEAP_WIRELESS:
					if (DeviceType == TEXT("Peripheral"))
					{
						Options.bUseTimeWarp = true;
						Options.bUseInterpolation = true;
						Options.TimewarpOffset = 9500;
						Options.TimewarpFactor = -1.f;
						Options.HandInterpFactor = -1.6f;
						Options.FingerInterpFactor = -1.6f;
					}
					else
					{
						Options.bUseTimeWarp = false;
						Options.bUseInterpolation = true;
						Options.TimewarpOffset = 10000;
						Options.TimewarpFactor = -1.f;
						Options.HandInterpFactor = -0.75f;
						Options.FingerInterpFactor = -0.75f;
					}
					break;
				case ELeapTrackingFidelity::LEAP_CUSTOM:
					break;
				default:
					break;
			}
		}

		// Rift, note requires negative timewarp!
		else if (HMDType == TEXT("OculusHMD") || HMDType == TEXT("OpenXR"))
		{
			// Apply default options to zero offsets/rotations
			if (InOptions.HMDPositionOffset.IsNearlyZero())
			{
				// in mm
				FVector OculusOffset = FVector(80.0, 0, 0);
				Options.HMDPositionOffset = OculusOffset;
			}
			if (InOptions.HMDRotationOffset.IsNearlyZero())
			{
				Options.HMDRotationOffset = FRotator(-4, 0, 0);	   // typically oculus mounts sag a tiny bit
			}

			switch (InOptions.TrackingFidelity)
			{
				case ELeapTrackingFidelity::LEAP_LOW_LATENCY:
					Options.bUseTimeWarp = true;
					Options.bUseInterpolation = true;
					Options.TimewarpOffset = 16000;
					Options.TimewarpFactor = -1.f;
					Options.HandInterpFactor = 0.5;
					Options.FingerInterpFactor = 0.5f;

					break;
				case ELeapTrackingFidelity::LEAP_NORMAL:
					if (DeviceType == TEXT("Peripheral"))
					{
						Options.TimewarpOffset = 20000;
					}
					else
					{
						Options.TimewarpOffset = 25000;
					}
					Options.bUseTimeWarp = true;
					Options.bUseInterpolation = true;
					Options.TimewarpFactor = -1.f;
					Options.HandInterpFactor = 0.f;
					Options.FingerInterpFactor = 0.f;
					break;

				case ELeapTrackingFidelity::LEAP_SMOOTH:
					Options.bUseTimeWarp = true;
					Options.bUseInterpolation = true;
					Options.TimewarpOffset = 26000;
					Options.TimewarpFactor = -1.f;
					Options.HandInterpFactor = -1.f;
					Options.FingerInterpFactor = -1.f;
					break;
				case ELeapTrackingFidelity::LEAP_CUSTOM:
					break;
				default:
					break;
			}
		}

		// Cardboard and Daydream
		else if (HMDType == TEXT("FGoogleVRHMD"))
		{
			if (InOptions.HMDPositionOffset.IsNearlyZero())
			{
				// in mm
				FVector DayDreamOffset = FVector(80.0, 0, 0);
				Options.HMDPositionOffset = DayDreamOffset;
			}

			switch (InOptions.TrackingFidelity)
			{
				case ELeapTrackingFidelity::LEAP_LOW_LATENCY:
				case ELeapTrackingFidelity::LEAP_NORMAL:
				case ELeapTrackingFidelity::LEAP_SMOOTH:
					Options.bUseTimeWarp = true;
					Options.bUseInterpolation = true;
					Options.TimewarpOffset = 10000;
					Options.TimewarpFactor = 1.f;
					Options.HandInterpFactor = -1.f;
					Options.FingerInterpFactor = -1.f;
					break;
				case ELeapTrackingFidelity::LEAP_CUSTOM:
					break;
				default:
					break;
			}

			// let's use basic vive settings for cardboard for now
		}
		// Other, e.g. cardboard
		else
		{
			// UE_LOG(UltraleapTrackingLog, Log, TEXT("%d doesn't have proper defaults
			// set yet, using passed in custom settings."), HMDType);
		}
	}
	// HMD offset not allowed in OpenXR (already corrected)
	if (Options.bUseOpenXRAsSource)
	{
		Options.HMDPositionOffset = FVector(0, 0, 0);
		Options.HMDRotationOffset = FRotator(0, 0, 0);
	}
	// Ensure other factors are synced
	HandInterpolationTimeOffset = Options.HandInterpFactor * FrameTimeInMicros;
	FingerInterpolationTimeOffset = Options.FingerInterpFactor * FrameTimeInMicros;

	// Disable time warp in desktop mode
	if (Options.Mode == ELeapMode::LEAP_MODE_DESKTOP)
	{
		Options.bUseTimeWarp = false;
	}

	// Always sync global offsets
	FLeapUtility::SetLeapGlobalOffsets(Options.HMDPositionOffset, Options.HMDRotationOffset);

	/*UseTimeBasedVisibilityCheck =*/UseTimeBasedGestureCheck = !Options.bUseFrameBasedGestureDetection;

	StartGrabThreshold = Options.StartGrabThreshold;
	EndGrabThreshold = Options.EndGrabThreshold;
	StartPinchThreshold = Options.StartPinchThreshold;
	EndPinchThreshold = Options.EndPinchThreshold;
	GrabTimeout = Options.GrabTimeout;
	PinchTimeout = Options.PinchTimeout;
}

FLeapOptions FUltraleapTrackingInputDevice::GetOptions()
{
	return Options;
}

FLeapStats FUltraleapTrackingInputDevice::GetStats()
{
	return Stats;
}
#pragma endregion Leap Input Device
