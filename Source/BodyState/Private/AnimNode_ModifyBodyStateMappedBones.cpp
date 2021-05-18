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

void FAnimNode_ModifyBodyStateMappedBones::EvaluateComponentPose_AnyThread(FComponentSpacePoseContext& Output)
{
	Super::EvaluateComponentPose_AnyThread(Output);

	if (BSAnimInstance->MappedBoneList.Num() == 0)
	{
		return;
	}
	if (MappedBoneAnimData.BoneMap.Num() == 0 || MappedBoneAnimData.BodyStateSkeleton == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Updating cached MappedBoneAnimData"));
		MappedBoneAnimData = BSAnimInstance->MappedBoneList[0];
	}
	// If we don't have data driving us, ignore this evaluation
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);

	TArray<FBoneTransform> TempTransform;
	if (!MappedBoneAnimData.BodyStateSkeleton)
	{
		return;
	}

	FScopeLock ScopeLock(&MappedBoneAnimData.BodyStateSkeleton->BoneDataLock);

	// use original quat calc one liner
	static const bool UseOldQuat = true;
	// used to be in the event graph for the anim blueprints
	// do nothing if not tracking
	bool IsTracking = false;
	switch (BSAnimInstance->AutoMapTarget)
	{
		case EBodyStateAutoRigType::HAND_LEFT:
		{
			IsTracking = MappedBoneAnimData.BodyStateSkeleton->LeftArm()->Hand->Wrist->IsTracked();
		}
		break;
		case EBodyStateAutoRigType::HAND_RIGHT:
		{
			IsTracking = MappedBoneAnimData.BodyStateSkeleton->RightArm()->Hand->Wrist->IsTracked();
		}
		break;
		case EBodyStateAutoRigType::BOTH_HANDS:
		{
			IsTracking = MappedBoneAnimData.BodyStateSkeleton->LeftArm()->Hand->Wrist->IsTracked() ||
						 MappedBoneAnimData.BodyStateSkeleton->RightArm()->Hand->Wrist->IsTracked();
		}
		break;
	}

	if (!IsTracking)
	{
		BlendWeight = 0;
	}

	CachedBoneLink ArmCachedBone;
	CachedBoneLink WristCachedBone;

	// SN: there should be an array re-ordered by hierarchy (parents -> children order)
	for (auto CachedBone : MappedBoneAnimData.CachedBoneList)
	{
		if (CachedBone.MeshBone.BoneIndex == -1)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has an invalid bone index: %d"), *CachedBone.MeshBone.BoneName.ToString(),
				CachedBone.MeshBone.BoneIndex);
			continue;
		}
		// based on the unity version, we use an offset from the wrist for the elbow
		// so skip, wait for wrist to be cached, then calc outside this loop
		if (CachedBone.BSBone->Name.Contains("lowerarm"))
		{
			ArmCachedBone = CachedBone;
			continue;
		}
		if (CachedBone.BSBone->Name.Contains("wrist"))
		{
			WristCachedBone = CachedBone;
		}

		FCompactPoseBoneIndex CompactPoseBoneToModify = CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer);
		FTransform NewBoneTM;
		FTransform ComponentTransform;

		// JIM: scoped lock used to be here
		{
			NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
			ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();
		}

		// Scale
		// Ignored

		// Rotate

		FQuat BoneQuat = CachedBone.BSBone->BoneData.Transform.GetRotation();

		// Apply pre and post adjustment (Post * (Input * Pre) )
		if (!MappedBoneAnimData.PreBaseRotation.ContainsNaN())
		{
			if (UseOldQuat)
			{
				BoneQuat =
					MappedBoneAnimData.OffsetTransform.GetRotation() * (BoneQuat * MappedBoneAnimData.PreBaseRotation.Quaternion());
			}
			else
			{
				BoneQuat *= MappedBoneAnimData.PreBaseRotation.Quaternion();
				BoneQuat *= MappedBoneAnimData.OffsetTransform.GetRotation();
			}
		}

		NewBoneTM.SetRotation(BoneQuat);

		if (MappedBoneAnimData.bShouldDeformMesh)
		{
			const FVector& BoneTranslation = CachedBone.BSBone->BoneData.Transform.GetTranslation();
			// const FVector RotatedTranslation = MappedBoneAnimData.OffsetTransform.GetRotation().RotateVector(BoneTranslation);
			NewBoneTM.SetTranslation(BoneTranslation + MappedBoneAnimData.OffsetTransform.GetLocation());
		}
		// wrist only, removes the need for a wrist modify node in the anim blueprint
		else
		{
			if (CachedBone.BSBone->Name.Contains("wrist"))
			{
				FVector BoneTranslation = CachedBone.BSBone->BoneData.Transform.GetTranslation();
				if (MappedBoneAnimData.IsFlippedByScale)
				{
					BoneTranslation.X = -BoneTranslation.X;
				}
				// const FVector RotatedTranslation =
				// MappedBoneAnimData.OffsetTransform.GetRotation().RotateVector(BoneTranslation);
				NewBoneTM.SetTranslation(BoneTranslation + MappedBoneAnimData.OffsetTransform.GetLocation());
			}
		}

		// Translate

		// Set the transform back into the anim system
		TempTransform.Add(FBoneTransform(CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer), NewBoneTM));
		Output.Pose.LocalBlendCSBoneTransforms(TempTransform, BlendWeight);
		TempTransform.Reset();

		// Apply transforms to list
		// OutBoneTransforms.Add(FBoneTransform(CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer), NewBoneTM));
	}
	//#if 0
	// Elbow position, only if lower arm bone found.
	if (WristCachedBone.MeshBone.BoneIndex > -1 && ArmCachedBone.MeshBone.BoneIndex > -1)
	{
		auto WristPosition = MappedBoneAnimData.BodyStateSkeleton->LeftArm()->Hand->Wrist->Position();
		auto ElbowForward =
			FRotationMatrix(MappedBoneAnimData.BodyStateSkeleton->LeftArm()->LowerArm->Orientation()).GetScaledAxis(EAxis::X);
		auto ElbowPosition =
			WristPosition - ((-MappedBoneAnimData.ElbowLength * ElbowForward) + MappedBoneAnimData.OffsetTransform.GetLocation());

		auto CachedBone = ArmCachedBone;
		FCompactPoseBoneIndex CompactPoseBoneToModify = CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer);
		FTransform NewBoneTM;
		FTransform ComponentTransform;

		NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
		ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();

		// Scale
		// Ignored

		// Rotate

		// Bone Space
		FAnimationRuntime::ConvertCSTransformToBoneSpace(
			ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_ComponentSpace);
		FQuat BoneQuat = CachedBone.BSBone->BoneData.Transform.GetRotation();

		// Apply pre and post adjustment (Post * (Input * Pre) )
		if (!MappedBoneAnimData.PreBaseRotation.ContainsNaN())
		{
			if (UseOldQuat)
			{
				BoneQuat =
					MappedBoneAnimData.OffsetTransform.GetRotation() * (BoneQuat * MappedBoneAnimData.PreBaseRotation.Quaternion());
			}
			else
			{
				BoneQuat *= MappedBoneAnimData.PreBaseRotation.Quaternion();
				BoneQuat *= MappedBoneAnimData.OffsetTransform.GetRotation();
			}
		}

		NewBoneTM.SetRotation(BoneQuat);

		if (MappedBoneAnimData.bShouldDeformMesh)
		{
			const FVector& BoneTranslation = ElbowPosition;
			// const FVector RotatedTranslation = MappedBoneAnimData.OffsetTransform.GetRotation().RotateVector(BoneTranslation);
			NewBoneTM.SetTranslation(BoneTranslation + MappedBoneAnimData.OffsetTransform.GetLocation());
		}

		// Back to component space
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(
			ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_ComponentSpace);

		// Translate

		// Set the transform back into the anim system
		TempTransform.Add(FBoneTransform(CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer), NewBoneTM));
		Output.Pose.LocalBlendCSBoneTransforms(TempTransform, BlendWeight);
		TempTransform.Reset();
	}
	//#endif
}

bool FAnimNode_ModifyBodyStateMappedBones::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return (MappedBoneAnimData.BodyStateSkeleton != nullptr);
}