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

#include "BodyStateEnums.h"
#include "Skeleton/BodyStateArm.h"
#include "Skeleton/BodyStateBone.h"
#include "UObject/CoreNet.h"

#include "BodyStateSkeleton.generated.h"

// Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FNamedBoneData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FBodyStateBoneData Data;

	UPROPERTY()
	EBodyStateBasicBoneType Name;
};

// Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FKeyedTransform
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	EBodyStateBasicBoneType Name;
};

// Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FNamedBoneMeta
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FBodyStateBoneMeta Meta;

	UPROPERTY()
	EBodyStateBasicBoneType Name;
};

// Used for replication
USTRUCT(BlueprintType)
struct BODYSTATE_API FNamedSkeletonData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FNamedBoneData> TrackedAdvancedBones;

	UPROPERTY()
	TArray<FKeyedTransform> TrackedBasicBones;

	UPROPERTY()
	TArray<FNamedBoneMeta> UniqueMetas;
};

/** Body Skeleton data, all bones are expected in component space*/
UCLASS(BlueprintType)
class BODYSTATE_API UBodyStateSkeleton : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Human readable name, taken from device config, useful for debug */
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Skeleton")
	FString Name;

	/** If tracking is active or frozen (useful for debugging)*/
	UPROPERTY(BlueprintReadWrite, Category = "BodyState Skeleton")
	bool bTrackingActive;

	/** Id issued to this skeleton, useful for getting device information */
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Skeleton")
	int32 SkeletonId;

	// Note: Storage of actual bone data should be here
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Skeleton")
	TArray<UBodyStateBone*> Bones;	  // All bones stored here

	// internal lookup for the bones
	TMap<EBodyStateBasicBoneType, UBodyStateBone*> BoneMap;

	/** Tracking Tags that this skeleton has currently inherited. */
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Skeleton")
	TArray<FString> TrackingTags;

	// Used for reference point calibration e.g. hydra base origin
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Skeleton")
	FTransform RootOffset;

	// Convenience bone getters

	// Root
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateBone* RootBone();

	// Arms & Hands

	/** Get a structured convenience wrapper around left arm bones*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateArm* LeftArm();

	/** Get a structured convenience wrapper around right arm bones*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateArm* RightArm();

	// Spine & Head
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	UBodyStateBone* Head();

	/*Get Bone data by enum*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	class UBodyStateBone* BoneForEnum(EBodyStateBasicBoneType Bone);

	/*Get Bone data by name matching*/
	UFUNCTION(BlueprintPure, Category = "BodyState Skeleton")
	class UBodyStateBone* BoneNamed(const FString& InName);

	// Replication and Setting Data

	// Setting Bone Data
	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void ResetToDefaultSkeleton();

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetDataForBone(const FBodyStateBoneData& BoneData, EBodyStateBasicBoneType Bone);

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetTransformForBone(const FTransform& Transform, EBodyStateBasicBoneType Bone);

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetMetaForBone(const FBodyStateBoneMeta& BoneMeta, EBodyStateBasicBoneType Bone);

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void ChangeBasis(const FRotator& PreBase, const FRotator& PostBase, bool AdjustVectors = true);

	// Conversion
	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	FNamedSkeletonData GetMinimalNamedSkeletonData();	 // key replication getter

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetFromNamedSkeletonData(const FNamedSkeletonData& NamedSkeletonData);	   // key replication setter

	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void SetFromOtherSkeleton(UBodyStateSkeleton* Other);

	/** Copies only bones that are tracked from the other skeleton */
	UFUNCTION(BlueprintCallable, Category = "BodyState Skeleton Setting")
	void MergeFromOtherSkeleton(UBodyStateSkeleton* Other);

	/** Check if the skeleton meets requires tracking tags e.g. hands, fingers, head etc*/
	bool HasValidTrackingTags(TArray<FString>& LimitTags);

	/** Check if any bone is being tracked */
	bool IsTrackingAnyBone();

	void ClearConfidence();

	// Replication
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerUpdateBodyState(const FNamedSkeletonData InBodyStateSkeleton);

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_UpdateBodyState(const FNamedSkeletonData InBodyStateSkeleton);

	FCriticalSection BoneDataLock;

	void ReleaseRefs();

protected:
	TArray<FNamedBoneData> TrackedBoneData();
	TArray<FKeyedTransform> TrackedBasicBones();
	TArray<FNamedBoneData> TrackedAdvancedBones();
	TArray<FNamedBoneMeta> UniqueBoneMetas();

private:
	UPROPERTY()
	UBodyStateArm* PrivateLeftArm;

	UPROPERTY()
	UBodyStateArm* PrivateRightArm;
};
