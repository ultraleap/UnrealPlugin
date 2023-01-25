/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapDevice.h"
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


DECLARE_STATS_GROUP(TEXT("UltraleapMultiTracking"), STATGROUP_UltraleapMultiTracking, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Multi Leap Game Input and Events"), STAT_MultiLeapInputTick, STATGROUP_UltraleapMultiTracking);
DECLARE_CYCLE_STAT(TEXT("Multi Leap BodyState Tick"), STAT_MultiLeapBodyStateTick, STATGROUP_UltraleapMultiTracking);

#pragma region Utility
bool FUltraleapDevice::bUseNewTrackingModeAPI = true;

// Bodystate data is X Up, Y Right, Z Forward
// UE is X Forward, Y Right, Z Up

FTransform FUltraleapDevice::ConvertUEDeviceOriginToBSTransform(const FTransform& TransformUE,const bool Direction)
{
	FTransform Ret = TransformUE;
	const float Y = TransformUE.GetRotation().Rotator().Yaw;
	const float R = TransformUE.GetRotation().Rotator().Roll;
	const float P = TransformUE.GetRotation().Rotator().Pitch;

	if (Direction)
	{
		// inverse out so we transform the hand against the device origin on the way in
		Ret.SetLocation(FVector(-TransformUE.GetLocation().Z, -TransformUE.GetLocation().Y, TransformUE.GetLocation().X));
		// constructor is pitch yaw roll
		Ret.SetRotation(FRotator(-P, -R , -Y).Quaternion());
	}
	else
	{
		Ret.SetLocation(FVector(TransformUE.GetLocation().Z, -TransformUE.GetLocation().Y, -TransformUE.GetLocation().X));
		Ret.SetRotation(FRotator(-P ,-R, -Y).Quaternion());
	}
	
	return Ret;
}
// Function call Utility
void FUltraleapDevice::CallFunctionOnComponents(TFunction<void(ULeapComponent*)> InFunction)
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
bool FUltraleapDevice::EmitKeyUpEventForKey(FKey Key, int32 User = 0, bool Repeat = false)
{
	if (IsInGameThread())
	{
		FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
		return FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
	}
	return false;
}

bool FUltraleapDevice::EmitKeyDownEventForKey(FKey Key, int32 User = 0, bool Repeat = false)
{
	if (IsInGameThread())
	{
		FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
		return FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
	}
	return false;
}

bool FUltraleapDevice::EmitAnalogInputEventForKey(FKey Key, float Value, int32 User = 0, bool Repeat = false)
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


bool FUltraleapDevice::HandClosed(float Strength)
{
	return (Strength == 1.f);
}

bool FUltraleapDevice::HandPinched(float Strength)
{
	return (Strength > 0.8);
}

int64 FUltraleapDevice::GetInterpolatedNow()
{
	return Leap->GetNow() + HandInterpolationTimeOffset;
}

// comes from service message loop
void FUltraleapDevice::InitOptions()
{
	//FLeapAsync::RunShortLambdaOnGameThread([&] {

		SetOptions(Options);
//	});
}


void FUltraleapDevice::OnFrame(const LEAP_TRACKING_EVENT* Frame)
{
	if (TrackingDeviceWrapper)
	{
		TrackingDeviceWrapper->HandleTrackingEvent(Frame);
	}
}

void FUltraleapDevice::OnImage(const LEAP_IMAGE_EVENT* ImageEvent)
{
	// Forward it to the handler
	LeapImageHandler->OnImage(ImageEvent);
}

void FUltraleapDevice::OnImageCallback(UTexture2D* LeftCapturedTexture, UTexture2D* RightCapturedTexture)
{
	// Handler has returned with batched easy-to-parse results, forward callback
	// on game thread
	CallFunctionOnComponents([LeftCapturedTexture, RightCapturedTexture](ULeapComponent* Component) {
		Component->OnImageEvent.Broadcast(LeftCapturedTexture, ELeapImageType::LEAP_IMAGE_LEFT);
		Component->OnImageEvent.Broadcast(RightCapturedTexture, ELeapImageType::LEAP_IMAGE_RIGHT);
	});
}

void FUltraleapDevice::OnPolicy(const uint32_t CurrentPolicies)
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
void FUltraleapDevice::OnTrackingMode(const eLeapTrackingMode CurrentMode)
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
void FUltraleapDevice::OnLog(const eLeapLogSeverity Severity, const int64_t Timestamp, const char* Message)
{
	if (!Message)
	{
		return;
	}

	switch (Severity)
	{
		case eLeapLogSeverity_Unknown:
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

FUltraleapDevice::FUltraleapDevice(
	IHandTrackingWrapper* LeapDeviceWrapper, ITrackingDeviceWrapper* TrackingDeviceWrapperIn, const bool StartInOpenXRMode)
	: 
	Leap(LeapDeviceWrapper), TrackingDeviceWrapper(TrackingDeviceWrapperIn)
{
	// Link callbacks

	// Open the connection
	DeltaTimeFromTick = 0;
	TimewarpTween = 0.5f;
	SlerpTween = 0.5f;
	GameTimeInSec = 0.f;
	HMDType = TEXT("SteamVR");
	FrameTimeInMicros = 0;	  // default

	// Set static stats
	Stats.LeapAPIVersion = FString(TEXT("4.0.1"));

	SwitchTrackingSource(StartInOpenXRMode);
	Options.bUseOpenXRAsSource = StartInOpenXRMode;

	if (StartInOpenXRMode)
	{
		Options.Mode = ELeapMode::LEAP_MODE_VR;
	}

	Init();
}

#undef LOCTEXT_NAMESPACE
ELeapDeviceType ToBlueprintDeviceType(eLeapDevicePID LeapType)
{
	switch(LeapType)
	{
		case eLeapDevicePID_Unknown:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_UNKNOWN;
			break;
		case eLeapDevicePID_Peripheral:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_PERIPHERAL;
			break;

		case eLeapDevicePID_Dragonfly:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_DRAGONFLY;
			break;
		case eLeapDevicePID_Nightcrawler:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_NIGHTCRAWLER;
			break;
		case eLeapDevicePID_Rigel:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_RIGEL;
			break;
		case eLeapDevicePID_SIR170:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_SIR170;
			break;
		case eLeapDevicePID_3Di:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_3DI;
			break;
		default:
			return ELeapDeviceType::LEAP_DEVICE_TYPE_UNKNOWN;
	}
}

void FUltraleapDevice::Init()
{
	// Attach to bodystate
	
	Config.DeviceName = FString::Printf(TEXT("Leap Motion %s"), *Leap->GetDeviceSerial());
	Config.InputType = EBodyStateDeviceInputType::HMD_MOUNTED_INPUT_TYPE;
	Config.TrackingTags.Add("Hands");
	Config.TrackingTags.Add("Fingers");
	if (Leap)
	{
		Config.DeviceSerial = Leap->GetDeviceSerial();
	}
	BodyStateDeviceId = UBodyStateBPLibrary::AttachDeviceNative(Config, this);

#if WITH_EDITOR
	// LiveLink startup
	LiveLink = MakeShareable(new FLeapLiveLinkProducer());
	LiveLink->Startup(Leap->GetDeviceSerial());
	LiveLink->SyncSubjectToSkeleton(IBodyState::Get().SkeletonForDevice(BodyStateDeviceId));
#endif

	// Image support
	LeapImageHandler = MakeShareable(new FLeapImage);
	LeapImageHandler->OnImageCallback.AddRaw(this, &FUltraleapDevice::OnImageCallback);

	InitOptions();

	if (Leap)
	{
		Leap->SetCallbackDelegate(this);
		if (Leap->GetDeviceProperties())
		{
			DeviceType = ToBlueprintDeviceType(Leap->GetDeviceProperties()->pid);
		}

	}
}
FUltraleapDevice::~FUltraleapDevice()
{
#if WITH_EDITOR
	if (LiveLink != nullptr)
	{
		// LiveLink cleanup
		LiveLink->ShutDown();
		LiveLink = nullptr;
	}
#endif

	ShutdownLeap();
}

void FUltraleapDevice::Tick(float DeltaTime)
{
	GameTimeInSec += DeltaTime;
	FrameTimeInMicros = DeltaTime * 1000000;
	DeltaTimeFromTick = DeltaTime;
	
}

// Main loop event emitter
void FUltraleapDevice::SendControllerEvents()
{
	CaptureAndEvaluateInput();
}
void FUltraleapDevice::GetLatestFrameData(FLeapFrameData& OutData,const bool ApplyDeviceOriginIn /* = false */)
{
	OutData = CurrentFrame;

	if (ApplyDeviceOriginIn)
	{
		ApplyDeviceOrigin(OutData);
	}
}
void FUltraleapDevice::ApplyDeviceOrigin(FLeapFrameData& OutData)
{
	// in BS Space
	const FTransform& Origin = GetDeviceOrigin();

	OutData.RotateFrame(Origin.GetRotation().Rotator());
	OutData.TranslateFrame(Origin.GetLocation());
}
void FUltraleapDevice::CaptureAndEvaluateInput()
{
	SCOPE_CYCLE_COUNTER(STAT_MultiLeapInputTick);
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

			if (!Frame)
			{
				return;
			}
			CurrentFrame.SetFromLeapFrame(Frame, Options.HMDPositionOffset, Options.HMDRotationOffset.Quaternion());

			// Get the future interpolated hand frame, farther than fingers to provide
			// lower latency
			Frame = Leap->GetInterpolatedFrameAtTime(LeapTimeNow + HandInterpolationTimeOffset);
			if (!Frame)
			{
				return;
			}
			CurrentFrame.SetInterpolationPartialFromLeapFrame(Frame,Options.HMDPositionOffset, Options.HMDRotationOffset.Quaternion());

			// Track our extrapolation time in stats
			Stats.FrameExtrapolationInMS = (CurrentFrame.TimeStamp - TimeWarpTimeStamp) / 1000.f;
		}
		else
		{
			CurrentFrame.SetFromLeapFrame(Frame, Options.HMDPositionOffset, Options.HMDRotationOffset.Quaternion());
			Stats.FrameExtrapolationInMS = 0;
		}
	}
	else
	{
		CurrentFrame.SetFromLeapFrame(Frame, Options.HMDPositionOffset, Options.HMDRotationOffset.Quaternion());
		Stats.FrameExtrapolationInMS = 0;
	}

	ParseEvents();
}

void FUltraleapDevice::ParseEvents()
{
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

		// store device origin for combiner
		// Ideally this should include the HMD offset
		//SetDeviceOrigin(FTransform(FinalHMDRotation, FinalHMDTranslation));
	}
	else if (Options.Mode == LEAP_MODE_SCREENTOP)
	{
		FRotator ScreentopToDesktop(-90, 0, 180);
		CurrentFrame.RotateFrame(ScreentopToDesktop.GetInverse());
	}
	if (LastLeapTime == 0)
		LastLeapTime = Leap->GetNow();
	
	// apply any tracking system specific changes to the hand
	// e.g. Pinch and Grasp simulation for OpenXR
	Leap->PostLeapHandUpdate(CurrentFrame);

	CheckHandVisibility();
	CheckGrabGesture();
	CheckPinchGesture();

	// Emit tracking data if it is being captured
	CallFunctionOnComponents(
		[this](ULeapComponent* Component)
		{
			// Scale input?
			// FinalFrameData.ScaleByWorldScale(Component->GetWorld()->GetWorldSettings()->WorldToMeters
			// / 100.f);
			Component->OnLeapTrackingData.Broadcast(CurrentFrame);
		});

	// It's now the past data
	PastFrame = CurrentFrame;
	LastLeapTime = Leap->GetNow();
}

