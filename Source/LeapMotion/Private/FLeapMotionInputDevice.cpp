
#include "FLeapMotionInputDevice.h"
#include "IHeadMountedDisplay.h"
#include "LeapLambdaRunnable.h"
#include "LeapComponent.h"
#include "BodyStateSkeleton.h"
#include "BodyStateBPLibrary.h"
#include "LeapMotionData.h"
#include "LeapUtility.h"
#include "SlateBasics.h"

DECLARE_STATS_GROUP(TEXT("LeapMotion"), STATGROUP_LeapMotion, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Leap Game Input and Events"), STAT_LeapInputTick, STATGROUP_LeapMotion);
DECLARE_CYCLE_STAT(TEXT("Leap BodyState Tick"), STAT_LeapBodyStateTick, STATGROUP_LeapMotion);

#pragma region Utility
//Function call Utility
void FLeapMotionInputDevice::CallFunctionOnComponents(TFunction< void(ULeapComponent*)> InFunction)
{
	//Callback optimization
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
		FLeapLambdaRunnable::RunShortLambdaOnGameThread([this, InFunction]
		{
			for (ULeapComponent* EventDelegate : EventDelegates)
			{
				InFunction(EventDelegate);
			}
		});
	}
}

//UE v4.6 IM event wrappers
bool FLeapMotionInputDevice::EmitKeyUpEventForKey(FKey Key, int32 User = 0, bool Repeat = false)
{
	if(IsInGameThread())
	{
		FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
		return FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
	}
	return false;
}

bool FLeapMotionInputDevice::EmitKeyDownEventForKey(FKey Key, int32 User = 0, bool Repeat = false)
{
	if (IsInGameThread())
	{
		FKeyEvent KeyEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0);
		return FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);
	}
	return false;
}

bool FLeapMotionInputDevice::EmitAnalogInputEventForKey(FKey Key, float Value, int32 User = 0, bool Repeat = false)
{
	if (IsInGameThread())
	{
		FAnalogInputEvent AnalogInputEvent(Key, FSlateApplication::Get().GetModifierKeys(), User, Repeat, 0, 0, Value);
		return FSlateApplication::Get().ProcessAnalogInputEvent(AnalogInputEvent);
	}
	return false;
}

//Utility

const FKey EKeysLeap::LeapPinchL("LeapPinchL");
const FKey EKeysLeap::LeapGrabL("LeapGrabL");
const FKey EKeysLeap::LeapPinchR("LeapPinchR");
const FKey EKeysLeap::LeapGrabR("LeapGrabR");

bool FLeapMotionInputDevice::HandClosed(float Strength)
{
	return (Strength == 1.f);
}

bool FLeapMotionInputDevice::HandPinched(float Strength)
{
	return (Strength > 0.8);
}

int64 FLeapMotionInputDevice::GetInterpolatedNow()
{
	return LeapGetNow() + HandInterpolationTimeOffset;
}

void FLeapMotionInputDevice::OnConnect()
{
	UE_LOG(LeapMotionLog, Log, TEXT("LeapService: OnConnect."));

	//Default to hmd mode if one is plugged in
	FLeapOptions DefaultOptions;
	Options.Mode = ELeapMode::LEAP_MODE_DESKTOP;

	//if we have a valid engine pointer and hmd update the device type
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		DefaultOptions.Mode = ELeapMode::LEAP_MODE_VR;
	}
	else
	{
		//HMD is disabled on load, default to desktop
		DefaultOptions.Mode = ELeapMode::LEAP_MODE_DESKTOP;
	}
		
	SetOptions(DefaultOptions);

	CallFunctionOnComponents([&](ULeapComponent* Component)
	{
		Component->OnLeapConnected.Broadcast();
	});
}

void FLeapMotionInputDevice::OnConnectionLost()
{
	UE_LOG(LeapMotionLog, Warning, TEXT("LeapService: OnConnectionLost."));
}

