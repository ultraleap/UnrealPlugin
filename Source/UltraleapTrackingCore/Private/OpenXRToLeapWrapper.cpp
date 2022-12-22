/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "OpenXRToLeapWrapper.h"
#include "HeadMountedDisplayTypes.h"
#include "IHandTracker.h"
#include "IXRTrackingSystem.h"
#include "Kismet/GameplayStatics.h"
#include "LeapBlueprintFunctionLibrary.h"
#include "LeapUtility.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "FUltraleapDevice.h"
#include "Kismet/KismetMathLibrary.h"

FOpenXRToLeapWrapper::FOpenXRToLeapWrapper()
{
	static int32 OpenXRDeviceID = OpenXRBaseDeviceID;

	OpenXRDeviceID++;
	DeviceID = OpenXRDeviceID;

	CurrentDeviceInfo = &DummyDeviceInfo;
	DummyDeviceInfo = {0};
	DummyDeviceInfo.size = sizeof(LEAP_DEVICE_INFO);
	DummyDeviceInfo.serial = (char*) ("OpenXRDummyDevice");
	DummyDeviceInfo.serial_length = strlen(DummyDeviceInfo.serial) + 1;

	DummyLeapHands[0] = {0};
	DummyLeapHands[1] = {0};

	DummyLeapHands[0].type = eLeapHandType::eLeapHandType_Left;
	DummyLeapHands[1].type = eLeapHandType::eLeapHandType_Right;

	DummyLeapHands[0].id = -1;
	DummyLeapHands[1].id = -1;

	DummyLeapFrame = {{0}};

	DummyLeapFrame.framerate = 90;
	DummyLeapFrame.pHands = DummyLeapHands;

	Device = MakeShared<FUltraleapDevice>((IHandTrackingWrapper*) this, (ITrackingDeviceWrapper*) this, true);

	// finger IDs are the index into the digits
	for (int i = 0; i < 5; ++i)
	{
		DummyLeapHands[0].digits[i].finger_id = DummyLeapHands[1].digits[i].finger_id = i;
	}
}
FOpenXRToLeapWrapper::~FOpenXRToLeapWrapper()
{
}
LEAP_CONNECTION* FOpenXRToLeapWrapper::OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode)
{
	CallbackDelegate = InCallbackDelegate;
	InitOpenXRHandTrackingModule();
	return nullptr;
}

void FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule()
{
	static FName SystemName(TEXT("OpenXR"));

	if (!GEngine)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Error: FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule() - GEngine is NULL"));

		return;
	}
	if (!GEngine->XRSystem.IsValid())
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Warning: FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule() No XR System found, is an HMD connected?"));
	
		return;
	}
	if (GEngine->XRSystem->GetSystemName() == SystemName)
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}
	if (XRTrackingSystem == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Warning: FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule() No OpenXR System found, are OpenXR plugins enabled"));
		return;
	}

	IModuleInterface* ModuleInterface = FModuleManager::Get().LoadModule("OpenXRULHandTrackingExt");

	if (!ModuleInterface)
	{
		ModuleInterface = FModuleManager::Get().LoadModule("OpenXRHandTracking");
	}

	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
	{
		auto Implementations = IModularFeatures::Get().GetModularFeatureImplementations<IHandTracker>(IHandTracker::GetModularFeatureName());
		
		for (auto Implementation : Implementations)
		{
			if (Implementation->GetHandTrackerDeviceTypeName() == "UltraleapOpenXRHandTracking")
			{
				HandTracker = Implementation;
				UsingUltraleapExtension = true;
			}
		}
		// fallback to default hand tracking
		if (!HandTracker)
		{
			HandTracker = &IModularFeatures::Get().GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());
			UsingUltraleapExtension = false;
		}
		
		bIsConnected = true;	

		if (CallbackDelegate)
		{
			CallbackDelegate->OnDeviceFound(&DummyDeviceInfo);
		}
	}
	else
	{
		UE_LOG(UltraleapTrackingLog, Log,
			TEXT(" FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule() - OpenXRHandTracking module not found, is the "
				 "OpenXRHandTracking plugin enabled?"));
	}
}
LEAP_VECTOR ConvertPositionToLeap(const FVector& FromOpenXR)
{
	LEAP_VECTOR Ret = FLeapUtility::ConvertAndScaleUEToLeap(FromOpenXR);

	return Ret;
}

