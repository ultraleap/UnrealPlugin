// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AnimNode_ModifyBodyStateMappedBones.h"
#include "AnimationRuntime.h"
#include "Runtime/Engine/Public/Animation/AnimInstanceProxy.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"

FAnimNode_ModifyBodyStateMappedBones::FAnimNode_ModifyBodyStateMappedBones()
	: FAnimNode_SkeletalControlBase()
{
	WorldIsGame = false;
	Alpha = 1.f;
}

void FAnimNode_ModifyBodyStateMappedBones::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	//Tag limit by skeleton
	if (!MappedBoneAnimData.SkeletonHasValidTags())
	{
		return;
	}

	//If we don't have data driving us, ignore this evaluation
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);

	TArray<FBoneTransform> TempTransform;

	

	//SN: there should be an array re-ordered by hierarchy (parents -> children order)
	for (auto CachedBone : MappedBoneAnimData.CachedBoneList)
	{
		if (CachedBone.MeshBone.BoneIndex == -1)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has an invalid bone index: %d"), *CachedBone.MeshBone.BoneName.ToString(), CachedBone.MeshBone.BoneIndex);
			continue;
		}
	
		FCompactPoseBoneIndex CompactPoseBoneToModify = CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer);
		FTransform NewBoneTM;
		FTransform ComponentTransform;

		{
			FScopeLock ScopeLock(&MappedBoneAnimData.BodyStateSkeleton->BoneDataLock);
			NewBoneTM = Output.Pose.GetComponentSpaceTransform(CompactPoseBoneToModify);
			ComponentTransform = Output.AnimInstanceProxy->GetComponentTransform();
		}

		//Scale
		//Ignored

		//Rotate

		//Bone Space
		FAnimationRuntime::ConvertCSTransformToBoneSpace(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_ComponentSpace);
		FQuat BoneQuat = CachedBone.BSBone->BoneData.Transform.GetRotation();
		

		//Apply pre and post adjustment (Post * (Input * Pre) )
		if (!MappedBoneAnimData.PreBaseRotation.ContainsNaN())
		{
			BoneQuat = MappedBoneAnimData.OffsetTransform.GetRotation() * (BoneQuat * MappedBoneAnimData.PreBaseRotation.Quaternion());
		}

		NewBoneTM.SetRotation(BoneQuat);

		if (MappedBoneAnimData.bShouldDeformMesh)
		{
			const FVector& BoneTranslation = CachedBone.BSBone->BoneData.Transform.GetTranslation();
			const FVector RotatedTranslation = MappedBoneAnimData.OffsetTransform.GetRotation().RotateVector(BoneTranslation);
			NewBoneTM.SetTranslation(RotatedTranslation + MappedBoneAnimData.OffsetTransform.GetLocation());
		}

		//Back to component space
		FAnimationRuntime::ConvertBoneSpaceTransformToCS(ComponentTransform, Output.Pose, NewBoneTM, CompactPoseBoneToModify, EBoneControlSpace::BCS_ComponentSpace);

		//Translate
		
		//Workaround ensure our rotations are offset for each bone
		TempTransform.Add(FBoneTransform(CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer), NewBoneTM));
		Output.Pose.LocalBlendCSBoneTransforms(TempTransform, BlendWeight);
		TempTransform.Reset();

		//Apply transforms to list
		//OutBoneTransforms.Add(FBoneTransform(CachedBone.MeshBone.GetCompactPoseIndex(BoneContainer), NewBoneTM));
	}
}

bool FAnimNode_ModifyBodyStateMappedBones::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	return (MappedBoneAnimData.BodyStateSkeleton != nullptr);
}