void FLeapMotionInputDevice::OnDeviceFound(const LEAP_DEVICE_INFO *props)
{
	SetOptions(Options);
	Stats.DeviceInfo.SetFromLeapDevice((_LEAP_DEVICE_INFO*)props);
	UE_LOG(LeapMotionLog, Log, TEXT("OnDeviceFound %s %s."), *Stats.DeviceInfo.PID, *Stats.DeviceInfo.Serial);
}

void FLeapMotionInputDevice::OnDeviceLost(const char* serial)
{
	UE_LOG(LeapMotionLog, Warning, TEXT("OnDeviceLost %s."), ANSI_TO_TCHAR(serial));
}

void FLeapMotionInputDevice::OnDeviceFailure(const eLeapDeviceStatus failure_code, const LEAP_DEVICE failed_device)
{
	FString ErrorString;
	switch (failure_code)
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
	UE_LOG(LeapMotionLog, Warning, TEXT("OnDeviceFailure: %s (%d)"), *ErrorString, (int32)failure_code);
}

void FLeapMotionInputDevice::OnFrame(const LEAP_TRACKING_EVENT *frame)
{
	//Not used. Polled on separate thread for lower latency.
}


void FLeapMotionInputDevice::OnImage(const LEAP_IMAGE_EVENT *image_event)
{
	//todo: handle image pairs
}

void FLeapMotionInputDevice::OnPolicy(const uint32_t current_policies)
{
	TArray<TEnumAsByte<ELeapPolicyFlag>> Flags;
	ELeapMode UpdatedMode = Options.Mode;
	if (current_policies & eLeapPolicyFlag_BackgroundFrames)
	{
		Flags.Add(ELeapPolicyFlag::LEAP_POLICY_BACKGROUND_FRAMES);
	}
	if (current_policies & eLeapPolicyFlag_OptimizeHMD)
	{
		UpdatedMode = ELeapMode::LEAP_MODE_VR;
		Flags.Add(ELeapPolicyFlag::LEAP_POLICY_OPTIMIZE_HMD);
	}
	if (current_policies & eLeapPolicyFlag_AllowPauseResume)
	{
		Flags.Add(ELeapPolicyFlag::LEAP_POLICY_ALLOW_PAUSE_RESUME);
	}

	Options.Mode = UpdatedMode;

	//Update mode for each component and broadcast current policies
	CallFunctionOnComponents([&, UpdatedMode, Flags](ULeapComponent* Component)
	{
		Component->TrackingMode = UpdatedMode;
		Component->OnLeapPoliciesUpdated.Broadcast(Flags);
	});
}

void FLeapMotionInputDevice::OnLog(const eLeapLogSeverity severity, const int64_t timestamp, const char* message)
{
	if (!message)
	{
		return;
	}

	switch (severity)
	{
	case eLeapLogSeverity_Unknown:
		break;
	case eLeapLogSeverity_Critical:
		if (Options.LeapServiceLogLevel > ELeapServiceLogLevel::LEAP_LOG_NONE)
		{
			UE_LOG(LeapMotionLog, Error, TEXT("LeapServiceError: %s"), UTF8_TO_TCHAR(message));
		}
		break;
	case eLeapLogSeverity_Warning:
		if (Options.LeapServiceLogLevel > ELeapServiceLogLevel::LEAP_LOG_ERROR)
		{
			UE_LOG(LeapMotionLog, Warning, TEXT("LeapServiceWarning: %s"), UTF8_TO_TCHAR(message));
		}
		break;
	case eLeapLogSeverity_Information:
		if (Options.LeapServiceLogLevel > ELeapServiceLogLevel::LEAP_LOG_WARNING)
		{
			UE_LOG(LeapMotionLog, Log, TEXT("LeapServiceLog: %s"), UTF8_TO_TCHAR(message));
		}
		break;
	default:
		break;
	}
}

#pragma endregion Utility

#pragma region Leap Input Device

#define LOCTEXT_NAMESPACE "LeapMotion"