int Sign(const ELeapQuatSwizzleAxisB& QuatSwizzleAxis)
{
	return (QuatSwizzleAxis > ELeapQuatSwizzleAxisB::W) ? -1 : 1;
}
LEAP_QUATERNION FOpenXRToLeapWrapper::ConvertOrientationToLeap(const FQuat& FromOpenXR)
{
	LEAP_QUATERNION Ret = {{{0}}};

	FVector4 OldRotVector(FromOpenXR.X, FromOpenXR.Y, FromOpenXR.Z, FromOpenXR.W);

	Ret.x = Sign(SwizzleX) * OldRotVector[StaticCast<uint8>(SwizzleX) % 4];
	Ret.y = Sign(SwizzleY) * OldRotVector[StaticCast<uint8>(SwizzleY) % 4];
	Ret.z = Sign(SwizzleZ) * OldRotVector[StaticCast<uint8>(SwizzleZ) % 4];
	Ret.w = Sign(SwizzleW) * OldRotVector[StaticCast<uint8>(SwizzleW) % 4];

	return Ret;
}
LEAP_VECTOR ConvertFVectorToLeapVector(const FVector& UEVector)
{
	LEAP_VECTOR Ret;
	// inverse of  FVector(LeapVector.y, -LeapVector.x, -LeapVector.z);

	Ret.x = -UEVector.Y;
	Ret.y = UEVector.X;
	Ret.z = -UEVector.Z;

	return Ret;
}
void FOpenXRToLeapWrapper::SetHandJointFromKeypoint(
	const int KeyPoint, LEAP_HAND& LeapHand, const FVector& Position, const FQuat& Rotation)
{
	EHandKeypoint eKeyPoint = (EHandKeypoint) KeyPoint;
	switch (eKeyPoint)
	{
		case EHandKeypoint::Palm:
			// wrist orientation comes from palm orientation in bodystate
			// palm orientation is calculated from palm direction in LeapHandData
			{
				LeapHand.palm.orientation = ConvertOrientationToLeap(Rotation);
				LeapHand.palm.position = ConvertPositionToLeap(Position);
			}
			break;
		case EHandKeypoint::Wrist:
			// wrist comes from arm next joint in bodystate
			LeapHand.arm.prev_joint = LeapHand.arm.next_joint = ConvertPositionToLeap(Position);
			// set arm rotation from Wrist
			LeapHand.arm.rotation = ConvertOrientationToLeap(Rotation);
			LeapHand.arm.width = 10;
			break;
			// Thumb ////////////////////////////////////////////////////
			/** From the leap data header
			 *
			 * For thumbs, this bone is set to have zero length and width, an identity basis matrix,
			 * and its joint positions are equal.
			 * Note that this is anatomically incorrect; in anatomical terms, the intermediate phalange
			 * is absent in a real thumb, rather than the metacarpal bone. In the Leap Motion model,
			 * however, we use a "zero" metacarpal bone instead for ease of programming.
			 * @since 3.0.0
			 */
		case EHandKeypoint::ThumbMetacarpal:
			LeapHand.thumb.metacarpal.prev_joint = LeapHand.thumb.proximal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.thumb.metacarpal.rotation = LeapHand.thumb.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::ThumbProximal:
			LeapHand.thumb.intermediate.prev_joint = LeapHand.thumb.metacarpal.next_joint = LeapHand.thumb.proximal.next_joint =
				ConvertPositionToLeap(Position);
			LeapHand.thumb.intermediate.rotation = ConvertOrientationToLeap(Rotation);
			break;
		case EHandKeypoint::ThumbDistal:
			LeapHand.thumb.distal.prev_joint = LeapHand.thumb.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.thumb.distal.rotation = ConvertOrientationToLeap(Rotation);
			break;
		case EHandKeypoint::ThumbTip:
			// tip is next of distal
			LeapHand.thumb.distal.next_joint = ConvertPositionToLeap(Position);
			break;

		// Index ////////////////////////////////////////////////////
		case EHandKeypoint::IndexMetacarpal:
			LeapHand.index.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.index.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::IndexProximal:
			LeapHand.index.proximal.prev_joint = LeapHand.index.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.index.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::IndexIntermediate:
			LeapHand.index.intermediate.prev_joint = LeapHand.index.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.index.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::IndexDistal:
			LeapHand.index.distal.prev_joint = LeapHand.index.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.index.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::IndexTip:
			LeapHand.index.distal.next_joint = ConvertPositionToLeap(Position);

			break;
		// Middle ////////////////////////////////////////////////////
		case EHandKeypoint::MiddleMetacarpal:
			LeapHand.middle.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::MiddleProximal:
			LeapHand.middle.proximal.prev_joint = LeapHand.middle.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::MiddleIntermediate:
			LeapHand.middle.intermediate.prev_joint = LeapHand.middle.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::MiddleDistal:
			LeapHand.middle.distal.prev_joint = LeapHand.middle.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::MiddleTip:
			LeapHand.middle.distal.next_joint = ConvertPositionToLeap(Position);

			break;
		// Ring ////////////////////////////////////////////////////
		case EHandKeypoint::RingMetacarpal:
			LeapHand.ring.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::RingProximal:
			LeapHand.ring.proximal.prev_joint = LeapHand.ring.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::RingIntermediate:
			LeapHand.ring.intermediate.prev_joint = LeapHand.ring.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::RingDistal:
			LeapHand.ring.distal.prev_joint = LeapHand.ring.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::RingTip:
			LeapHand.ring.distal.next_joint = ConvertPositionToLeap(Position);

			break;

		// Little/pinky ////////////////////////////////////////////////////
		case EHandKeypoint::LittleMetacarpal:
			LeapHand.pinky.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::LittleProximal:
			LeapHand.pinky.proximal.prev_joint = LeapHand.pinky.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::LittleIntermediate:
			LeapHand.pinky.intermediate.prev_joint = LeapHand.pinky.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::LittleDistal:
			LeapHand.pinky.distal.prev_joint = LeapHand.pinky.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypoint::LittleTip:
			LeapHand.pinky.distal.next_joint = ConvertPositionToLeap(Position);

			break;
		default:
			UE_LOG(UltraleapTrackingLog, Log,
				TEXT("FOpenXRToLeapWrapper::ConvertToLeapSpace() - Unknown keypoint found in OpenXR data"));
			break;
	}
}
void FOpenXRToLeapWrapper::SetHandJointFromKeypointExt(const int KeyPoint, LEAP_HAND& LeapHand, const FVector& Position, const FQuat& Rotation)
{
	EHandKeypointUL eKeyPoint = (EHandKeypointUL) KeyPoint;
	switch (eKeyPoint)
	{
		case EHandKeypointUL::Palm:
			// wrist orientation comes from palm orientation in bodystate
			// palm orientation is calculated from palm direction in LeapHandData
			{
				LeapHand.palm.orientation = ConvertOrientationToLeap(Rotation);
				LeapHand.palm.position = ConvertPositionToLeap(Position);
			}
			break;
		case EHandKeypointUL::Wrist:
			// wrist comes from arm next joint in bodystate
			LeapHand.arm.next_joint = ConvertPositionToLeap(Position);
			// set arm rotation from Wrist
			//LeapHand.arm.rotation = ConvertOrientationToLeap(Rotation);
			LeapHand.arm.width = 10;
			break;
		case EHandKeypointUL::Elbow:
			LeapHand.arm.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.arm.rotation = ConvertOrientationToLeap(Rotation);
			break;
			// Thumb ////////////////////////////////////////////////////
			/** From the leap data header
			 *
			 * For thumbs, this bone is set to have zero length and width, an identity basis matrix,
			 * and its joint positions are equal.
			 * Note that this is anatomically incorrect; in anatomical terms, the intermediate phalange
			 * is absent in a real thumb, rather than the metacarpal bone. In the Leap Motion model,
			 * however, we use a "zero" metacarpal bone instead for ease of programming.
			 * @since 3.0.0
			 */
		
		case EHandKeypointUL::ThumbMetacarpal:
			LeapHand.thumb.metacarpal.prev_joint = LeapHand.thumb.proximal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.thumb.metacarpal.rotation = LeapHand.thumb.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::ThumbProximal:
			LeapHand.thumb.intermediate.prev_joint = LeapHand.thumb.metacarpal.next_joint = LeapHand.thumb.proximal.next_joint =
				ConvertPositionToLeap(Position);
			LeapHand.thumb.intermediate.rotation = ConvertOrientationToLeap(Rotation);
			break;
		case EHandKeypointUL::ThumbDistal:
			LeapHand.thumb.distal.prev_joint = LeapHand.thumb.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.thumb.distal.rotation = ConvertOrientationToLeap(Rotation);
			break;
		case EHandKeypointUL::ThumbTip:
			// tip is next of distal
			LeapHand.thumb.distal.next_joint = ConvertPositionToLeap(Position);
			break;

		// Index ////////////////////////////////////////////////////
		case EHandKeypointUL::IndexMetacarpal:
			LeapHand.index.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.index.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::IndexProximal:
			LeapHand.index.proximal.prev_joint = LeapHand.index.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.index.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::IndexIntermediate:
			LeapHand.index.intermediate.prev_joint = LeapHand.index.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.index.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::IndexDistal:
			LeapHand.index.distal.prev_joint = LeapHand.index.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.index.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::IndexTip:
			LeapHand.index.distal.next_joint = ConvertPositionToLeap(Position);

			break;
		// Middle ////////////////////////////////////////////////////
		case EHandKeypointUL::MiddleMetacarpal:
			LeapHand.middle.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::MiddleProximal:
			LeapHand.middle.proximal.prev_joint = LeapHand.middle.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::MiddleIntermediate:
			LeapHand.middle.intermediate.prev_joint = LeapHand.middle.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::MiddleDistal:
			LeapHand.middle.distal.prev_joint = LeapHand.middle.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.middle.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::MiddleTip:
			LeapHand.middle.distal.next_joint = ConvertPositionToLeap(Position);

			break;
		// Ring ////////////////////////////////////////////////////
		case EHandKeypointUL::RingMetacarpal:
			LeapHand.ring.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::RingProximal:
			LeapHand.ring.proximal.prev_joint = LeapHand.ring.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::RingIntermediate:
			LeapHand.ring.intermediate.prev_joint = LeapHand.ring.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::RingDistal:
			LeapHand.ring.distal.prev_joint = LeapHand.ring.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.ring.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::RingTip:
			LeapHand.ring.distal.next_joint = ConvertPositionToLeap(Position);

			break;

		// Little/pinky ////////////////////////////////////////////////////
		case EHandKeypointUL::LittleMetacarpal:
			LeapHand.pinky.metacarpal.prev_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.metacarpal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::LittleProximal:
			LeapHand.pinky.proximal.prev_joint = LeapHand.pinky.metacarpal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.proximal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::LittleIntermediate:
			LeapHand.pinky.intermediate.prev_joint = LeapHand.pinky.proximal.next_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.intermediate.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::LittleDistal:
			LeapHand.pinky.distal.prev_joint = LeapHand.pinky.intermediate.next_joint = ConvertPositionToLeap(Position);
			LeapHand.pinky.distal.rotation = ConvertOrientationToLeap(Rotation);

			break;
		case EHandKeypointUL::LittleTip:
			LeapHand.pinky.distal.next_joint = ConvertPositionToLeap(Position);

			break;
		default:
			UE_LOG(UltraleapTrackingLog, Log,
				TEXT("FOpenXRToLeapWrapper::ConvertToLeapSpace() - Unknown keypoint found in OpenXR data"));
			break;
	}
}
void FOpenXRToLeapWrapper::ConvertToLeapSpace(LEAP_HAND& LeapHand, const TArray<FVector>& Positions, const TArray<FQuat>& Rotations)
{
	if (!XRTrackingSystem)
	{
		return;
	}
	// Enums for each bone are in EHandKeypoint/UL
	uint8 KeyPoint = 0;
	for (auto Position : Positions)
	{
		auto Rotation = Rotations[KeyPoint];

		// Take out the player transform, this isn't valid as we want Leap Space which knows nothing about
		// the player world position
		const FTransform& TrackingToWorldTransform = XRTrackingSystem->GetTrackingToWorldTransform();
		Position = TrackingToWorldTransform.InverseTransformPosition(Position);
		Rotation = TrackingToWorldTransform.InverseTransformRotation(Rotation);

		// additional rotate all to get into Leap rotation space
		// see FLeapUtility::LeapRotationOffset()
		FTransform RotateToLeap;
		RotateToLeap.SetRotation(FRotator(90, 0, 180).Quaternion());

		Position = RotateToLeap.TransformPosition(Position);
		Rotation = RotateToLeap.TransformRotation(Rotation);

		if (UsingUltraleapExtension)
		{
			SetHandJointFromKeypointExt(KeyPoint, LeapHand, Position, Rotation);
		}
		else
		{
			SetHandJointFromKeypoint(KeyPoint, LeapHand, Position, Rotation);
		}
		KeyPoint++;
	}
}

