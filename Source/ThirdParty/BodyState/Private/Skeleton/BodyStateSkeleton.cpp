/*************************************************************************************************************************************
 *The MIT License(MIT)
 *
 *Copyright(c) 2016 Jan Kaniewski(Getnamo)
 *Modified work Copyright(C) 2019 - 2021 Ultraleap, Inc.
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 *files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
 *merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions :
 *
 *The above copyright notice and this permission notice shall be included in all copies or
 *substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 *FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *************************************************************************************************************************************/

#include "Skeleton/BodyStateSkeleton.h"

#include "BodyStateUtility.h"

UBodyStateSkeleton::UBodyStateSkeleton(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Todo: build

	// add a bone for each possible bone in the skeleton
	for (int i = 0; i < (int32) EBodyStateBasicBoneType::BONES_COUNT; i++)
	{
		// Make bone
		const FString BoneName = FBodyStateUtility::EnumToString(TEXT("EBodyStateBasicBoneType"), (EBodyStateBasicBoneType) i);
		auto Bone = NewObject<UBodyStateBone>(this, *FString::Printf(TEXT("%s-%d"), *BoneName, i));

		Bone->Name = BoneName;
		Bone->BoneType = (EBodyStateBasicBoneType) i;
		// Add bone
		Bones.Add(Bone);
	}

	// Setup parent-child links

	// Torso
	Bones[(int32) EBodyStateBasicBoneType::BONE_ROOT]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_PELVIS]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_PELVIS]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_1]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_1]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_2]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_2]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_3]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_3]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_CLAVICLE_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_3]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_CLAVICLE_R]);

	// Head
	Bones[(int32) EBodyStateBasicBoneType::BONE_SPINE_3]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_NECK_1]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_NECK_1]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_HEAD]);

	// Left Leg
	Bones[(int32) EBodyStateBasicBoneType::BONE_PELVIS]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_THIGH_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_THIGH_L]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_CALF_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_CALF_L]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_FOOT_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_FOOT_L]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_BALL_L]);

	// Right Leg
	Bones[(int32) EBodyStateBasicBoneType::BONE_PELVIS]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_THIGH_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_THIGH_R]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_CALF_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_CALF_R]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_FOOT_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_FOOT_R]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_BALL_R]);

	// Left Arm
	Bones[(int32) EBodyStateBasicBoneType::BONE_CLAVICLE_L]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_UPPERARM_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_UPPERARM_L]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_LOWERARM_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_LOWERARM_L]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L]);

	// Left Hand

	// Thumb
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_L]);

	// Index
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_L]);

	// Middle
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_L]);

	// Ring
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_3_DISTAL_L]);

	// Pinky
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_L]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_L]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_L]);

	// Right Arm
	Bones[(int32) EBodyStateBasicBoneType::BONE_CLAVICLE_R]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_UPPERARM_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_UPPERARM_R]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_LOWERARM_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_LOWERARM_R]->AddChild(Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R]);

	// Right Hand

	// Thumb
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R]);

	// Index
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_R]);

	// Middle
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_R]);

	// Ring
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_RING_3_DISTAL_R]);

	// Pinky
	Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_R]);
	Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_R]->AddChild(
		Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_R]);

}

UBodyStateBone* UBodyStateSkeleton::RootBone()
{
	return Bones[(int32) EBodyStateBasicBoneType::BONE_ROOT];
}

