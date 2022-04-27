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

#include "AnimNode_ModifyBodyStateMappedBones.h"

#include "AnimationRuntime.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Runtime/Engine/Public/Animation/AnimInstanceProxy.h"
#include "Skeleton/BodyStateArm.h"

FAnimNode_ModifyBodyStateMappedBones::FAnimNode_ModifyBodyStateMappedBones() : FAnimNode_SkeletalControlBase()
{
	WorldIsGame = false;
	Alpha = 1.f;
}

void FAnimNode_ModifyBodyStateMappedBones::OnInitializeAnimInstance(
	const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance)
{
	Super::OnInitializeAnimInstance(InProxy, InAnimInstance);
	BSAnimInstance = Cast<UBodyStateAnimInstance>(InAnimInstance);
}

void FAnimNode_ModifyBodyStateMappedBones::EvaluateSkeletalControl_AnyThread(
	FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	EvaluateComponentPose_AnyThread(Output);
}
void FAnimNode_ModifyBodyStateMappedBones::ApplyTranslation(const FCachedBoneLink& CachedBone, FTransform& NewBoneTM,
	const FCachedBoneLink* WristCachedBone, const FCachedBoneLink* ArmCachedBone)
{
	FVector BoneTranslation = CachedBone.BSBone->BoneData.Transform.GetTranslation();
	FTransform ComponentTransform = BSAnimInstance->GetSkelMeshComponent()->GetRelativeTransform();
	int32 WristBoneIndex = -1;

	if (WristCachedBone)
	{
		WristBoneIndex = WristCachedBone->MeshBone.BoneIndex;
	}
	if (&MappedBoneAnimData.CachedBoneList[0] == &CachedBone)
	{
		if (&CachedBone == ArmCachedBone && WristBoneIndex > -1 && WristCachedBone)
		{
			auto WristPosition = WristCachedBone->BSBone->BoneData.Transform.GetLocation();
			auto ElbowForward = FRotationMatrix(CachedBone.BSBone->BoneData.Transform.Rotator()).GetScaledAxis(EAxis::X);
			auto ElbowPosition = WristPosition - ((MappedBoneAnimData.ElbowLength * ElbowForward) +
													 MappedBoneAnimData.OffsetTransform.GetLocation());

			const FVector CorrectTranslation =
				ComponentTransform.InverseTransformVector(ElbowPosition + MappedBoneAnimData.OffsetTransform.GetLocation());

			NewBoneTM.SetTranslation(CorrectTranslation);
		}
		else
		{
			FQuat AdditionalRotation = MappedBoneAnimData.OffsetTransform.GetRotation();

			const FVector RotatedTranslation = AdditionalRotation.RotateVector(BoneTranslation);

			// this deals with the case where the component's scale has been messed with to flip hands left to right or right to
			// left
			const FVector CorrectTranslation =
				ComponentTransform.InverseTransformVector(RotatedTranslation + MappedBoneAnimData.OffsetTransform.GetLocation());
			NewBoneTM.SetTranslation(CorrectTranslation);
		}
	}
	else if (MappedBoneAnimData.bShouldDeformMesh)
	{
		FQuat AdditionalRotation = MappedBoneAnimData.AutoCorrectRotation * MappedBoneAnimData.OffsetTransform.GetRotation();

		const FVector RotatedTranslation = AdditionalRotation.RotateVector(BoneTranslation);
		const FVector CorrectTranslation =
			ComponentTransform.InverseTransformVector(RotatedTranslation + MappedBoneAnimData.OffsetTransform.GetLocation());

		NewBoneTM.SetTranslation(CorrectTranslation);
	}
}
void FAnimNode_ModifyBodyStateMappedBones::ApplyRotation(
	const FCachedBoneLink& CachedBone, FTransform& NewBoneTM, const FCachedBoneLink* CachedWristBone)
{
	FQuat BoneQuat = CachedBone.BSBone->BoneData.Transform.GetRotation();

	// Apply pre and post adjustment (Post * (Input * Pre) )
	BoneQuat = MappedBoneAnimData.AutoCorrectRotation *
			   (MappedBoneAnimData.OffsetTransform.GetRotation() * (BoneQuat * MappedBoneAnimData.PreBaseRotation.Quaternion()));
	NewBoneTM.SetRotation(BoneQuat);
}
FVector CalculateAxis(const FTransform& Transform, const FVector& Direction)
{
	FVector BoneForward = Transform.InverseTransformVector(Direction);
	BoneForward.Normalize();
	return BoneForward;
}
void FAnimNode_ModifyBodyStateMappedBones::ApplyScale(
	const FCachedBoneLink& CachedBone, const FCachedBoneLink* CachedPrevBone, FTransform& NewBoneTM, FTransform& PrevBoneTM)
{
	if (!BSAnimInstance->ScaleModelToTrackingData || !MappedBoneAnimData.FingerTipLengths.Num())
	{
		return;
	}
	bool IsTip = false;
	bool IsLeft = false;

	int FingerIndex = 0;
	float FingerScaleOffset = 0;
	// is it a tip?
	switch (CachedBone.BSBone->BoneType)
	{
		case EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_R:
			FingerIndex = 1;
			FingerScaleOffset = BSAnimInstance->IndexTipScaleOffset;
			IsTip = true;
			break;
		case EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_R:
			FingerIndex = 2;
			FingerScaleOffset = BSAnimInstance->MiddleTipScaleOffset;
			IsTip = true;
			break;
		case EBodyStateBasicBoneType::BONE_RING_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_RING_3_DISTAL_R:
			FingerIndex = 3;
			FingerScaleOffset = BSAnimInstance->RingTipScaleOffset;
			IsTip = true;
			break;
		case EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_R:
			FingerScaleOffset = BSAnimInstance->PinkyTipScaleOffset;
			FingerIndex = 4;
			IsTip = true;
			break;
		case EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R:
			FingerScaleOffset = BSAnimInstance->ThumbTipScaleOffset;
			FingerIndex = 0;
			IsTip = true;
			break;
	}
	switch (CachedBone.BSBone->BoneType)
	{
		case EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_RING_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_L:
		case EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_L:
			IsLeft = true;
			break;
		case EBodyStateBasicBoneType::BONE_INDEX_3_DISTAL_R:
		case EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_R:
		case EBodyStateBasicBoneType::BONE_RING_3_DISTAL_R:
		case EBodyStateBasicBoneType::BONE_PINKY_3_DISTAL_R:
		case EBodyStateBasicBoneType::BONE_THUMB_2_DISTAL_R:
			IsLeft = false;
			break;
	}
	
	if (IsTip)
	{
		FVector TipPosition = CachedBone.BSBone->BoneData.Transform.GetLocation();
		FTransform DirectionTransform = CachedPrevBone->BSBone->BoneData.Transform;
		float DirectionMult = -1;
		FVector BehindTipPosition = CachedPrevBone->BSBone->BoneData.Transform.GetLocation();
		
		float LeapFingerTipLength = FVector::Distance(TipPosition, BehindTipPosition);

		

		float ModelFingerTipLength = MappedBoneAnimData.FingerTipLengths[FingerIndex];
		// never tracked/uninitialised state
		if (TipPosition.IsZero())
		{
			LeapFingerTipLength = ModelFingerTipLength;

			TipPosition = NewBoneTM.GetLocation();
			BehindTipPosition = PrevBoneTM.GetLocation();

			DirectionTransform = NewBoneTM;

			if (!IsLeft)
			{
				DirectionMult = 1;
			}

		}
		float Ratio = LeapFingerTipLength / ModelFingerTipLength;
		// Fingerscale offset of one is a zero scale change
		float AdjustedRatio = Ratio * (FingerScaleOffset - 1.0);


		// Calculate the direction that goes up the bone towards the next bone
		FVector Direction = (BehindTipPosition - TipPosition);
		Direction.Normalize();
		Direction *= DirectionMult;
		// Calculate which axis to scale along
		FVector Axis = CalculateAxis(DirectionTransform, Direction);
		// Calculate the scale by ensuring all axis are 1 apart from the axis to scale along
		FVector Scale = FVector::OneVector + (Axis * AdjustedRatio);
		NewBoneTM.SetScale3D(Scale * BSAnimInstance->ModelScaleOffset);
	}
}

