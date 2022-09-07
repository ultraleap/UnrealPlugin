/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "UltraleapTrackingData.h"

#include "LeapC.h"
#include "LeapUtility.h"

#define MAX_DIGITS 5		 // almost all humans have 5?
#define MAX_DIGIT_BONES 4	 // some bones don't have all bones, see Leap documentation

FLeapHandData FLeapFrameData::HandForId(int32 HandId)
{
	for (auto& Hand : Hands)
	{
		// if found return hand
		if (Hand.Id == HandId)
		{
			return Hand;
		}
	}
	// not found? return an empty hand
	FLeapHandData EmptyHand;
	return EmptyHand;
}

void FLeapFrameData::SetFromLeapFrame(
	struct _LEAP_TRACKING_EVENT* frame, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	if (frame == nullptr)
	{
		return;
	}

	// Copy basics
	NumberOfHandsVisible = frame->nHands;
	FrameRate = frame->framerate;

	TimeStamp = frame->info.timestamp;

	// Copy hand data
	if (Hands.Num() != NumberOfHandsVisible)	// always clear the hand data if number of hands changed
	{
		Hands.Empty();
	}

	LeftHandVisible = false;
	RightHandVisible = false;

	for (int i = 0; i < NumberOfHandsVisible; i++)
	{
		// Expand as necessary to fit
		if (Hands.Num() <= i)
		{
			FLeapHandData HandData;
			Hands.Add(HandData);
		}

		const LEAP_HAND& LeapHand = frame->pHands[i];
		Hands[i].SetFromLeapHand((_LEAP_HAND*) &LeapHand, LeapMountTranslationOffset, LeapMountRotationOffset);

		if (Hands[i].HandType == EHandType::LEAP_HAND_LEFT)
		{
			LeftHandVisible = true;
		}
		else if (Hands[i].HandType == EHandType::LEAP_HAND_RIGHT)
		{
			RightHandVisible = true;
		}
	}

	FrameId = frame->tracking_frame_id;
}

void FLeapFrameData::SetInterpolationPartialFromLeapFrame(
	struct _LEAP_TRACKING_EVENT* frame, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	if (frame == nullptr)
	{
		return;
	}

	if (NumberOfHandsVisible != frame->nHands)
	{
		return;
	}

	for (int i = 0; i < NumberOfHandsVisible; i++)
	{
		const LEAP_HAND& LeapHand = frame->pHands[i];
		Hands[i].SetArmPartialsFromLeapHand(
			(_LEAP_HAND*) &LeapHand, LeapMountTranslationOffset, LeapMountRotationOffset);
	}

	TimeStamp = frame->info.timestamp;
}

void FLeapFrameData::ScaleFrame(float InScale)
{
	for (auto& Hand : Hands)
	{
		Hand.ScaleHand(InScale);
	}
}

void FLeapFrameData::RotateFrame(const FRotator& InRotation)
{
	for (auto& Hand : Hands)
	{
		Hand.RotateHand(InRotation);
	}
}

