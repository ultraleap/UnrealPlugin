// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Skeleton/BodyStateSkeleton.h"
#include "BodyStateAnimInstance.h"
#include "AnimNode_ModifyBodyStateMappedBones.generated.h"

USTRUCT()
struct BODYSTATE_API FAnimNode_ModifyBodyStateMappedBones : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

public:

	/** All combined settings required for this node to process mapped bones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BodyState, meta = (PinHiddenByDefault))
	FMappedBoneAnimData MappedBoneAnimData;

	// FAnimNode_SkeletalControlBase interface
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;
	virtual bool NeedsOnInitializeAnimInstance() const override { return true; };
	virtual void EvaluateComponentPose_AnyThread(FComponentSpacePoseContext& Output) override;
	// End of FAnimNode_SkeletalControlBase interface

	// Constructor 
	FAnimNode_ModifyBodyStateMappedBones();

protected:
	bool WorldIsGame;
	AActor* OwningActor;
	const UBodyStateAnimInstance* BSAnimInstance;

};