// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LeapMotionData.h"
#include "LeapC.h"
#include "LeapUtility.h"

#define MAX_DIGITS 5	//almost all humans have 5?
#define MAX_DIGIT_BONES 4	//some bones don't have all bones, see Leap documentation

FLeapHandData FLeapFrameData::HandForId(int32 HandId)
{
	for (auto& Hand : Hands)
	{
		//if found return hand
		if (Hand.Id == HandId)
		{
			return Hand;
		}
	}
	//not found? return an empty hand
	FLeapHandData EmptyHand;
	return EmptyHand;
}

void FLeapFrameData::SetFromLeapFrame(struct _LEAP_TRACKING_EVENT* frame, bool bAddHMDOffset)
{
	if (frame == nullptr)
	{
		return;
	}

	//Copy basics
	NumberOfHandsVisible = frame->nHands;
	FrameRate = frame->framerate;

	TimeStamp = frame->info.timestamp;

	//Copy hand data
	if (Hands.Num() != NumberOfHandsVisible)	//always clear the hand data if number of hands changed
	{
		Hands.Empty();
	}

	LeftHandVisible = false;
	RightHandVisible = false;

	for (int i = 0; i < NumberOfHandsVisible; i++)
	{
		//Expand as necessary to fit
		if (Hands.Num() <= i) 
		{
			FLeapHandData HandData;
			Hands.Add(HandData);
		}

		const LEAP_HAND& LeapHand = frame->pHands[i];
		Hands[i].SetFromLeapHand((_LEAP_HAND*)&LeapHand, bAddHMDOffset);

		if (Hands[i].HandType == EHandType::LEAP_HAND_LEFT)
		{
			LeftHandVisible = true;
		}
		else if (Hands[i].HandType == EHandType::LEAP_HAND_RIGHT)
		{
			RightHandVisible= true;
		}
	}

	FrameId = frame->tracking_frame_id;
}

void FLeapFrameData::SetInterpolationPartialFromLeapFrame(struct _LEAP_TRACKING_EVENT* frame, bool bAddHMDOffset)
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
		Hands[i].SetArmPartialsFromLeapHand((_LEAP_HAND*)&LeapHand, bAddHMDOffset);
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

void FLeapHandData::SetFromLeapHand(struct _LEAP_HAND* hand, bool bAddHMDOffset)
{
	Arm.SetFromLeapBone((_LEAP_BONE*)&hand->arm);
	Confidence = hand->confidence;
	GrabAngle = hand->grab_angle;
	GrabStrength = hand->grab_strength;
	Id = hand->id;

	for (int i = 0; i < MAX_DIGITS; i++)
	{
		if (Digits.Num() <= i) //will only pay the cost of filling once
		{
			FLeapDigitData DigitData;
			Digits.Add(DigitData);
		}
		Digits[i].SetFromLeapDigit((_LEAP_DIGIT*)&hand->digits[i], bAddHMDOffset);
	}

	Flags = hand->flags;

	Index.SetFromLeapDigit((_LEAP_DIGIT*)&hand->index, bAddHMDOffset);
	Middle.SetFromLeapDigit((_LEAP_DIGIT*)&hand->middle, bAddHMDOffset);
	Pinky.SetFromLeapDigit((_LEAP_DIGIT*)&hand->pinky, bAddHMDOffset);
	Ring.SetFromLeapDigit((_LEAP_DIGIT*)&hand->ring, bAddHMDOffset);
	Thumb.SetFromLeapDigit((_LEAP_DIGIT*)&hand->thumb, bAddHMDOffset);

	PinchDistance = FLeapUtility::ScaleLeapFloatToUE(hand->pinch_distance);
	PinchStrength = hand->pinch_strength;

	HandType = (EHandType)hand->type;

	Palm.SetFromLeapPalm((_LEAP_PALM*)&hand->palm, bAddHMDOffset);

	VisibleTime = ((double)hand->visible_time / 1000000.0);	//convert to seconds
}

void FLeapHandData::SetArmPartialsFromLeapHand(struct _LEAP_HAND* hand, bool bAddHMDOffset)
{
	//Arm Partial
	Arm.NextJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(hand->arm.next_joint, bAddHMDOffset);
	Arm.PrevJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(hand->arm.prev_joint, bAddHMDOffset);

	//Palm Partial
	Palm.Position = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(hand->palm.position, bAddHMDOffset);
	Palm.StabilizedPosition = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(hand->palm.stabilized_position, bAddHMDOffset);

	//Debug - Set Orientation
	//Palm.Direction = ConvertLeapVectorToFVector(hand->palm.direction);
	//Palm.Normal = ConvertLeapVectorToFVector(hand->palm.normal);
	//Palm.Orientation = FRotationMatrix::MakeFromXZ(Palm.Direction, -Palm.Normal).Rotator();
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
}