FLeapMotionInputDevice::FLeapMotionInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) :
	MessageHandler(InMessageHandler)
{
	//Link callbacks

	//Open the connection
	TimewarpTween = 0.5f;
	SlerpTween = 0.5f;
	GameTimeInSec = 0.f;
	HMDType = TEXT("SteamVR");
	FrameTimeInMicros = 0;	//default

	//Set static stats
	Stats.LeapAPIVersion = FString(TEXT("3.2.0"));

	Leap.OpenConnection(this);							//pass in the this as callback delegate

	bSceneViewExtensionSet = false;

	//Attach to bodystate
	Config.DeviceName = "Leap Motion";
	Config.InputType = EBodyStateDeviceInputType::HMD_MOUNTED_INPUT_TYPE;
	BodyStateDeviceId = UBodyStateBPLibrary::AttachDeviceNative(Config, this);

	//Add IM keys
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchL, LOCTEXT("LeapPinchL", "Leap (L) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabL, LOCTEXT("LeapGrabL", "Leap (L) Grab"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapPinchR, LOCTEXT("LeapPinchR", "Leap (R) Pinch"), FKeyDetails::GamepadKey));
	EKeys::AddKey(FKeyDetails(EKeysLeap::LeapGrabR, LOCTEXT("LeapGrabR", "Leap (R) Grab"), FKeyDetails::GamepadKey));
}

#undef LOCTEXT_NAMESPACE

FLeapMotionInputDevice::~FLeapMotionInputDevice()
{
	ShutdownLeap();
}


void FLeapMotionInputDevice::Tick(float DeltaTime)
{
	GameTimeInSec += DeltaTime;
	FrameTimeInMicros = DeltaTime * 1000000;

	//TODO: enable the scene view extension and add the late update
	/*if (!bSceneViewExtensionSet && GEngine)
	{
		TSharedPtr<ISceneViewExtension, ESPMode::ThreadSafe> ViewExtension(this);
		GEngine->ViewExtensions.Add(ViewExtension);
		bSceneViewExtensionSet = true;
	}*/
}


//Main loop event emitter
void FLeapMotionInputDevice::SendControllerEvents()
{
	CaptureAndEvaluateInput();
}

void FLeapMotionInputDevice::CaptureAndEvaluateInput()
{
	SCOPE_CYCLE_COUNTER(STAT_LeapInputTick);

	//Did a device connect?
	if (!Leap.bIsConnected || !Leap.lastDevice)
	{
		return;
	}

	_LEAP_TRACKING_EVENT* Frame = Leap.GetFrame();

	//Is the frame valid?
	if (!Frame)
	{
		return;
	}

	TimeWarpTimeStamp = Frame->info.timestamp;

	int64 LeapTimeNow = LeapGetNow();
	SnapshotHandler.AddCurrentHMDSample(LeapTimeNow);

	HandInterpolationTimeOffset = Options.HandInterpFactor * FrameTimeInMicros;
	FingerInterpolationTimeOffset = Options.FingerInterpFactor * FrameTimeInMicros;

	if (Options.bUseInterpolation)
	{
		//Let's interpolate the frame using leap function

		//Get the future interpolated finger frame
		Frame = Leap.GetInterpolatedFrameAtTime(LeapTimeNow + FingerInterpolationTimeOffset);
		CurrentFrame.SetFromLeapFrame(Frame);

		//Get the future interpolated hand frame, farther than fingers to provide lower latency
		Frame = Leap.GetInterpolatedFrameAtTime(LeapTimeNow + HandInterpolationTimeOffset);
		CurrentFrame.SetInterpolationPartialFromLeapFrame(Frame);

		//Track our extrapolation time in stats
		Stats.FrameExtrapolationInMS = (CurrentFrame.TimeStamp - TimeWarpTimeStamp) / 1000.f;
	}
	else
	{
		//Frame = Leap.GetFrame();
		CurrentFrame.SetFromLeapFrame(Frame);
		Stats.FrameExtrapolationInMS = 0;
	}

	ParseEvents();
}

void FLeapMotionInputDevice::ParseEvents()
{
	//Are we in HMD mode? add our HMD snapshot
	if (Options.Mode == LEAP_MODE_VR && Options.bTransformOriginToHMD)
	{
		CurrentFrame.TranslateFrame(Options.HMDPositionOffset);		//Offset HMD-Leap

		if (Options.bTransformOriginToHMD)
		{
			BodyStateHMDSnapshot SnapshotNow = SnapshotHandler.LastHMDSample();

			if (IsInGameThread())
			{
				SnapshotNow = BSHMDSnapshotHandler::CurrentHMDSample(LeapGetNow());
			}

			FRotator FinalHMDRotation = SnapshotNow.Orientation.Rotator();
			FVector FinalHMDTranslation = SnapshotNow.Position;

			//Determine time-warp, only relevant for VR
			if (Options.bUseTimeWarp)
			{
				//We use fixed timewarp offsets so then is a fixed amount away from now (negative). Positive numbers are invalid for TimewarpOffset
				BodyStateHMDSnapshot SnapshotThen = SnapshotHandler.HMDSampleClosestToTimestamp(SnapshotNow.Timestamp - Options.TimewarpOffset);

				BodyStateHMDSnapshot SnapshotDifference = SnapshotNow.Difference(SnapshotThen);

				FRotator WarpRotation = SnapshotDifference.Orientation.Rotator() *Options.TimewarpFactor;
				FVector WarpTranslation = SnapshotDifference.Position * Options.TimewarpFactor;

				FinalHMDTranslation += WarpTranslation;

				FinalHMDRotation = FLeapUtility::CombineRotators(WarpRotation, FinalHMDRotation);
				CurrentFrame.FinalRotationAdjustment = FinalHMDRotation;
			}

			//Rotate our frame by time warp difference
			CurrentFrame.RotateFrame(FinalHMDRotation);
			CurrentFrame.TranslateFrame(FinalHMDTranslation);
		}
	}

	//Update visible hand list, must happen first
	VisibleHands.Empty(CurrentFrame.Hands.Num());
	for (auto& Hand : CurrentFrame.Hands)
	{
		//Add each hand to visible hands
		VisibleHands.Add(Hand.Id);
	}

	//Compare past to present visible hands to determine hand enums.
	//== change can happen when chirality is incorrect and changes
	//Hand end tracking must be called first before we call begin tracking
	if (VisibleHands.Num() <= PastVisibleHands.Num())
	{
		for (auto HandId : PastVisibleHands)
		{
			//Not visible anymore? lost hand
			if (!VisibleHands.Contains(HandId))
			{
				const FLeapHandData Hand = PastFrame.HandForId(HandId);
				CallFunctionOnComponents([Hand](ULeapComponent* Component)
				{
					Component->OnHandEndTracking.Broadcast(Hand);
				});
			}
		}
	}

	//Check for hand visibility changes
	if (PastFrame.LeftHandVisible != CurrentFrame.LeftHandVisible)
	{
		const bool LeftVisible = CurrentFrame.LeftHandVisible;
		CallFunctionOnComponents([this, LeftVisible](ULeapComponent* Component)
		{
			Component->OnLeftHandVisibilityChanged.Broadcast(LeftVisible);
		});
	}
	if (PastFrame.RightHandVisible != CurrentFrame.RightHandVisible)
	{
		const bool RightVisible = CurrentFrame.RightHandVisible;
		CallFunctionOnComponents([this, RightVisible](ULeapComponent* Component)
		{
			Component->OnRightHandVisibilityChanged.Broadcast(RightVisible);
		});
	}

	for (auto& Hand : CurrentFrame.Hands)
	{
		FLeapHandData PastHand;

		//Hand list is tiny, typically 1-3, just enum until you find the matching one
		for (auto& EnumPastHand : PastFrame.Hands)
		{
			if (Hand.Id == EnumPastHand.Id)
			{
				//Same id? same hand
				PastHand = EnumPastHand;
			}
		}

		const FLeapHandData& FinalHandData = Hand;

		if (!PastVisibleHands.Contains(Hand.Id))	//or if the hand changed type?
		{
			//New hand
			CallFunctionOnComponents([Hand](ULeapComponent* Component)
			{
				Component->OnHandBeginTracking.Broadcast(Hand);
			});
		}
		
		//Grab
		if (Hand.GrabStrength > 0.5f && PastHand.GrabStrength <= 0.5f)
		{
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				EmitKeyDownEventForKey(EKeysLeap::LeapGrabL);
			}
			else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				EmitKeyDownEventForKey(EKeysLeap::LeapGrabR);
			}
			CallFunctionOnComponents([FinalHandData](ULeapComponent* Component)
			{
				Component->OnHandGrabbed.Broadcast(FinalHandData);
			});
		}
		//Release
		else if (Hand.GrabStrength <= 0.5f && PastHand.GrabStrength > 0.5f)
		{
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				EmitKeyUpEventForKey(EKeysLeap::LeapGrabL);
			}
			else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				EmitKeyUpEventForKey(EKeysLeap::LeapGrabR);
			}
			CallFunctionOnComponents([FinalHandData](ULeapComponent* Component)
			{
				Component->OnHandReleased.Broadcast(FinalHandData);
			});
		}

		//Pinch
		if (Hand.PinchStrength > 0.5f && PastHand.PinchStrength <= 0.5f)
		{
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				EmitKeyDownEventForKey(EKeysLeap::LeapPinchL);
			}
			else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				EmitKeyDownEventForKey(EKeysLeap::LeapPinchR);
			}
			CallFunctionOnComponents([FinalHandData](ULeapComponent* Component)
			{
				Component->OnHandPinched.Broadcast(FinalHandData);
			});
		}
		//Unpinch
		else if (Hand.PinchStrength <= 0.5f && PastHand.PinchStrength > 0.5f)
		{
			if (Hand.HandType == EHandType::LEAP_HAND_LEFT)
			{
				EmitKeyUpEventForKey(EKeysLeap::LeapPinchL);
			}
			else if (Hand.HandType == EHandType::LEAP_HAND_RIGHT)
			{
				EmitKeyUpEventForKey(EKeysLeap::LeapPinchR);
			}
			CallFunctionOnComponents([FinalHandData](ULeapComponent* Component)
			{
				Component->OnHandUnpinched.Broadcast(FinalHandData);
			});
		}
	}//End for each hand

	CallFunctionOnComponents([this](ULeapComponent* Component)
	{
		//Scale input?
		//FinalFrameData.ScaleByWorldScale(Component->GetWorld()->GetWorldSettings()->WorldToMeters / 100.f);
		Component->OnLeapTrackingData.Broadcast(CurrentFrame);
	});

	//It's now the past data
	PastVisibleHands = VisibleHands;
	PastFrame = CurrentFrame;
}

