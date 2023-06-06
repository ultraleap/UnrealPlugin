/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapCombinedDevice.h"

int FUltraleapCombinedDevice::HandID = 0;



FUltraleapCombinedDevice::FUltraleapCombinedDevice(IHandTrackingWrapper* LeapDeviceWrapper,
	ITrackingDeviceWrapper* TrackingDeviceWrapperIn, TArray<IHandTrackingWrapper*> DevicesToCombineIn) : 
	FUltraleapDevice(LeapDeviceWrapper, TrackingDeviceWrapperIn),
	DevicesToCombine(DevicesToCombineIn)
{
	// static
	++HandID;
}

FUltraleapCombinedDevice::~FUltraleapCombinedDevice()
{
}
void FUltraleapCombinedDevice::TransformFrame(
	FLeapFrameData& OutData, const FVector& TranslationOffset, const FRotator& RotationOffset)
{	
	OutData.RotateFrame(RotationOffset);
	OutData.TranslateFrame(TranslationOffset);
}
// Main loop event emitter and handler
void FUltraleapCombinedDevice::SendControllerEvents()
{
	// Create combined frame here and call parse
	// the parent class will then behave as if it had one device
	TArray<FLeapFrameData> SourceFrames;
	FTransform VRDeviceOrigin;

	bool AreAnyVR = false;
	for (auto SourceDevice : DevicesToCombine)
	{
		auto InternalSourceDevice = SourceDevice->GetDevice();
		if (InternalSourceDevice)
		{
			FLeapFrameData SourceFrame;

			const bool IsVR = InternalSourceDevice->GetOptions().Mode == LEAP_MODE_VR;
			if (IsVR)
			{
				AreAnyVR = true;
				VRDeviceOrigin = InternalSourceDevice->GetDeviceOrigin();
				break;
			}
		}
	}
	// add combiner logic based on DevicesToCombine List. All devices will have ticked before this is called
	for (auto SourceDevice : DevicesToCombine)
	{
		auto InternalSourceDevice = SourceDevice->GetDevice();
		if (InternalSourceDevice)
		{
			FLeapFrameData SourceFrame;

			// For VR/XR mounted devices, the frame here is already transformed by the HMD position
			// so we don't want to re-apply the device origin as this will transform it twice
			// BUT the device origin is still required for confidence calcs
			const bool IsVR = InternalSourceDevice->GetOptions().Mode == LEAP_MODE_VR;
			const bool IsScreenTop = InternalSourceDevice->GetOptions().Mode == LEAP_MODE_SCREENTOP;

			InternalSourceDevice->GetLatestFrameData(SourceFrame, !IsVR);
			
			if (IsVR)
			{
				// Transform HMD into Desktop rotation
				FRotator Rotation(90, 0, 180);
				FUltraleapCombinedDevice::TransformFrame(
					SourceFrame,  VRDeviceOrigin.GetLocation(), Rotation.GetInverse());
			}
			// comment in for debugging desktop devices only in the combined hand -> 
			//if (IsScreenTop)
			{
				SourceFrames.Add(SourceFrame);
			}
		}
	}
	
	CombineFrame(SourceFrames);

	if (AreAnyVR)
	{
		// from desktop rotation to HMD rotation
		FRotator Rotation(90, 0, 180);
		FUltraleapCombinedDevice::TransformFrame(CurrentFrame,  -Rotation.RotateVector(VRDeviceOrigin.GetLocation()), Rotation);
	}
	

	ParseEvents();
}
FVector ToLocal(const FVector& WorldPoint,const FVector& LocalOrigin,const FQuat& LocalRot)
{
	return LocalRot.Inverse() * (WorldPoint - LocalOrigin);
}
FVector ToWorld(const FVector& LocalPoint,const FVector& LocalOrigin,const FQuat& LocalRot)
{
	return (LocalRot * LocalPoint) + LocalOrigin;
}
// create local joint list from Hand (relative to palm)
void FUltraleapCombinedDevice::CreateLocalLinearJointList(const FLeapHandData& Hand, TArray<FVector>& JointPositions)
{
	FVector PalmPos = Hand.Palm.Position;
	FQuat PalmRot = Hand.Palm.Orientation.Quaternion();

	int BoneIdx = 0;
	static const int NumFingers = 5;
	for (int i = 0; i < NumFingers; i++)
	{
		FVector BaseMetacarpal = ToLocal(Hand.Digits[i].Bones[0].PrevJoint, PalmPos, PalmRot);
		JointPositions[BoneIdx++] = BaseMetacarpal;
		
		static const int NumBones = 4;
		for (int j = 0; j < NumBones; j++)
		{
			FVector Joint = ToLocal(Hand.Digits[i].Bones[j].NextJoint, PalmPos, PalmRot);
			JointPositions[BoneIdx++] = Joint;
		}
	}
}


FLeapBoneData* GetBone( FLeapHandData& Hand, const int BoneIndex)
{
	return &Hand.Digits[BoneIndex / 4].Bones[BoneIndex % 4];
}
void FillBone(FLeapBoneData& Bone, const FVector& PrevJoint, const FVector& NextJoint,
	const float Width,const FQuat& Rotation)
{
	Bone.PrevJoint = PrevJoint;
	Bone.NextJoint = NextJoint;
	Bone.Rotation = Rotation.Rotator();
	Bone.Width = Width;
}