UBodyStateArm* UBodyStateSkeleton::LeftArm()
{
	if (!PrivateLeftArm)
	{
		// Allocate
		PrivateLeftArm = NewObject<UBodyStateArm>(this, "LeftArm");
		PrivateLeftArm->AddToRoot();
		// Linkup
		PrivateLeftArm->LowerArm = Bones[(int32) EBodyStateBasicBoneType::BONE_LOWERARM_L];
		PrivateLeftArm->UpperArm = Bones[(int32) EBodyStateBasicBoneType::BONE_UPPERARM_L];

		// Hand
		PrivateLeftArm->Hand = NewObject<UBodyStateHand>(PrivateLeftArm, "LeftHand");
		PrivateLeftArm->Hand->Wrist = Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L];
		PrivateLeftArm->Hand->Palm =
			Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_L];	  // this should have some offset from wrist...

		UBodyStateFinger* ThumbFinger = NewObject<UBodyStateFinger>(PrivateLeftArm->Hand, "LeftThumbFinger");
		ThumbFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_L];
		ThumbFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_L];
		ThumbFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_L];
		ThumbFinger->Intermediate =
			Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_L];	  // set intermediate to distal too for thumb

		UBodyStateFinger* IndexFinger = NewObject<UBodyStateFinger>(PrivateLeftArm->Hand, "LeftIndexFinger");
		IndexFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_L];
		IndexFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_L];
		IndexFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_L];
		IndexFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_L];

		UBodyStateFinger* MiddleFinger = NewObject<UBodyStateFinger>(PrivateLeftArm->Hand, "LeftMiddleFinger");
		MiddleFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_L];
		MiddleFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_L];
		MiddleFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_L];
		MiddleFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_L];

		UBodyStateFinger* RingFinger = NewObject<UBodyStateFinger>(PrivateLeftArm->Hand, "LeftRingFinger");
		RingFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_L];
		RingFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_L];
		RingFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_L];
		RingFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_3_DISTAL_L];

		UBodyStateFinger* PinkyFinger = NewObject<UBodyStateFinger>(PrivateLeftArm->Hand, "LeftPinkyFinger");
		PinkyFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_L];
		PinkyFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_L];
		PinkyFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_L];
		PinkyFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_L];

		PrivateLeftArm->Hand->Fingers.Add(ThumbFinger);
		PrivateLeftArm->Hand->Fingers.Add(IndexFinger);
		PrivateLeftArm->Hand->Fingers.Add(MiddleFinger);
		PrivateLeftArm->Hand->Fingers.Add(RingFinger);
		PrivateLeftArm->Hand->Fingers.Add(PinkyFinger);
	}
	return PrivateLeftArm;
}

UBodyStateArm* UBodyStateSkeleton::RightArm()
{
	if (!PrivateRightArm)
	{
		// Allocate
		PrivateRightArm = NewObject<UBodyStateArm>(this, "RightArm");
		PrivateRightArm->AddToRoot();
		// Linkup
		PrivateRightArm->LowerArm = Bones[(int32) EBodyStateBasicBoneType::BONE_LOWERARM_R];
		PrivateRightArm->UpperArm = Bones[(int32) EBodyStateBasicBoneType::BONE_UPPERARM_R];

		// Hand
		PrivateRightArm->Hand = NewObject<UBodyStateHand>(PrivateRightArm, "RightHand");
		PrivateRightArm->Hand->Wrist = Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R];
		PrivateRightArm->Hand->Palm =
			Bones[(int32) EBodyStateBasicBoneType::BONE_HAND_WRIST_R];	  // this should have some offset from wrist...

		UBodyStateFinger* ThumbFinger = NewObject<UBodyStateFinger>(PrivateRightArm->Hand, "RightThumbFinger");
		ThumbFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R];
		ThumbFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_1_PROXIMAL_R];
		ThumbFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R];
		ThumbFinger->Intermediate =
			Bones[(int32) EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R];	  // set intermediate to distal too for thumb

		UBodyStateFinger* IndexFinger = NewObject<UBodyStateFinger>(PrivateRightArm->Hand, "RightIndexFinger");
		IndexFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_R];
		IndexFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_R];
		IndexFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_2_INTERMEDIATE_R];
		IndexFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_R];

		UBodyStateFinger* MiddleFinger = NewObject<UBodyStateFinger>(PrivateRightArm->Hand, "RightMiddleFinger");
		MiddleFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_R];
		MiddleFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_R];
		MiddleFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_R];
		MiddleFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_R];

		UBodyStateFinger* RingFinger = NewObject<UBodyStateFinger>(PrivateRightArm->Hand, "RightRingFinger");
		RingFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_R];
		RingFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_R];
		RingFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_2_INTERMEDIATE_R];
		RingFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_RING_3_DISTAL_R];

		UBodyStateFinger* PinkyFinger = NewObject<UBodyStateFinger>(PrivateRightArm->Hand, "RightPinkyFinger");
		PinkyFinger->Metacarpal = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_R];
		PinkyFinger->Proximal = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_R];
		PinkyFinger->Intermediate = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_2_INTERMEDIATE_R];
		PinkyFinger->Distal = Bones[(int32) EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_R];

		PrivateRightArm->Hand->Fingers.Add(ThumbFinger);
		PrivateRightArm->Hand->Fingers.Add(IndexFinger);
		PrivateRightArm->Hand->Fingers.Add(MiddleFinger);
		PrivateRightArm->Hand->Fingers.Add(RingFinger);
		PrivateRightArm->Hand->Fingers.Add(PinkyFinger);
	}
	return PrivateRightArm;
}

UBodyStateBone* UBodyStateSkeleton::Head()
{
	return Bones[(int32) EBodyStateBasicBoneType::BONE_HEAD];
}