LEAP_TRACKING_EVENT* FOpenXRToLeapWrapper::GetInterpolatedFrameAtTime(int64 TimeStamp)
{
	return GetFrame();
}
void FOpenXRToLeapWrapper::UpdateHandState()
{
}
LEAP_DEVICE_INFO* FOpenXRToLeapWrapper::GetDeviceProperties()
{
	return CurrentDeviceInfo;
}
// TODO: update hand Ids on tracking lost and found
// TODO: work out hand poses and confidence based on how the MS/WMR code does it (pinch and grab)
LEAP_TRACKING_EVENT* FOpenXRToLeapWrapper::GetFrame()
{
	if (HandTracker == nullptr)
	{
		return &DummyLeapFrame;
	}
	TArray<FVector> OutPositions[2];
	TArray<FQuat> OutRotations[2];
	TArray<float> OutRadii[2];

	// status only true when the hand is being tracked/visible to the tracking device
	// these are in world space
	//
	// IMPORTANT: OpenXR tracking only works in VR mode, this will always return false in desktop mode
	bool StatusLeft = HandTracker->GetAllKeypointStates(EControllerHand::Left, OutPositions[0], OutRotations[0], OutRadii[0]);
	bool StatusRight = HandTracker->GetAllKeypointStates(EControllerHand::Right, OutPositions[1], OutRotations[1], OutRadii[1]);

	DummyLeapFrame.nHands = StatusLeft + StatusRight;
	DummyLeapFrame.info.frame_id++;
	UWorld* World = nullptr;

	// time in microseconds
	DummyLeapFrame.info.timestamp = GetDummyLeapTime();
	DummyLeapFrame.tracking_frame_id++;

	if (!StatusLeft)
	{
		DummyLeapFrame.pHands = &DummyLeapHands[1];
	}
	else
	{
		DummyLeapFrame.pHands = &DummyLeapHands[0];
	}

	if (StatusLeft)
	{
		
		ConvertToLeapSpace(DummyLeapHands[0], OutPositions[0], OutRotations[0]);
		// not tracking -> tracking = update hand IDs
		if (!LeftHandVisible)
		{
			LeftHandVisible = true;
			DummyLeapHands[0].id = HandID++;
			FirstSeenLeft = GetGameTimeInSeconds();
		}
	}
	else
	{
		LeftHandVisible = false;
	}
	if (StatusRight)
	{
		ConvertToLeapSpace(DummyLeapHands[1], OutPositions[1], OutRotations[1]);
		// not tracking -> tracking = update hand IDs
		if (!RightHandVisible)
		{
			RightHandVisible = true;
			DummyLeapHands[1].id = HandID++;
			FirstSeenRight = GetGameTimeInSeconds();
		}
		
	}
	else
	{
		RightHandVisible = false;
	}

	return &DummyLeapFrame;
}
int64_t FOpenXRToLeapWrapper::GetDummyLeapTime()
{
	// time in microseconds
	if (!CurrentWorld)
	{
		return 0;
	}
	{
		return CurrentWorld->GetTimeSeconds() * 1000000.0f;
	}
}
void FOpenXRToLeapWrapper::SetWorld(UWorld* World)
{
	FLeapWrapperBase::SetWorld(World);
	/* no delta TODO: get world framerate from somewhere if (World)
	{
		float WorldDelta = World->GetDeltaSeconds();
		if (WorldDelta)
		{
			DummyLeapFrame.framerate = 1.0f / WorldDelta;
		}
	}*/
}