void FLeapHandData::TranslateHand(const FVector& InTranslation)
{
	Arm.TranslateBone(InTranslation);
	Palm.TranslatePalm(InTranslation);

	Index.TranslateDigit(InTranslation);
	Middle.TranslateDigit(InTranslation);
	Pinky.TranslateDigit(InTranslation);
	Ring.TranslateDigit(InTranslation);
	Thumb.TranslateDigit(InTranslation);
}

void FLeapBoneData::SetFromLeapBone(struct _LEAP_BONE* bone, bool bAddHMDOffset)
{
	NextJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(bone->next_joint, bAddHMDOffset);
	PrevJoint = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(bone->prev_joint, bAddHMDOffset);
	Rotation = FLeapUtility::ConvertToFQuatWithHMDOffsets(bone->rotation, bAddHMDOffset).Rotator();
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

void FLeapDigitData::SetFromLeapDigit(struct _LEAP_DIGIT* digit, bool bAddHMDOffset)
{
	//set bone data
	for (int i = 0; i < MAX_DIGIT_BONES; i++)
	{
		if (Bones.Num() <= i)  //will only pay the cost of filling once
		{
			FLeapBoneData BoneData;
			Bones.Add(BoneData);
		}
		Bones[i].SetFromLeapBone((_LEAP_BONE*)&digit->bones[i], bAddHMDOffset);
	}

	Distal.SetFromLeapBone((_LEAP_BONE*)&digit->distal, bAddHMDOffset);
	Intermediate.SetFromLeapBone((_LEAP_BONE*)&digit->intermediate, bAddHMDOffset);
	Metacarpal.SetFromLeapBone((_LEAP_BONE*)&digit->metacarpal, bAddHMDOffset);
	Proximal.SetFromLeapBone((_LEAP_BONE*)&digit->proximal, bAddHMDOffset);
	
	FingerId = digit->finger_id;
	IsExtended = digit->is_extended == 1;
}

void FLeapDigitData::ScaleDigit(float InScale)
{
	/*for (auto& Bone : Bones)	//should the array also be used?
	{
		Bone.ScaleByInScale(InScale);
	}*/

	Distal.ScaleBone(InScale);
	Intermediate.ScaleBone(InScale);
	Metacarpal.ScaleBone(InScale);
	Proximal.ScaleBone(InScale);
}

void FLeapDigitData::RotateDigit(const FRotator& InRotation)
{
	/*for (auto& Bone : Bones)
	{
		Bone.RotateBone(InRotation);
	}*/

	Distal.RotateBone(InRotation);
	Intermediate.RotateBone(InRotation);
	Metacarpal.RotateBone(InRotation);
	Proximal.RotateBone(InRotation);
}

void FLeapDigitData::TranslateDigit(const FVector& InTranslation)
{
	Distal.TranslateBone(InTranslation);
	Intermediate.TranslateBone(InTranslation);
	Metacarpal.TranslateBone(InTranslation);
	Proximal.TranslateBone(InTranslation);
}

void FLeapPalmData::SetFromLeapPalm(struct _LEAP_PALM* palm, bool bAddHMDOffset)
{
	Direction = FLeapUtility::ConvertLeapVectorToFVector(palm->direction);

	Normal = FLeapUtility::ConvertLeapVectorToFVector(palm->normal);

	Orientation = FRotationMatrix::MakeFromXZ(Direction, -Normal).Rotator();	//normal*-1.f

	Position = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(palm->position, bAddHMDOffset);

	StabilizedPosition = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(palm->stabilized_position, bAddHMDOffset);

	Velocity = FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(palm->velocity, bAddHMDOffset);

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
	//Velocity += InTranslation;
}

FLeapOptions::FLeapOptions()
{
	//Good Vive settings used as defaults
	Mode = LEAP_MODE_UNSET;
	TrackingFidelity = LEAP_NORMAL;
	LeapServiceLogLevel = LEAP_LOG_INFO;	//most verbose by default
	bUseTimeWarp = true;
	bUseInterpolation = true;
	bTransformOriginToHMD = true;
	TimewarpOffset = 5500;
	TimewarpFactor = 1.f;
	HandInterpFactor = 0.f;
	FingerInterpFactor = 0.f;
	HMDPositionOffset = FVector(9.0, 0, 0);		//Vive default, for oculus use 8,0,0
	HMDRotationOffset = FRotator(0, 0, 0);		//If imperfectly mounted it might need to sag
	bEnableImageStreaming = false;		//default image streaming to off
}

FLeapStats::FLeapStats()
{
	
}

void FLeapDeviceProperties::SetFromLeapDevice(struct _LEAP_DEVICE_INFO* LeapInfo)
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

FLeapDeviceSettings::FLeapDeviceSettings()
{
	bAddHMDOrigin = true;
	bShouldInterpolate = true;
	ConfidenceBoost = 0.f;
	bMergeBSLeftHand = true;
	bMergeBSRightHand = true;
	bMergeBSLerpBias = 1.f;
}