UBodyStateBone* UBodyStateSkeleton::BoneForEnum(EBodyStateBasicBoneType Bone)
{
	int32 BoneIndex = (int32) Bone;
	if (BoneIndex >= 0 && BoneIndex < Bones.Num())
	{
		return Bones[BoneIndex];
	}
	else
	{
		// invalid bone requests return the root bone
		return Bones[(int32) EBodyStateBasicBoneType::BONE_ROOT];
	}
}

class UBodyStateBone* UBodyStateSkeleton::BoneNamed(const FString& InName)
{
	UE_LOG(BodyStateLog, Log, TEXT("Not implemented yet."));
	return nullptr;
}

// All types of bones
TArray<FNamedBoneData> UBodyStateSkeleton::TrackedBoneData()
{
	TArray<FNamedBoneData> ResultArray;

	for (int i = 0; i < Bones.Num(); i++)
	{
		if (Bones[i]->IsTracked())
		{
			FNamedBoneData NamedData;
			NamedData.Data = Bones[i]->BoneData;
			NamedData.Name = EBodyStateBasicBoneType(i);

			ResultArray.Add(NamedData);
		}
	}

	return ResultArray;
}

// Only basic ones
TArray<FKeyedTransform> UBodyStateSkeleton::TrackedBasicBones()
{
	TArray<FKeyedTransform> ResultArray;

	for (int i = 0; i < Bones.Num(); i++)
	{
		if (Bones[i]->IsTracked() && !Bones[i]->BoneData.AdvancedBoneType)
		{
			FKeyedTransform NamedData;
			NamedData.Transform = Bones[i]->BoneData.Transform;
			NamedData.Name = EBodyStateBasicBoneType(i);

			ResultArray.Add(NamedData);
		}
	}

	return ResultArray;
}

// Only advanced ones
TArray<FNamedBoneData> UBodyStateSkeleton::TrackedAdvancedBones()
{
	TArray<FNamedBoneData> ResultArray;

	for (int i = 0; i < Bones.Num(); i++)
	{
		if (Bones[i]->IsTracked() && Bones[i]->BoneData.AdvancedBoneType)
		{
			FNamedBoneData NamedData;
			NamedData.Data = Bones[i]->BoneData;
			NamedData.Name = EBodyStateBasicBoneType(i);

			ResultArray.Add(NamedData);
		}
	}

	return ResultArray;
}

TArray<FNamedBoneMeta> UBodyStateSkeleton::UniqueBoneMetas()
{
	TArray<FNamedBoneMeta> ResultArray;

	for (int i = 0; i < Bones.Num(); i++)
	{
		if (Bones[i]->Meta.ParentDistinctMeta)
		{
			FNamedBoneMeta NamedMeta;
			NamedMeta.Meta = Bones[i]->Meta;
			NamedMeta.Name = EBodyStateBasicBoneType(i);

			ResultArray.Add(NamedMeta);
		}
	}

	return ResultArray;
}

FNamedSkeletonData UBodyStateSkeleton::GetMinimalNamedSkeletonData()
{
	FNamedSkeletonData NamedSkeleton;

	NamedSkeleton.TrackedBasicBones = TrackedBasicBones();
	NamedSkeleton.TrackedAdvancedBones = TrackedAdvancedBones();
	NamedSkeleton.UniqueMetas = UniqueBoneMetas();

	return NamedSkeleton;
}

void UBodyStateSkeleton::ResetToDefaultSkeleton()
{
	for (int i = 0; i < Bones.Num(); i++)
	{
		Bones[i]->Initialize();
	}
}

void UBodyStateSkeleton::SetDataForBone(const FBodyStateBoneData& BoneData, EBodyStateBasicBoneType Bone)
{
	Bones[(int32) Bone]->BoneData = BoneData;
}

void UBodyStateSkeleton::SetTransformForBone(const FTransform& Transform, EBodyStateBasicBoneType Bone)
{
	Bones[(int32) Bone]->BoneData.SetFromTransform(Transform);
}

void UBodyStateSkeleton::SetMetaForBone(const FBodyStateBoneMeta& BoneMeta, EBodyStateBasicBoneType Bone)
{
	Bones[(int32) Bone]->Meta = BoneMeta;
}

void UBodyStateSkeleton::ChangeBasis(const FRotator& PreBase, const FRotator& PostBase, bool AdjustVectors /*= true*/)
{
	for (auto Bone : Bones)
	{
		Bone->ChangeBasis(PreBase, PostBase, AdjustVectors);
	}
}

