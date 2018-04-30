#pragma once
#include "CoreMinimal.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
#include "BodyStateSkeleton.h"
#include "BodyStateAnimInstance.h"
#include "AnimNode_ModifyBodyStateMappedBones.generated.h"

USTRUCT()
struct BODYSTATE_API FAnimNode_ModifyBodyStateMappedBones : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()

	/** All combined settings required for this node to process mapped bones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BodyState, meta = (PinShownByDefault))
	FMappedBoneAnimData MappedBoneAnimData;

public:

	// FAnimNode_SkeletalControlBase interface
	//virtual void UpdateInternal(const FAnimationUpdateContext& Context) override;
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	// End of FAnimNode_SkeletalControlBase interface

	// Constructor 
public:

	FAnimNode_ModifyBodyStateMappedBones();

protected:
	bool WorldIsGame;
	AActor* OwningActor;
};