void FLeapMotionInputDevice::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FLeapMotionInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
	return false;
}


void FLeapMotionInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}


void FLeapMotionInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values)
{
	// Nothing necessary to do (boilerplate code to complete the interface)
}

void FLeapMotionInputDevice::ClearReferences(UObject* object)
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

void FLeapMotionInputDevice::AddEventDelegate(const ULeapComponent* EventDelegate)
{
	UWorld* ComponentWorld = EventDelegate->GetOwner()->GetWorld();
	if (ComponentWorld == nullptr)
	{
		return;
	}
	//only add delegates to world
	if (ComponentWorld->WorldType == EWorldType::Game ||
		ComponentWorld->WorldType == EWorldType::GamePreview ||
		ComponentWorld->WorldType == EWorldType::PIE)
	{
		if (EventDelegate != nullptr && EventDelegate->IsValidLowLevel())
		{
			EventDelegates.Add((ULeapComponent*)EventDelegate);
		}

		UE_LOG(LeapMotionLog, Log, TEXT("AddEventDelegate (%d)."), EventDelegates.Num());
	}
}

void FLeapMotionInputDevice::RemoveEventDelegate(const ULeapComponent* EventDelegate)
{
	EventDelegates.Remove((ULeapComponent*)EventDelegate);
	//UE_LOG(LeapMotionLog, Log, TEXT("RemoveEventDelegate (%d)."), EventDelegates.Num());
}