void FOpenXRToLeapWrapper::CloseConnection()
{
	if (!bIsConnected)
	{
		// Not connected, already done
		UE_LOG(UltraleapTrackingLog, Log, TEXT("FOpenXRToLeapWrapper Attempt at closing an already closed connection."));
		return;
	}
	bIsConnected = false;
	// Nullify the callback delegate. Any outstanding task graphs will not run if the delegate is nullified.
	CallbackDelegate = nullptr;

	UE_LOG(UltraleapTrackingLog, Log, TEXT("FOpenXRToLeapWrapper Connection successfully closed."));
}
void FOpenXRToLeapWrapper::SetTrackingMode(eLeapTrackingMode TrackingMode)
{
	// needed to make sure delegates in BP get called back on mode change
	if (CallbackDelegate)
	{
		CallbackDelegate->OnTrackingMode(TrackingMode);
	}
}
void FOpenXRToLeapWrapper::PostLeapHandUpdate(FLeapFrameData& Frame)
{
	// simulate pinch and grab state (in LeapC this comes from the tracking service)
	for (auto& Hand : Frame.Hands)
	{
		UpdatePinchAndGrab(Hand);
	}
}
IHandTrackingDevice* FOpenXRToLeapWrapper::GetDevice()
{
	return Device.Get();
}