void UBodyStateSkeleton::SetFromNamedSkeletonData(const FNamedSkeletonData& NamedSkeletonData)
{
	// Clear our skeleton data
	ResetToDefaultSkeleton();

	// Set the tracked bone data

	// Basic
	for (int i = 0; i < NamedSkeletonData.TrackedBasicBones.Num(); i++)
	{
		const FKeyedTransform& NamedData = NamedSkeletonData.TrackedBasicBones[i];
		SetTransformForBone(NamedData.Transform, NamedData.Name);
	}

	// Advanced
	for (int i = 0; i < NamedSkeletonData.TrackedAdvancedBones.Num(); i++)
	{
		const FNamedBoneData& NamedData = NamedSkeletonData.TrackedAdvancedBones[i];
		SetDataForBone(NamedData.Data, NamedData.Name);
	}

	// Add the unique meta for each bone
	for (int i = 0; i < NamedSkeletonData.UniqueMetas.Num(); i++)
	{
		const FNamedBoneMeta& NamedMeta = NamedSkeletonData.UniqueMetas[i];
		SetMetaForBone(NamedMeta.Meta, NamedMeta.Name);
	}
}

// Not fully deep copy atm, but usable
void UBodyStateSkeleton::SetFromOtherSkeleton(UBodyStateSkeleton* Other)
{
	if (!bTrackingActive)
	{
		return;
	}

	for (int i = 0; i < Bones.Num(); i++)
	{
		UBodyStateBone* OtherBone = Other->Bones[i];
		UBodyStateBone* Bone = Bones[i];

		// copy bone and meta data
		Bone->BoneData = OtherBone->BoneData;
		Bone->Meta = OtherBone->Meta;
	}
}

void UBodyStateSkeleton::MergeFromOtherSkeleton(UBodyStateSkeleton* Other)
{
	if (!bTrackingActive)
	{
		return;
	}

	
	if (Other->Name != "HMD")
	{
		for (int i = 0; i < Bones.Num(); i++)
		{
			UBodyStateBone* OtherBone = Other->Bones[i];
			UBodyStateBone* Bone = Bones[i];

			// todo: discriminate based on accuracy

			// If the bone confidence is same or higher, copy the bone
			if (OtherBone->Meta.Confidence >= Bone->Meta.Confidence)
			{
				Bone->BoneData = OtherBone->BoneData;
				Bone->Meta = OtherBone->Meta;
			}
		}
	
		int Count = 0;
		for (auto OtherFinger : Other->LeftArm()->Hand->Fingers)
		{
			auto ThisFinger = LeftArm()->Hand->Fingers[Count++];

			ThisFinger->bIsExtended = OtherFinger->bIsExtended;
		}
		Count = 0;
		for (auto OtherFinger : Other->RightArm()->Hand->Fingers)
		{
			auto ThisFinger = RightArm()->Hand->Fingers[Count++];

			ThisFinger->bIsExtended = OtherFinger->bIsExtended;
		}
	}
	// merge tags, add unique tags of other skeleton
	for (FString& Tag : Other->TrackingTags)
	{
		TrackingTags.AddUnique(Tag);
	}
}

bool UBodyStateSkeleton::HasValidTrackingTags(TArray<FString>& LimitTags)
{
	for (FString& Tag : LimitTags)
	{
		if (!TrackingTags.Contains(Tag))
		{
			return false;
		}
	}
	return true;
}

bool UBodyStateSkeleton::IsTrackingAnyBone()
{
	for (UBodyStateBone* Bone : Bones)
	{
		if (Bone->IsTracked())
		{
			return true;
		}
	}
	return false;
}

void UBodyStateSkeleton::ClearConfidence()
{
	// Clear from root bone
	Bones[0]->SetTrackingConfidenceRecursively(0.f);
}

bool UBodyStateSkeleton::ServerUpdateBodyState_Validate(FNamedSkeletonData BodyState)
{
	return true;
}

void UBodyStateSkeleton::ServerUpdateBodyState_Implementation(const FNamedSkeletonData InBodyStateSkeleton)
{
	// Multi cast to everybody
	Multi_UpdateBodyState(InBodyStateSkeleton);
}

void UBodyStateSkeleton::Multi_UpdateBodyState_Implementation(const FNamedSkeletonData InBodyStateSkeleton)
{
	SetFromNamedSkeletonData(InBodyStateSkeleton);
	Name = TEXT("Network");
}
void UBodyStateSkeleton::ReleaseRefs()
{
	if (PrivateLeftArm && PrivateLeftArm->IsValidLowLevel())
	{
		PrivateLeftArm->RemoveFromRoot();
		PrivateLeftArm = nullptr;
	}
	if (PrivateRightArm && PrivateRightArm->IsValidLowLevel())
	{
		PrivateRightArm->RemoveFromRoot();
		PrivateRightArm = nullptr;
	}
}