void FLeapMotionInputDevice::ShutdownLeap()
{
	//Detach from body state
	UBodyStateBPLibrary::DetachDevice(BodyStateDeviceId);

	//This will kill the leap thread
	Leap.CloseConnection();
}

void FLeapMotionInputDevice::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible)
{
	LeftHandIsVisible = CurrentFrame.LeftHandVisible;
	RightHandIsVisible = CurrentFrame.RightHandVisible;
}

void FLeapMotionInputDevice::LatestFrame(FLeapFrameData& OutFrame)
{
	OutFrame = CurrentFrame;
}

//Policies
bool FLeapMotionInputDevice::EnableImageStreaming(bool Enable)
{
	//not yet implemented
	return false;
}

void FLeapMotionInputDevice::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	switch (Flag)
	{
	case LEAP_POLICY_ALLOW_PAUSE_RESUME:
		Leap.SetPolicyFlagFromBoolean(eLeapPolicyFlag_AllowPauseResume, Enable);
		break;
	case LEAP_POLICY_BACKGROUND_FRAMES:
		Leap.SetPolicyFlagFromBoolean(eLeapPolicyFlag_BackgroundFrames, Enable);
		break;
	case LEAP_POLICY_OPTIMIZE_HMD:
		Leap.SetPolicyFlagFromBoolean(eLeapPolicyFlag_OptimizeHMD, Enable);
		break;
	default:
		break;
	}
}