void FLeapFrameData::TranslateFrame(const FVector& InTranslation)
{
	for (auto& Hand : Hands)
	{
		Hand.TranslateHand(InTranslation);
	}
}
void FLeapHandData::InitFromEmpty(const EHandType HandTypeIn, const int HandID)
{
	static int FingerID = 0;

	Confidence = 1.0;
	GrabAngle = 100;
	GrabStrength = 0.5;
	PinchStrength = 0.5;

	Id = HandID;

	for (int i = 0; i < MAX_DIGITS; i++)
	{
		if (Digits.Num() <= i)	  // will only pay the cost of filling once
		{
			FLeapDigitData DigitData;
			DigitData.Bones.AddZeroed(4);
			DigitData.FingerId = ++FingerID;
			Digits.Add(DigitData);
		}
	}

	PinchDistance = 50;
	

	HandType = HandTypeIn;

	VisibleTime = 1;

	Flags = 0;
}
void FLeapHandData::UpdateFromDigits()
{
	// The hand merger only sets the bone arrays
	// Set the high level digits and digit members here
	for (auto& Digit : Digits)
	{
		Digit.Metacarpal = Digit.Bones[0];
		Digit.Proximal = Digit.Bones[1];
		Digit.Intermediate = Digit.Bones[2];
		Digit.Distal = Digit.Bones[3];
	
		// this could be a merged state
		Digit.IsExtended = false;
	}

	Thumb = Digits[0];
	Index = Digits[1];
	Middle = Digits[2];
	Ring = Digits[3];
	Pinky = Digits[4];
	
}
void FLeapHandData::SetFromLeapHand(
	struct _LEAP_HAND* hand, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	Arm.SetFromLeapBone((_LEAP_BONE*) &hand->arm, LeapMountTranslationOffset, LeapMountRotationOffset);
	Confidence = hand->confidence;
	GrabAngle = hand->grab_angle;
	GrabStrength = hand->grab_strength;
	Id = hand->id;

	for (int i = 0; i < MAX_DIGITS; i++)
	{
		if (Digits.Num() <= i)	  // will only pay the cost of filling once
		{
			FLeapDigitData DigitData;
			Digits.Add(DigitData);
		}
		Digits[i].SetFromLeapDigit((_LEAP_DIGIT*) &hand->digits[i], LeapMountTranslationOffset, LeapMountRotationOffset);
	}

	Flags = hand->flags;

	Index.SetFromLeapDigit((_LEAP_DIGIT*) &hand->index, LeapMountTranslationOffset, LeapMountRotationOffset);
	Middle.SetFromLeapDigit((_LEAP_DIGIT*) &hand->middle, LeapMountTranslationOffset, LeapMountRotationOffset);
	Pinky.SetFromLeapDigit((_LEAP_DIGIT*) &hand->pinky, LeapMountTranslationOffset, LeapMountRotationOffset);
	Ring.SetFromLeapDigit((_LEAP_DIGIT*) &hand->ring, LeapMountTranslationOffset, LeapMountRotationOffset);
	Thumb.SetFromLeapDigit((_LEAP_DIGIT*) &hand->thumb, LeapMountTranslationOffset, LeapMountRotationOffset);

	PinchDistance = FLeapUtility::ScaleLeapFloatToUE(hand->pinch_distance);
	PinchStrength = hand->pinch_strength;

	HandType = (EHandType) hand->type;

	Palm.SetFromLeapPalm((_LEAP_PALM*) &hand->palm, LeapMountTranslationOffset, LeapMountRotationOffset);

	VisibleTime = ((double) hand->visible_time / 1000000.0);	// convert to seconds
}

void FLeapHandData::SetArmPartialsFromLeapHand(struct _LEAP_HAND* hand, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	// Arm Partial
	Arm.NextJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(hand->arm.next_joint,LeapMountTranslationOffset, LeapMountRotationOffset);
	Arm.PrevJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		hand->arm.prev_joint, LeapMountTranslationOffset, LeapMountRotationOffset);

	// Palm Partial
	Palm.Position = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		hand->palm.position, LeapMountTranslationOffset, LeapMountRotationOffset);

	// Debug - Set Orientation
	// Palm.Direction = ConvertLeapVectorToFVector(hand->palm.direction);
	// Palm.Normal = ConvertLeapVectorToFVector(hand->palm.normal);
	// Palm.Orientation = FRotationMatrix::MakeFromXZ(Palm.Direction, -Palm.Normal).Rotator();
}

void FLeapHandData::ScaleHand(float InScale)
{
	Arm.ScaleBone(InScale);

	Index.ScaleDigit(InScale);
	Middle.ScaleDigit(InScale);
	Pinky.ScaleDigit(InScale);
	Ring.ScaleDigit(InScale);
	Thumb.ScaleDigit(InScale);

	Palm.ScalePalm(InScale);
}