bool FAnimNode_ModifyBodyStateMappedBones::CheckInitEvaulate()
{
	if (!BSAnimInstance)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
				TEXT("Ultraleap Modify Mapped Bones - No Bodystate Anim Instance - Reparent your anim instance to "
					 "BodyStateAnimInstance"));
		}
		return false;
	}
	if (BSAnimInstance->MappedBoneList.Num() == 0)
	{
		return false;
	}
	if (MappedBoneAnimData.BoneMap.Num() == 0 || MappedBoneAnimData.BodyStateSkeleton == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Updating cached MappedBoneAnimData"));
		MappedBoneAnimData = BSAnimInstance->MappedBoneList[0];
	}

	if (!MappedBoneAnimData.BodyStateSkeleton)
	{
		return false;
	}
	return true;
}
void FAnimNode_ModifyBodyStateMappedBones::CacheArmOrWrist(
	const FCachedBoneLink& CachedBone, const FCachedBoneLink** ArmCachedBone, const FCachedBoneLink** WristCachedBone)
{
	switch (CachedBone.BSBone->BoneType)
	{
		case EBodyStateBasicBoneType::BONE_LOWERARM_L:
		case EBodyStateBasicBoneType::BONE_LOWERARM_R:
		{
			*ArmCachedBone = &CachedBone;
			break;
		}
		case EBodyStateBasicBoneType::BONE_HAND_WRIST_L:
		case EBodyStateBasicBoneType::BONE_HAND_WRIST_R:
		{
			*WristCachedBone = &CachedBone;
			break;
		}
	}
}
void FAnimNode_ModifyBodyStateMappedBones::SetHandGlobalScale(FTransform& NewBoneTM)
{
	if (!BSAnimInstance->ScaleModelToTrackingData || !MappedBoneAnimData.HandModelLength)
	{
		return;
	}
	float LeapLength = CalculateLeapHandLength();
	// never tracked
	if (LeapLength == 0)
	{
		LeapLength = MappedBoneAnimData.HandModelLength;
	}

	// ratio between model middle finger length and hand middle finger length
	const float MiddleFingerRatio = LeapLength / MappedBoneAnimData.HandModelLength;
	// constant user entered correction for model
	const float ScaleRatio = MiddleFingerRatio * BSAnimInstance->ModelScaleOffset;
	NewBoneTM.SetScale3D(MappedBoneAnimData.OriginalScale * ScaleRatio);
}