#pragma endregion Leap Input Device

#pragma region BodyState

void FLeapMotionInputDevice::UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton)
{
	SCOPE_CYCLE_COUNTER(STAT_LeapBodyStateTick);
	//UE_LOG(LeapMotionLog, Log, TEXT("Update requested for %d"), DeviceID);
	bool bLeftIsTracking = false;
	bool bRightIsTracking = false;

	//Update our skeleton with new data
	for (auto LeapHand : CurrentFrame.Hands)
	{
		if (LeapHand.HandType == EHandType::LEAP_HAND_LEFT)
		{
			UBodyStateArm* LeftArm = Skeleton->LeftArm();

			LeftArm->LowerArm->SetPosition(LeapHand.Arm.PrevJoint);
			LeftArm->LowerArm->SetOrientation(LeapHand.Arm.Rotation);
			//LeftArm->LowerArm->Meta.Confidence = LeapHand.Confidence;
			//LeftArm->LowerArm->Meta.Confidence = 1.f;

			//Set hand data
			SetBSHandFromLeapHand(LeftArm->Hand, LeapHand);

			//We're tracking that hand, show it. If we haven't updated tracking, update it.
			bLeftIsTracking = true;
		}
		else if (LeapHand.HandType == EHandType::LEAP_HAND_RIGHT)
		{
			UBodyStateArm* RightArm = Skeleton->RightArm();

			RightArm->LowerArm->SetPosition(LeapHand.Arm.PrevJoint);
			RightArm->LowerArm->SetOrientation(LeapHand.Arm.Rotation);
			//RightArm->LowerArm->Meta.Confidence = LeapHand.Confidence;
			//RightArm->LowerArm->Meta.Confidence = 1.f;

			//Set hand data
			SetBSHandFromLeapHand(RightArm->Hand, LeapHand);

			//We're tracking that hand, show it. If we haven't updated tracking, update it.
			bRightIsTracking = true;
		}
	}

	UBodyStateArm* Arm = Skeleton->LeftArm();

	//Did left hand tracking state change? propagate it
	if (bLeftIsTracking != Arm->LowerArm->IsTracked())
	{
		if (bLeftIsTracking)
		{
			Arm->LowerArm->Meta.TrackingType = Config.DeviceName;
			Arm->LowerArm->Meta.ParentDistinctMeta = true;
			Arm->LowerArm->SetTrackingConfidenceRecursively(1.f);
		}
		else
		{
			Arm->LowerArm->SetTrackingConfidenceRecursively(0.f);
			Arm->LowerArm->Meta.ParentDistinctMeta = false;
		}
	}

	Arm = Skeleton->RightArm();

	//Did right hand tracking state change? propagate it
	if (bRightIsTracking != Arm->LowerArm->IsTracked())
	{
		if (bRightIsTracking)
		{
			Arm->LowerArm->Meta.TrackingType = Config.DeviceName;
			Arm->LowerArm->Meta.ParentDistinctMeta = true;
			Arm->LowerArm->SetTrackingConfidenceRecursively(1.f);
		}
		else
		{
			Arm->LowerArm->SetTrackingConfidenceRecursively(0.f);
			Arm->LowerArm->Meta.ParentDistinctMeta = false;
		}
	}
}