float FOpenXRToLeapWrapper::CalculatePinchStrength(const FLeapHandData& Hand, float PalmWidth)
{
	// Magic values taken from existing LeapC implementation (scaled to metres)

	float HandScale = PalmWidth / 0.08425f;
	float DistanceZero = 0.0600f * HandScale;
	float DistanceOne = 0.0220f * HandScale;

	// Get the thumb position.
	// TipPosition in Unity, is Distal->Next the same?
	auto ThumbTipPosition = Hand.Thumb.Distal.NextJoint;

	// Compute the distance midpoints between the thumb and the each finger and find the smallest.
	float MinDistanceSquared = TNumericLimits<float>::Max();
	
	int32 FingerIndex = 0;
	for(const auto& Finger : Hand.Digits)
	{
		// skip 1
		if (!FingerIndex)
		{
			FingerIndex++;
			continue;
		}
		FVector Diff = Finger.Distal.NextJoint - ThumbTipPosition;
		float DistanceSquared = Diff.SizeSquared();
		MinDistanceSquared = FMath::Min(DistanceSquared, MinDistanceSquared);
	
		FingerIndex++;
	}

	// Compute the pinch strength.
	return FMath::Clamp<float>((FMath::Sqrt(MinDistanceSquared) - DistanceZero) / (DistanceOne - DistanceZero), 0.0, 1.0);
}

