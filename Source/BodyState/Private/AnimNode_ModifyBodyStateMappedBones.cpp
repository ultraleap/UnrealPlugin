// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

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
void FAnimNode_ModifyBodyStateMappedBones::ApplyTranslation(
	const FCachedBoneLink& CachedBone, FTransform& NewBoneTM, const FCachedBoneLink& WristCachedBone)
{
	FVector BoneTranslation = CachedBone.BSBone->BoneData.Transform.GetTranslation();
	FTransform ComponentTransform = BSAnimInstance->GetSkelMeshComponent()->GetRelativeTransform();

	if (&MappedBoneAnimData.CachedBoneList[0] == &CachedBone)
	// if (CachedBone.BSBone->Name.Contains("wrist") || CachedBone.BSBone->Name.Contains("arm"))
	{
		if (CachedBone.BSBone->Name.Contains("arm") && WristCachedBone.MeshBone.BoneIndex > -1)
		{
			auto WristPosition = WristCachedBone.BSBone->BoneData.Transform.GetLocation();
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
void FAnimNode_ModifyBodyStateMappedBones::ApplyRotation(const FCachedBoneLink& CachedBone, FTransform& NewBoneTM)
{
	FQuat BoneQuat = CachedBone.BSBone->BoneData.Transform.GetRotation();

	// Apply pre and post adjustment (Post * (Input * Pre) )
	BoneQuat = MappedBoneAnimData.AutoCorrectRotation *
			   (MappedBoneAnimData.OffsetTransform.GetRotation() * (BoneQuat * MappedBoneAnimData.PreBaseRotation.Quaternion()));
	NewBoneTM.SetRotation(BoneQuat);
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
	const FCachedBoneLink& CachedBone, FCachedBoneLink& ArmCachedBone, FCachedBoneLink& WristCachedBone)
{
	// these names are constant as they're the bodystate names
	if (CachedBone.BSBone->Name.Contains("lowerarm"))
	{
		ArmCachedBone = CachedBone;
	}
	if (CachedBone.BSBone->Name.Contains("wrist"))
	{
		WristCachedBone = CachedBone;
	}
}

void FAnimNode_ModifyBodyStateMappedBones::EvaluateComponentPose_AnyThread(FComponentSpacePoseContext& Output)
{
	Super::EvaluateComponentPose_AnyThread(Output);

	if (!CheckInitEvaulate())
	{
		return;
	}
	// If we don't have data driving us, ignore this evaluation
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);

	FScopeLock ScopeLock(&MappedBoneAnimData.BodyStateSkeleton->BoneDataLock);

	// cached for elbow position
	FCachedBoneLink ArmCachedBone;
	FCachedBoneLink WristCachedBone;

	for (auto& CachedBone : MappedBoneAnimData.CachedBoneList)
	{
		CacheArmOrWrist(CachedBone, ArmCachedBone, WristCachedBone);
	}
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

		ApplyRotation(CachedBone, NewBoneTM);
		ApplyTranslation(CachedBone, NewBoneTM, WristCachedBone);

		// Set the transform back into the anim system
		TArray<FBoneTransform> TempTransform;
		TempTransform.Add(FBoneTransform(CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer), NewBoneTM));
		Output.Pose.LocalBlendCSBoneTransforms(TempTransform, BlendWeight);
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