void FLeapMotionInputDevice::OnDeviceDetach()
{
	ShutdownLeap();
	UE_LOG(LeapMotionLog, Log, TEXT("OnDeviceDetach call from BodyState."));
}

void FLeapMotionInputDevice::SetBSFingerFromLeapDigit(UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit)
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

void FLeapMotionInputDevice::SetBSThumbFromLeapThumb(UBodyStateFinger* Finger, const FLeapDigitData& LeapDigit)
{
	Finger->Metacarpal->SetPosition(LeapDigit.Proximal.PrevJoint);
	Finger->Metacarpal->SetOrientation(LeapDigit.Proximal.Rotation);

	Finger->Proximal->SetPosition(LeapDigit.Intermediate.PrevJoint);
	Finger->Proximal->SetOrientation(LeapDigit.Intermediate.Rotation);

	Finger->Distal->SetPosition(LeapDigit.Distal.PrevJoint);
	Finger->Distal->SetOrientation(LeapDigit.Distal.Rotation);
}

void FLeapMotionInputDevice::SetBSHandFromLeapHand(UBodyStateHand* Hand, const FLeapHandData& LeapHand)
{
	SetBSThumbFromLeapThumb(Hand->ThumbFinger(), LeapHand.Thumb);
	SetBSFingerFromLeapDigit(Hand->IndexFinger(), LeapHand.Index);
	SetBSFingerFromLeapDigit(Hand->MiddleFinger(), LeapHand.Middle);
	SetBSFingerFromLeapDigit(Hand->RingFinger(), LeapHand.Ring);
	SetBSFingerFromLeapDigit(Hand->PinkyFinger(), LeapHand.Pinky);

	Hand->Wrist->SetPosition(LeapHand.Arm.NextJoint);
	Hand->Wrist->SetOrientation(LeapHand.Palm.Orientation);

	//did our confidence change? set it recursively
	/* 4.0 breaks this as confidence isn't set!
	if (Hand->Wrist->Meta.Confidence != LeapHand.Confidence)
	{
		Hand->Wrist->SetTrackingConfidenceRecursively(LeapHand.Confidence);
	}*/
}

#pragma endregion BodyState

#pragma region FSceneViewExtension

void FLeapMotionInputDevice::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
}

void FLeapMotionInputDevice::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)
{
	//SnapshotHandler.AddCurrentHMDSample(LeapGetNow());
	CurrentFrame.SetFromLeapFrame(Leap.GetFrame());
	ParseEvents();
}