void FLeapHandData::RotateHand(const FRotator& InRotation)
{
	Arm.RotateBone(InRotation);

	Index.RotateDigit(InRotation);
	Middle.RotateDigit(InRotation);
	Pinky.RotateDigit(InRotation);
	Ring.RotateDigit(InRotation);
	Thumb.RotateDigit(InRotation);

	Palm.RotatePalm(InRotation);

	for (auto& Digit : Digits)
	{
		Digit.RotateDigit(InRotation);
	}
}

void FLeapHandData::TranslateHand(const FVector& InTranslation)
{
	Arm.TranslateBone(InTranslation);

	Index.TranslateDigit(InTranslation);
	Middle.TranslateDigit(InTranslation);
	Pinky.TranslateDigit(InTranslation);
	Ring.TranslateDigit(InTranslation);
	Thumb.TranslateDigit(InTranslation);

	Palm.TranslatePalm(InTranslation);

	for(auto& Digit : Digits)
	{
		Digit.TranslateDigit(InTranslation);
	}
}

void FLeapBoneData::SetFromLeapBone(
	struct _LEAP_BONE* bone, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	NextJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		bone->next_joint, LeapMountTranslationOffset, LeapMountRotationOffset);
	PrevJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		bone->prev_joint, LeapMountTranslationOffset, LeapMountRotationOffset);
	Rotation = FLeapUtility::ConvertToFQuatWithHMDOffsets(bone->rotation,LeapMountRotationOffset).Rotator();
	Width = FLeapUtility::ScaleLeapFloatToUE(bone->width);
}

void FLeapBoneData::ScaleBone(float InScale)
{
	NextJoint *= InScale;
	PrevJoint *= InScale;
}

void FLeapBoneData::RotateBone(const FRotator& InRotation)
{
	NextJoint = InRotation.RotateVector(NextJoint);
	PrevJoint = InRotation.RotateVector(PrevJoint);
	Rotation = FLeapUtility::CombineRotators(Rotation, InRotation);
}

void FLeapBoneData::TranslateBone(const FVector& InTranslation)
{
	NextJoint += InTranslation;
	PrevJoint += InTranslation;
}

void FLeapDigitData::SetFromLeapDigit(
	struct _LEAP_DIGIT* digit, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	// set bone data
	for (int i = 0; i < MAX_DIGIT_BONES; i++)
	{
		if (Bones.Num() <= i)	 // will only pay the cost of filling once
		{
			FLeapBoneData BoneData;
			Bones.Add(BoneData);
		}
		Bones[i].SetFromLeapBone((_LEAP_BONE*) &digit->bones[i], LeapMountTranslationOffset, LeapMountRotationOffset);
	}

	Distal.SetFromLeapBone((_LEAP_BONE*) &digit->distal, LeapMountTranslationOffset, LeapMountRotationOffset);
	Intermediate.SetFromLeapBone((_LEAP_BONE*) &digit->intermediate, LeapMountTranslationOffset, LeapMountRotationOffset);
	Metacarpal.SetFromLeapBone((_LEAP_BONE*) &digit->metacarpal, LeapMountTranslationOffset, LeapMountRotationOffset);
	Proximal.SetFromLeapBone((_LEAP_BONE*) &digit->proximal, LeapMountTranslationOffset, LeapMountRotationOffset);

	FingerId = digit->finger_id;
	IsExtended = digit->is_extended == 1;
}

void FLeapDigitData::ScaleDigit(float InScale)
{
	Distal.ScaleBone(InScale);
	Intermediate.ScaleBone(InScale);
	Metacarpal.ScaleBone(InScale);
	Proximal.ScaleBone(InScale);

	for (auto& Bone : Bones)
	{
		Bone.ScaleBone(InScale);
	}
}

void FLeapDigitData::RotateDigit(const FRotator& InRotation)
{
	Distal.RotateBone(InRotation);
	Intermediate.RotateBone(InRotation);
	Metacarpal.RotateBone(InRotation);
	Proximal.RotateBone(InRotation);

	for (auto& Bone : Bones)
	{
		Bone.RotateBone(InRotation);
	}
}