void FUltraleapDevice::CheckHandVisibility()
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
						CallFunctionOnComponents([this, LeftVisible](ULeapComponent* Component)
							{ Component->OnLeftHandVisibilityChanged.Broadcast(LeftVisible); });
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
						CallFunctionOnComponents([this, RightVisible](ULeapComponent* Component)
							{ Component->OnRightHandVisibilityChanged.Broadcast(RightVisible); });
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
			CallFunctionOnComponents([this, RightVisible](ULeapComponent* Component)
				{ Component->OnRightHandVisibilityChanged.Broadcast(RightVisible); });
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
			CallFunctionOnComponents([this, RightVisible](ULeapComponent* Component)
				{ Component->OnRightHandVisibilityChanged.Broadcast(RightVisible); });
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

void FUltraleapDevice::CheckPinchGesture()
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
				if ((!IsLeftGrabbing && (!IsLeftPinching && (Hand.PinchStrength > StartPinchThreshold))) ||
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
				if ((!IsRightGrabbing && (!IsRightPinching && (Hand.PinchStrength > StartPinchThreshold))) ||
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

void FUltraleapDevice::CheckGrabGesture()
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
// Device specific events such as tracking mode change will be passed through here
// in addition to global events such as add remove device.
void FUltraleapDevice::AddEventDelegate(const ULeapComponent* EventDelegate)
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

void FUltraleapDevice::RemoveEventDelegate(const ULeapComponent* EventDelegate)
{
	EventDelegates.Remove((ULeapComponent*) EventDelegate);
	// UE_LOG(UltraleapTrackingLog, Log, TEXT("RemoveEventDelegate (%d)."),
	// EventDelegates.Num());
}

void FUltraleapDevice::ShutdownLeap()
{
	// Detach from body state
	UBodyStateBPLibrary::DetachDevice(BodyStateDeviceId);

	if (Leap != nullptr)
	{
		// This will kill the leap thread
		Leap->CloseConnection();
	}
	if (LeapImageHandler != nullptr)
	{
		LeapImageHandler->CleanupImageData();
	}
}

void FUltraleapDevice::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible)
{
	LeftHandIsVisible = CurrentFrame.LeftHandVisible;
	RightHandIsVisible = CurrentFrame.RightHandVisible;
}

void FUltraleapDevice::SetSwizzles(
	ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW)
{
	Leap->SetSwizzles(ToX, ToY, ToZ, ToW);
}
// Policies
void FUltraleapDevice::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
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
void FUltraleapDevice::SetTrackingMode(ELeapMode Flag)
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

void FUltraleapDevice::UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton)
{
	SCOPE_CYCLE_COUNTER(STAT_MultiLeapBodyStateTick);
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
void FUltraleapDevice::SetBSFingerFromLeapDigit(UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit)
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

void FUltraleapDevice::SetBSThumbFromLeapThumb(UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit)
{
	Finger->Metacarpal->SetPosition(LeapDigit.Proximal.PrevJoint);
	Finger->Metacarpal->SetOrientation(LeapDigit.Proximal.Rotation);

	Finger->Proximal->SetPosition(LeapDigit.Intermediate.PrevJoint);
	Finger->Proximal->SetOrientation(LeapDigit.Intermediate.Rotation);

	Finger->Distal->SetPosition(LeapDigit.Distal.PrevJoint);
	Finger->Distal->SetOrientation(LeapDigit.Distal.Rotation);

	Finger->bIsExtended = LeapDigit.IsExtended;
}

void FUltraleapDevice::SetBSHandFromLeapHand(UBodyStateHand* Hand, const FLeapHandData& LeapHand)
{
	SetBSThumbFromLeapThumb(Hand->ThumbFinger(), LeapHand.Thumb);
	SetBSFingerFromLeapDigit(Hand->IndexFinger(), LeapHand.Index);
	SetBSFingerFromLeapDigit(Hand->MiddleFinger(), LeapHand.Middle);
	SetBSFingerFromLeapDigit(Hand->RingFinger(), LeapHand.Ring);
	SetBSFingerFromLeapDigit(Hand->PinkyFinger(), LeapHand.Pinky);

	Hand->Wrist->SetPosition(LeapHand.Arm.NextJoint);
	Hand->Wrist->SetOrientation(LeapHand.Palm.Orientation);
}

#pragma endregion BodyState
void FUltraleapDevice::SwitchTrackingSource(const bool UseOpenXRAsSource)
{
	Leap->OpenConnection(this);
}
void FUltraleapDevice::SetOptions(const FLeapOptions& InOptions)
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
					if (DeviceType == ELeapDeviceType::LEAP_DEVICE_TYPE_PERIPHERAL)
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
			Options.HMDPositionOffset = FVector(90,0,0);
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
					if (DeviceType == ELeapDeviceType::LEAP_DEVICE_TYPE_PERIPHERAL)
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
		// Pico
		else if (HMDType == TEXT("PicoXRHMD"))
		{
			// Apply default options to zero offsets/rotations
			if (InOptions.HMDPositionOffset.IsNearlyZero())
			{
				// in mm
				FVector Offset = FVector(50.0, 0, 0);
				Options.HMDPositionOffset = Offset;
			}
			if (InOptions.HMDRotationOffset.IsNearlyZero())
			{
				Options.HMDRotationOffset = FRotator(-4, 0, 0);	  // does it point down because velcro?
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
					if (DeviceType == ELeapDeviceType::LEAP_DEVICE_TYPE_PERIPHERAL)
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
		Options.HMDPositionOffset = FVector(0, 0, 0);
		Options.HMDRotationOffset = FRotator::ZeroRotator;
	}

	UseTimeBasedGestureCheck = !Options.bUseFrameBasedGestureDetection;

	StartGrabThreshold = Options.StartGrabThreshold;
	EndGrabThreshold = Options.EndGrabThreshold;
	StartPinchThreshold = Options.StartPinchThreshold;
	EndPinchThreshold = Options.EndPinchThreshold;
	GrabTimeout = Options.GrabTimeout;
	PinchTimeout = Options.PinchTimeout;
}
FLeapOptions FUltraleapDevice::GetOptions()
{
	return Options;
}

FLeapStats FUltraleapDevice::GetStats()
{
	return Stats;
}
void FUltraleapDevice::OnDeviceDetach()
{
	ShutdownLeap();
	UE_LOG(UltraleapTrackingLog, Log, TEXT("OnDeviceDetach call from BodyState."));
}

#pragma endregion Leap Input Device