float FOpenXRToLeapWrapper::CalculateBoneDistanceSquared(const FLeapBoneData& BoneA, const FLeapBoneData& BoneB)
{
	// Denormalize directions to bone length.
	const auto BoneAJoint = BoneA.PrevJoint;
	const auto BoneBJoint = BoneB.PrevJoint;

	const auto BoneADirection = BoneA.Rotation.Vector() * ((BoneA.NextJoint - BoneA.PrevJoint).Size());
	const auto BoneBDirection = BoneB.Rotation.Vector() * ((BoneB.NextJoint - BoneB.PrevJoint).Size());

	// Compute the minimum (squared) distance between two bones.
	const auto Diff = BoneBJoint - BoneAJoint;
	const auto D1 = FVector::DotProduct(BoneADirection, Diff);
	const auto D2 = FVector::DotProduct(BoneBDirection, Diff);
	const auto A = BoneADirection.SizeSquared();
	const auto B = FVector::DotProduct(BoneADirection, BoneBDirection);
	const auto C = BoneBDirection.SizeSquared();
	const auto Det = B * B - A * C;
	const auto T1 = FMath::Clamp<float>((B * D2 - C * D1) / Det, 0.0, 1.0);
	const auto T2 = FMath::Clamp<float>((A * D2 - B * D1) / Det, 0.0, 1.0);
	const auto Pa = BoneAJoint + T1 * BoneADirection;
	const auto Pb = BoneBJoint + T2 * BoneBDirection;
	return (Pa - Pb).SizeSquared();
}