// middle finger length as tracked
float FAnimNode_ModifyBodyStateMappedBones::CalculateLeapHandLength()
{
	float Length = 0;
	TArray<FCachedBoneLink> FingerBones;
	for (auto& CachedBone : MappedBoneAnimData.CachedBoneList)
	{
		switch (CachedBone.BSBone->BoneType)
		{
			case EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_L:
			case EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_L:
			case EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_L:
			case EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_L:
			case EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_R:
			case EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_R:
			case EBodyStateBasicBoneType::BONE_MIDDLE_2_INTERMEDIATE_R:
			case EBodyStateBasicBoneType::BONE_MIDDLE_3_DISTAL_R:
				FingerBones.Add(CachedBone);
				break;
		}
	}

	for (int i = 0; i < (FingerBones.Num() - 1); ++i)
	{
		float Magnitude = FVector::Distance(FingerBones[i].BSBone->BoneData.Transform.GetLocation(),FingerBones[i+1].BSBone->BoneData.Transform.GetLocation());
		Length += Magnitude;
	}
	return Length;
}

void FAnimNode_ModifyBodyStateMappedBones::EvaluateComponentPose_AnyThread(FComponentSpacePoseContext& Output)
{
	Super::EvaluateComponentPose_AnyThread(Output);

	if (!CheckInitEvaulate())
	{
		return;
	}
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);

	FScopeLock ScopeLock(&MappedBoneAnimData.BodyStateSkeleton->BoneDataLock);
	
	// cached for elbow position
	const FCachedBoneLink* ArmCachedBone = nullptr;
	const FCachedBoneLink* WristCachedBone = nullptr;

	for (auto& CachedBone : MappedBoneAnimData.CachedBoneList)
	{
		CacheArmOrWrist(CachedBone, &ArmCachedBone, &WristCachedBone);
	}
	int LoopCount = 0;
	FTransform PrevBoneTM;
	FCachedBoneLink* CachedPrevBone = &MappedBoneAnimData.CachedBoneList[0];

	for (auto& CachedBone : MappedBoneAnimData.CachedBoneList)
	{
		if (CachedBone.MeshBone.BoneIndex == -1)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has an invalid bone index: %d"), *CachedBone.MeshBone.BoneName.ToString(),
				CachedBone.MeshBone.BoneIndex);
			continue;
		}
		
		FCompactPoseBoneIndex CompactPoseBoneToModify = CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer);
		FTransform NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);

		// setup global scale on the root bone
		if (!LoopCount)
		{
			SetHandGlobalScale(NewBoneTM);
		}
		// Apply scale even when not tracking, so we can see it in editor
		ApplyScale(CachedBone, CachedPrevBone, NewBoneTM, PrevBoneTM);
		if (BSAnimInstance->IsTracking)
		{
			ApplyRotation(CachedBone, NewBoneTM, WristCachedBone);
			ApplyTranslation(CachedBone, NewBoneTM, WristCachedBone, ArmCachedBone);
		}
		// Set the transform back into the anim system
		TArray<FBoneTransform> TempTransform;
		TempTransform.Add(FBoneTransform(CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer), NewBoneTM));
		Output.Pose.LocalBlendCSBoneTransforms(TempTransform, BlendWeight);

		CachedPrevBone = &CachedBone;
		PrevBoneTM = NewBoneTM;
		LoopCount++;
	}
}

bool FAnimNode_ModifyBodyStateMappedBones::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	if (!BSAnimInstance)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
				TEXT("Ultraleap Modify Mapped Bones - No Bodystate Anim Instance - Reparent your anim instance to "
					 "BodyStateAnimInstance"));
		}
		return false;
	}

	return (MappedBoneAnimData.BodyStateSkeleton != nullptr);
}