void FLeapMotionInputDevice::SetOptions(const FLeapOptions& InOptions)
{
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		HMDType = GEngine->XRSystem->GetSystemName();
	}
	else
	{
		Options.Mode = ELeapMode::LEAP_MODE_DESKTOP;
	}

	//Did we change the mode?
	if (Options.Mode != InOptions.Mode)
	{
		bool bOptimizeForHMd = InOptions.Mode == ELeapMode::LEAP_MODE_VR;
		SetLeapPolicy(LEAP_POLICY_OPTIMIZE_HMD, bOptimizeForHMd);
	}

	//Set main options
	Options = InOptions;

	//If our tracking fidelity is not custom, set the parameters to good defaults for each platform
	if (InOptions.TrackingFidelity == ELeapTrackingFidelity::LEAP_CUSTOM)
	{
		//We don't enforce any settings if the user is passing in custom options
	}
	else
	{
		//Vive
		if (HMDType == TEXT("SteamVR") ||
			HMDType == TEXT("GearVR"))
		{
			//Apply default options to zero offsets/rotations
			if (InOptions.HMDPositionOffset.IsNearlyZero())
			{
				FVector ViveOffset = FVector(9.0, 0, 0);
				Options.HMDPositionOffset = ViveOffset;
			}
			if (InOptions.HMDRotationOffset.IsNearlyZero())
			{
				Options.HMDRotationOffset = FRotator(-10.f, 0, 0);	//typically vive mounts sag a bit
			}

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
			case ELeapTrackingFidelity::LEAP_CUSTOM:
				break;
			default:
				break;
			}
		}

		//Rift, note requires negative timewarp!
		else if (HMDType == TEXT("OculusHMD"))
		{
			//Apply default options to zero offsets/rotations
			if (InOptions.HMDPositionOffset.IsNearlyZero())
			{
				FVector OculusOffset = FVector(8.0, 0, 0);
				Options.HMDPositionOffset = OculusOffset;
			}

			switch (InOptions.TrackingFidelity)
			{
			case ELeapTrackingFidelity::LEAP_LOW_LATENCY:
				Options.bUseTimeWarp = true;
				Options.bUseInterpolation = true;
				Options.TimewarpOffset = 24000;
				Options.TimewarpFactor = -1.f;
				Options.HandInterpFactor = 0.5;
				Options.FingerInterpFactor = 0.5f;
				
				break;
			case ELeapTrackingFidelity::LEAP_NORMAL:

				Options.bUseTimeWarp = true;
				Options.bUseInterpolation = true;
				Options.TimewarpOffset = 29000;
				Options.TimewarpFactor = -1.f;
				Options.HandInterpFactor = 0.f;
				Options.FingerInterpFactor = 0.f;
				break;
			case ELeapTrackingFidelity::LEAP_SMOOTH:
				Options.bUseTimeWarp = true;
				Options.bUseInterpolation = true;
				Options.TimewarpOffset = 35000;
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
		//Cardboard and Daydream
		else if (HMDType == TEXT("FGoogleVRHMD"))
		{
			if (InOptions.HMDPositionOffset.IsNearlyZero())
			{
				FVector DayDreamOffset = FVector(8.0, 0, 0);
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

			//let's use basic vive settings for cardboard for now

		}
		//Other, e.g. cardboard 
		else
		{
			//UE_LOG(LeapMotionLog, Log, TEXT("%d doesn't have proper defaults set yet, using passed in custom settings."), HMDType);
		}
	}

	//Ensure other factors are synced
	HandInterpolationTimeOffset = Options.HandInterpFactor * FrameTimeInMicros;
	FingerInterpolationTimeOffset = Options.FingerInterpFactor * FrameTimeInMicros;

	//Disable time warp in desktop mode
	if (Options.Mode == ELeapMode::LEAP_MODE_DESKTOP)
	{
		Options.bUseTimeWarp = false;
	}

	//Always sync global offsets
	FLeapUtility::SetLeapGlobalOffsets(Options.HMDPositionOffset, Options.HMDRotationOffset);

	//Did we trigger our image mode?
	//TODO: add image request support here and forward the events to a struct with two UTexture2Ds for each image
	//Leap.EnableImageStream(Options.bEnableImageStreaming)
}

FLeapOptions FLeapMotionInputDevice::GetOptions()
{
	return Options;
}

FLeapStats FLeapMotionInputDevice::GetStats()
{
	return Stats;
}

#pragma endregion Leap Input Device