void FLeapDigitData::TranslateDigit(const FVector& InTranslation)
{
	Distal.TranslateBone(InTranslation);
	Intermediate.TranslateBone(InTranslation);
	Metacarpal.TranslateBone(InTranslation);
	Proximal.TranslateBone(InTranslation);

	for (auto& Bone : Bones)
	{
		Bone.TranslateBone(InTranslation);
	}
}

void FLeapPalmData::SetFromLeapPalm(
	struct _LEAP_PALM* palm, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset)
{
	Direction = FLeapUtility::ConvertLeapVectorToFVector(palm->direction);

	Normal = FLeapUtility::ConvertLeapVectorToFVector(palm->normal);

	Orientation = FLeapUtility::ConvertLeapQuatToFQuat(palm->orientation).Rotator();

	Position = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		palm->position, LeapMountTranslationOffset, LeapMountRotationOffset);

	StabilizedPosition = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		palm->stabilized_position, LeapMountTranslationOffset, LeapMountRotationOffset);

	Velocity = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(
		palm->velocity, LeapMountTranslationOffset, LeapMountRotationOffset);

	Width = FLeapUtility::ScaleLeapFloatToUE(palm->width);
}

void FLeapPalmData::ScalePalm(float InScale)
{
	Position *= InScale;
	StabilizedPosition *= InScale;
	Velocity *= InScale;
}

void FLeapPalmData::RotatePalm(const FRotator& InRotation)
{
	Position = InRotation.RotateVector(Position);
	StabilizedPosition = InRotation.RotateVector(StabilizedPosition);
	Velocity = InRotation.RotateVector(Velocity);
	Direction = InRotation.RotateVector(Direction);
	Normal = InRotation.RotateVector(Normal);
	Orientation = FLeapUtility::CombineRotators(Orientation, InRotation);
}

void FLeapPalmData::TranslatePalm(const FVector& InTranslation)
{
	Position += InTranslation;
	StabilizedPosition += InTranslation;
	// Velocity += InTranslation;
}

FLeapOptions::FLeapOptions()
{
	// Good Vive settings used as defaults
	Mode = LEAP_MODE_DESKTOP;
	TrackingFidelity = LEAP_NORMAL;
	LeapServiceLogLevel = LEAP_LOG_INFO;	// most verbose by default
	bUseTimeWarp = true;
	bUseInterpolation = true;
	bTransformOriginToHMD = true;
	TimewarpOffset = 5500;
	TimewarpFactor = 1.f;
	HandInterpFactor = 0.f;
	FingerInterpFactor = 0.f;
	// in mm
//	HMDPositionOffset = FVector(90.0, 0, 0);	// Vive default, for oculus use 80,0,0
//	HMDRotationOffset = FRotator(0, 0, 0);		// If imperfectly mounted it might need to sag
	bUseFrameBasedGestureDetection = false;
	StartGrabThreshold = .8f;
	EndGrabThreshold = .5f;
	StartPinchThreshold = .8f;
	EndPinchThreshold = .5f;
	GrabTimeout = 100000;
	PinchTimeout = 100000;
	bUseOpenXRAsSource = false;

	HMDPositionOffset = FVector(80.f, 0, 0);
	HMDRotationOffset = FRotator(0, 0, 0);
	// bEnableImageStreaming = false;		//default image streaming to off
}

FLeapStats::FLeapStats() : FrameExtrapolationInMS(0)
{
}

void FLeapDevice::SetFromLeapDevice(struct _LEAP_DEVICE_INFO* LeapInfo)
{
	Status = LeapInfo->status;
	Caps = LeapInfo->caps;
	PID = FString(LeapDevicePIDToString(LeapInfo->pid));
	Baseline = LeapInfo->baseline;
	Serial = LeapInfo->serial;
	HorizontalFOV = LeapInfo->h_fov;
	VerticalFOV = LeapInfo->v_fov;
	Range = LeapInfo->range;
}
