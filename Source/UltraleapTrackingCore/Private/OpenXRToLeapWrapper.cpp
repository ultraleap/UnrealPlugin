// /****************************************************************************** * Copyright (C) Ultraleap, Ltd. 2011-2021. * *
// * * Use subject to the terms of the Apache License 2.0 available at            * * http://www.apache.org/licenses/LICENSE-2.0, or
// another agreement           * * between Ultraleap and you, your company or other organization.             *
// ******************************************************************************/

#include "OpenXRToLeapWrapper.h"

#include "HeadMountedDisplayTypes.h"
#include "IHandTracker.h"
#include "Kismet/GameplayStatics.h"
#include "LeapBlueprintFunctionLibrary.h"
#include "LeapUtility.h"
#include "Runtime/Engine/Classes/Engine/World.h"

FOpenXRToLeapWrapper::FOpenXRToLeapWrapper()
{
	CurrentDeviceInfo = &DummyDeviceInfo;
	DummyDeviceInfo = {0};
	DummyDeviceInfo.size = sizeof(LEAP_DEVICE_INFO);
	DummyDeviceInfo.serial = "OpenXRDummyDevice";
	DummyDeviceInfo.serial_length = strlen(DummyDeviceInfo.serial) + 1;

	DummyLeapHands[0] = {0};
	DummyLeapHands[1] = {0};

	DummyLeapHands[0].type = eLeapHandType::eLeapHandType_Left;
	DummyLeapHands[1].type = eLeapHandType::eLeapHandType_Right;

	DummyLeapHands[0].id = 1000;
	DummyLeapHands[1].id = 2000;

	DummyLeapFrame = {0};

	DummyLeapFrame.framerate = 90;
	DummyLeapFrame.pHands = DummyLeapHands;
}

FOpenXRToLeapWrapper::~FOpenXRToLeapWrapper()
{
}
LEAP_CONNECTION* FOpenXRToLeapWrapper::OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate)
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
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}
	if (XRTrackingSystem == nullptr)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("Error: FOpenXRToLeapWrapper::InitOpenXRHandTrackingModule() No XR System found"));
		return;
	}

	IModuleInterface* ModuleInterface = FModuleManager::Get().LoadModule("OpenXRHandTracking");
	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	if (ModularFeatures.IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
	{
		HandTracker = &IModularFeatures::Get().GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());
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
	LEAP_QUATERNION Ret = {0};

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

// FOccluderVertexArray is really an array of vectors, don't know why this type was used in UE
void FOpenXRToLeapWrapper::ConvertToLeapSpace(
	LEAP_HAND& LeapHand, const FOccluderVertexArray& Positions, const TArray<FQuat>& Rotations)
{
	if (!XRTrackingSystem)
	{
		return;
	}
	// Enums for each bone are in EHandKeypoint
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
	FOccluderVertexArray OutPositions[2];
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
	}
	if (StatusRight)
	{
		ConvertToLeapSpace(DummyLeapHands[1], OutPositions[1], OutRotations[1]);
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