float FOpenXRToLeapWrapper::CalculatePinchDistance(const FLeapHandData& Hand)
{
	// Get the farthest 2 segments of thumb and index finger, respectively, and compute distances.
	auto MinDistanceSquared = TNumericLimits<float>::Max();
	
	int32 ThumbBoneIndex = 0;
	for(const auto ThumbBone : Hand.Thumb.Bones)
	{
		// skip 2
		if (ThumbBoneIndex < 2)
		{
			ThumbBoneIndex++;
			continue;
		}

		int IndexBoneIndex = 0;
		for(const auto IndexBone : Hand.Index.Bones)
		{
			// skip 2
			if (IndexBoneIndex < 2)
			{
				IndexBoneIndex++;
				continue;
			}
			const auto DistanceSquared = CalculateBoneDistanceSquared(ThumbBone, IndexBone);
			MinDistanceSquared = FMath::Min(DistanceSquared, MinDistanceSquared);

			IndexBoneIndex++;
		}
		ThumbBoneIndex++;
	}

	// Return the pinch distance
	return FMath::Sqrt(MinDistanceSquared);
}
/// Returns the the direction towards the thumb that is perpendicular to the palmar
/// and distal axes. Left and right hands will return opposing directions.
///
/// The direction away from the thumb would be called the ulnar axis.
FVector HandRadialAxis(const FLeapHandData& Hand)
{
	FTransform Basis(Hand.Palm.Orientation, Hand.Palm.Position);

	const FVector RightVector = UKismetMathLibrary::GetRightVector(Hand.Palm.Orientation);
	
	if (Hand.HandType == LEAP_HAND_RIGHT)
	{
		return -RightVector;
	}
	else
	{
		return RightVector;
	}
	return FVector::ZeroVector;
}
/// Returns the direction towards the fingers that is perpendicular to the palmar
/// and radial axes.
///
/// The direction towards the wrist would be called the proximal axis.
FVector HandDistalAxis(const FLeapHandData& Hand)
{
	// in BS Space forward is Z = UpVector in UE
	return UKismetMathLibrary::GetUpVector(Hand.Palm.Orientation);
}
FVector FingerDirection(const FLeapDigitData& Finger)
{
	// The direction in which this finger or tool is pointing.The direction is expressed
	// as a unit vector pointing in the same direction as the tip.
	return UKismetMathLibrary::GetUpVector(Finger.Distal.Rotation);
}
/// Maps the value between valueMin and valueMax to its linearly proportional equivalent between resultMin and resultMax.
/// The input value is clamped between valueMin and valueMax; if this is not desired, see MapUnclamped.
float MapRange(const float Value,const float ValueMin,const float ValueMax,const float ResultMin,const float ResultMax)
{
	if (ValueMin == ValueMax)
		return ResultMin;
	return FMath::Lerp(ResultMin, ResultMax, ((Value - ValueMin) / (ValueMax - ValueMin)));
}
float GetFingerStrength(const FLeapHandData& Hand, int Finger)
{
	return MapRange(FVector::DotProduct(FingerDirection(Hand.Digits[Finger]), -HandDistalAxis(Hand)), -1, 1, 0, 1);
}
float FOpenXRToLeapWrapper::CalculateGrabStrength(const FLeapHandData& Hand)
{
	// magic numbers so it approximately lines up with the leap results
	const float BendZero = 0.25f;
	const float BendOne = 0.85f;

	// Find the minimum bend angle for the non-thumb fingers.
	float MinBend = TNumericLimits<float>::Max();

	for (int FingerIdx = 1; FingerIdx < 5; FingerIdx++)
	{
		MinBend = FMath::Min( GetFingerStrength(Hand, FingerIdx), MinBend);
	}

	// Return the grab strength.
	return FMath::Clamp<float>((MinBend - BendZero) / (BendOne - BendZero), 0.0, 1.0);
}

