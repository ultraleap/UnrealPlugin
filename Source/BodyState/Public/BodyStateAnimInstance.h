// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BodyStateEnums.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "Skeleton/BodyStateSkeleton.h"

#include "BodyStateAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct BODYSTATE_API FBodyStateIndexedBone
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Indexed Bone")
	FName BoneName;

	UPROPERTY(BlueprintReadWrite, Category = "Indexed Bone")
	int32 ParentIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Indexed Bone")
	int32 Index;

	UPROPERTY(BlueprintReadWrite, Category = "Indexed Bone")
	TArray<int32> Children;

	FBodyStateIndexedBone()
	{
		ParentIndex = -1;
		Index = -1;
		Children.Empty();
	}
};

struct FBodyStateIndexedBoneList
{
	TArray<FBodyStateIndexedBone> Bones;
	int32 RootBoneIndex;

	// run after filling our index
	TArray<int32> FindBoneWithChildCount(int32 Count);
	void SetFromRefSkeleton(const FReferenceSkeleton& RefSkeleton);
	int32 LongestChildTraverseForBone(int32 Bone);

	FBodyStateIndexedBoneList()
	{
		RootBoneIndex = 0;
	}
};

// C++ only struct used for cached bone lookup
struct CachedBoneLink
{
	FBoneReference MeshBone;
	UBodyStateBone* BSBone;
};

/** Required struct since 4.17 to expose hotlinked mesh bone references*/
USTRUCT(BlueprintType)
struct FBPBoneReference
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = BoneName)
	FBoneReference MeshBone;
};

USTRUCT(BlueprintType)
struct FMappedBoneAnimData
{
	GENERATED_BODY()

	/** Whether the mesh should deform to match the tracked data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	bool bShouldDeformMesh;

	/** List of tags required by the tracking solution for this animation to use that data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	TArray<FString> TrackingTagLimit;

	/** Offset rotation base applied before given rotation (will rotate input) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	FRotator PreBaseRotation;

	/** Transform applied after rotation changes to all bones in map. Consider this an offset */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	FTransform OffsetTransform;

	/** Matching list of body state bone keys mapped to local mesh bone names */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	TMap<EBodyStateBasicBoneType, FBPBoneReference> BoneMap;

	/** Skeleton driving mapped data */
	UPROPERTY(BlueprintReadWrite, Category = "Bone Anim Struct")
	class UBodyStateSkeleton* BodyStateSkeleton;

	// Data structure containing a parent -> child ordered bone list
	TArray<CachedBoneLink> CachedBoneList;

	FMappedBoneAnimData()
	{
		bShouldDeformMesh = true;
		OffsetTransform.SetScale3D(FVector(1.f));
		PreBaseRotation = FRotator(ForceInitToZero);
		TrackingTagLimit.Empty();
	}

	void SyncCachedList(const USkeleton* LinkedSkeleton);

	bool BoneHasValidTags(const UBodyStateBone* QueryBone);
	bool SkeletonHasValidTags();
};

UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class UBodyStateAnimInstance : public UAnimInstance
{
public:
	GENERATED_UCLASS_BODY()

	/** Toggle to freeze the tracking at current state. Useful for debugging your anim instance*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BS Anim Instance - Debug")
	bool bFreezeTracking;

	/** Whether the anim instance should autodetect and fill the bonemap on anim init*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	bool bAutoDetectBoneMapAtInit;

	/** Whether the anim instance should map the skeleton rotation on auto map*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	bool bDetectHandRotationDuringAutoMapping;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	EBodyStateAutoRigType AutoMapTarget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	int32 DefaultBodyStateIndex;

	/** Skeleton driving our data */
	UPROPERTY(BlueprintReadWrite, Category = "Bone Anim Struct")
	class UBodyStateSkeleton* BodyStateSkeleton;

	// UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	TMap<EBodyStateBasicBoneType, FBPBoneReference> AutoDetectHandBones(
		USkeletalMeshComponent* Component, EBodyStateAutoRigType RigTargetType = EBodyStateAutoRigType::HAND_LEFT);

	/** Adjust rotation by currently defines offset base rotators */
	UFUNCTION(BlueprintPure, Category = "BS Anim Instance")
	FRotator AdjustRotationByMapBasis(const FRotator& InRotator, const FMappedBoneAnimData& ForMap);

	UFUNCTION(BlueprintPure, Category = "BS Anim Instance")
	FVector AdjustPositionByMapBasis(const FVector& InPosition, const FMappedBoneAnimData& ForMap);

	/** Link given mesh bone with body state bone enum. */
	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void AddBSBoneToMeshBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone, FName MeshBone);

	/** Remove a link. Useful when e.g. autorigging gets 80% there but you need to remove a bone. */
	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void RemoveBSBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone);

	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void SetAnimSkeleton(UBodyStateSkeleton* InSkeleton);

	/** Struct containing all variables needed at anim node time */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	TArray<FMappedBoneAnimData> MappedBoneList;

	UFUNCTION(BlueprintPure, Category = "BS Anim Instance")
	FString BoneMapSummary();

	// Manual sync
	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void SyncMappedBoneDataCache(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData);

	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	static FName GetBoneNameFromRef(const FBPBoneReference& BoneRef);

protected:
	// traverse a bone index node until you hit -1, count the hops
	int32 TraverseLengthForIndex(int32 Index);
	void AddFingerToMap(EBodyStateBasicBoneType BoneType, int32 BoneIndex,
		TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& BoneMap, int32 InBonesPerFinger = 3);

	// Internal Map with parent information
	FBodyStateIndexedBoneList BoneLookupList;
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> IndexedBoneMap;
	TMap<EBodyStateBasicBoneType, FBPBoneReference> ToBoneReferenceMap(
		TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> InIndexedMap);
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> AutoDetectHandIndexedBones(
		USkeletalMeshComponent* Component, EBodyStateAutoRigType RigTargetType = EBodyStateAutoRigType::HAND_LEFT);

	FRotator EstimateAutoMapRotation(const FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType);
	void AutoMapBoneDataForRigType(FMappedBoneAnimData& ForMap, EBodyStateAutoRigType RigTargetType);

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};