// Construct a hand from local space bones (relative to palm to relative to world)
void FUltraleapCombinedDevice::ConvertToWorldSpaceHand(
	FLeapHandData& Hand, const bool IsLeft, const FVector& PalmPos, const FQuat& PalmRot, const TArray<FVector>& JointPositions)
{
	// Create data structure
	EHandType HandType = EHandType::LEAP_HAND_RIGHT;
	int HandIDToSet = HandID*2;
	if (IsLeft)
	{
		HandIDToSet++;
		HandType = EHandType::LEAP_HAND_LEFT;
	}
	Hand.InitFromEmpty(HandType, HandIDToSet);

	int BoneIdx = 0;
	FVector PrevJoint = FVector::ZeroVector;
	FVector NextJoint = FVector::ZeroVector;
	FQuat BoneRot = FQuat::Identity;

	// Fill fingers.
	static const int NumFingers = 5;
	for (int FingerIdx = 0; FingerIdx < NumFingers; FingerIdx++)
	{
		static const int NumJoints = 4;
		for (int JointIdx = 0; JointIdx < NumJoints; JointIdx++)
		{
			BoneIdx = FingerIdx * NumJoints + JointIdx;
			PrevJoint = JointPositions[FingerIdx * NumFingers + JointIdx];
			NextJoint = JointPositions[FingerIdx * NumFingers + JointIdx + 1];

			FVector Normalized(NextJoint - PrevJoint);
			Normalized.Normalize();
			
			if (Normalized == FVector::ZeroVector)
			{
				// Thumb "metacarpal" slot is an identity bone.
				BoneRot = FQuat::Identity;
			}
			else
			{
				FVector Cross = FVector::CrossProduct(Normalized,
					(FingerIdx == 0 ? (IsLeft ? -FVector::ForwardVector : FVector::ForwardVector) : FVector::RightVector));

				Cross.Normalize();

				// Equivalent of LookRotation in Unity
				BoneRot = FRotationMatrix::MakeFromXZ(Normalized, Cross).ToQuat();
			}

			// Convert to world space from palm space.
			NextJoint = ToWorld(NextJoint, PalmPos, PalmRot);
			PrevJoint = ToWorld(PrevJoint, PalmPos, PalmRot);
			BoneRot = PalmRot * BoneRot;

			FillBone(*GetBone(Hand,BoneIdx), PrevJoint, NextJoint
										   , 1, 
										   BoneRot);
		}
	}
	// Copy digits we just filled into top level data structures
	Hand.UpdateFromDigits();

	// Fill arm data.
	// TODO: this is hardwired and would be better off using tracked elbow positions from the leapdata
	FillBone(Hand.Arm, ToWorld(FVector(-30.0f, 0.0f, 0), PalmPos, PalmRot), ToWorld(FVector(-5.5f, 0.0f, 0), PalmPos, PalmRot),
		5,PalmRot);
	
	// Palm members
	// Up vector as in LeapSpace Z is forwards and these are UE space APIs
	Hand.Palm.Direction = PalmRot.GetUpVector();
	Hand.Palm.Normal = FVector::ZeroVector;		// Unused elsewhere
	Hand.Palm.Orientation = PalmRot.Rotator();
	Hand.Palm.Position = PalmPos;
	Hand.Palm.StabilizedPosition = PalmPos;
	Hand.Palm.Velocity = PalmPos;	// why palmpos in unity?
	Hand.Palm.Width = 8.5;
}
void FUltraleapCombinedDevice::CreateLinearJointListInterp(
	const FLeapHandData& HandA, const FLeapHandData& HandB, TArray<FVector>& Joints, const float Alpha, FVector& PalmPos, FQuat& PalmRot)
{
	TArray<FVector> JointsA;
	TArray<FVector> JointsB;

	Joints.Empty();
	Joints.AddZeroed(NumJointPositions);

	JointsA.AddZeroed(NumJointPositions);
	JointsB.AddZeroed(NumJointPositions);

	CreateLocalLinearJointList(HandA, JointsA);
	CreateLocalLinearJointList(HandB, JointsB);

	if (HandA.HandType != HandB.HandType)
	{
		UE_LOG(UltraleapTrackingLog, Log, TEXT("FUltraleapCombinedDevice::CreateLinearJointListInterp hands have mixed chirality"));
		return;
	}
	const bool IsLeft = HandA.HandType == LEAP_HAND_LEFT;

	PalmPos = FMath::Lerp(HandA.Palm.Position, HandB.Palm.Position, Alpha);
	PalmRot = FQuat::FastLerp(HandA.Palm.Orientation.Quaternion(), HandB.Palm.Orientation.Quaternion(), Alpha);
	
	for (int i = 0; i < Joints.Num(); i++)
	{
		Joints[i] = FMath::Lerp(JointsA[i], JointsB[i], Alpha);
	}
	return;
}
FTransform FUltraleapCombinedDevice::GetSourceDeviceOrigin(const int ProviderIndex)
{
	return DevicesToCombine[ProviderIndex]->GetDevice()->GetDeviceOrigin();
}