float FOpenXRToLeapWrapper::CalculateGrabAngle(const FLeapHandData& Hand)
{
	// Compute the sum of the angles between the fingertips and hands.
	// For every finger, the angle is the sumb of bend + pitch + bow.
	float AngleSum = 0.0f;
	for (int FingerIdx = 1; FingerIdx < 5; FingerIdx++)
	{
		AngleSum += FMath::Lerp< float > (0, PI, GetFingerStrength(Hand, FingerIdx));
	}
	// Average between all fingers
	return AngleSum / 4.0f;
}
void FOpenXRToLeapWrapper::UpdatePinchAndGrab(FLeapHandData& Hand)
{
	Hand.PinchDistance = CalculatePinchDistance(Hand);
	Hand.PinchStrength =
		CalculatePinchStrength(Hand, FVector::Distance(Hand.Thumb.Metacarpal.NextJoint, Hand.Pinky.Metacarpal.NextJoint));
	Hand.GrabAngle = CalculateGrabAngle(Hand);
	Hand.GrabStrength = CalculateGrabStrength(Hand);

	float TimeNow = GetGameTimeInSeconds();

	switch (Hand.HandType)
	{
		case LEAP_HAND_LEFT:
		{
			if (LeftHandVisible)
			{
				Hand.VisibleTime = TimeNow - FirstSeenLeft;
			}
			break;
		}
		case LEAP_HAND_RIGHT:
		{
			if (RightHandVisible)
			{
				Hand.VisibleTime = TimeNow - FirstSeenRight;
			}
			break;
		}
	}

	int FingerIndex = 0;
	for (auto& Finger : Hand.Digits)
	{
		Finger.IsExtended = GetFingerStrength(Hand, FingerIndex) < 0.4;
		FingerIndex++;
	}
}
float FOpenXRToLeapWrapper::GetGameTimeInSeconds()
{
	return CurrentWorld ? CurrentWorld->GetTimeSeconds() : 0.f;
}