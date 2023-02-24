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

#pragma once

#include "BodyStateAnimInstance.h"
#include "CoreMinimal.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Skeleton/BodyStateSkeleton.h"

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
	virtual void EvaluateSkeletalControl_AnyThread(
		FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	virtual void OnInitializeAnimInstance(const FAnimInstanceProxy* InProxy, const UAnimInstance* InAnimInstance) override;
	virtual bool NeedsOnInitializeAnimInstance() const override
	{
		return true;
	};
	virtual void EvaluateComponentPose_AnyThread(FComponentSpacePoseContext& Output) override;
	// End of FAnimNode_SkeletalControlBase interface

	// Constructor
	FAnimNode_ModifyBodyStateMappedBones();

protected:
	bool WorldIsGame;
	AActor* OwningActor;
	const UBodyStateAnimInstance* BSAnimInstance;

private:
	void ApplyTranslation(const FCachedBoneLink& CachedBone, FTransform& NewBoneTM, const FCachedBoneLink* WristCachedBone,
		const FCachedBoneLink* ArmCachedBone,const FMappedBoneAnimData& MappedBoneAnimData);
	void ApplyRotation(const FCachedBoneLink& CachedBone, FTransform& NewBoneTM, const FCachedBoneLink* CachedWristBone,
		const FMappedBoneAnimData& MappedBoneAnimData);
	void ApplyScale(const FCachedBoneLink& CachedBone, const FCachedBoneLink* CachedPrevBone, FTransform& NewBoneTM,
		FTransform& PrevBoneTM, const FMappedBoneAnimData& MappedBoneAnimData);
	
	
	void ApplyAutoCorrectRotation(const FTransform& WristBeforeMapping, const FTransform& LeapWrist, FTransform& NewBoneTM,
		const FMappedBoneAnimData& MappedBoneAnimData);


	bool CheckInitEvaulate();
	void CacheArmOrWrist(
		const FCachedBoneLink& CachedBone, const FCachedBoneLink** ArmCachedBone, const FCachedBoneLink** WristCachedBone);

	void SetHandGlobalScale(FTransform& NewBoneTM, const FMappedBoneAnimData& MappedBoneAnimData);
	

	FTransform GetComponentTransformScaleOnly();
	float CalculateLeapHandLength(const FMappedBoneAnimData& MappedBoneAnimData);
	
};