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
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "Skeleton/BodyStateSkeleton.h"
#include "BodyStateInputInterface.h"
#include "BodyStateAnimInstance.generated.h"


UENUM(BlueprintType)
enum EBSMultiDeviceMode
{
	BS_MULTI_DEVICE_SINGULAR = 0,
	BS_MULTI_DEVICE_COMBINED
};

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
	FORCEINLINE bool operator<(const FBodyStateIndexedBone& other) const;
};

struct FBodyStateIndexedBoneList
{
	TArray<FBodyStateIndexedBone> Bones;
	TArray<FBodyStateIndexedBone> SortedBones;
	int32 RootBoneIndex;

	// run after filling our index
	TArray<int32> FindBoneWithChildCount(int32 Count);
	void SetFromRefSkeleton(
		const FReferenceSkeleton& RefSkeleton, bool SortBones, EBodyStateAutoRigType HandType, const bool FilterByHand);
	int32 LongestChildTraverseForBone(int32 Bone);

	FBodyStateIndexedBoneList()
	{
		RootBoneIndex = 0;
	}
	int32 TreeIndexFromSortedIndex(int32 SortedIndex);
};

// C++ only struct used for cached bone lookup
USTRUCT(BlueprintType)
struct FCachedBoneLink
{
	GENERATED_USTRUCT_BODY()

	FBoneReference MeshBone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	UBodyStateBone* BSBone = nullptr;
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
	GENERATED_USTRUCT_BODY()

	FMappedBoneAnimData();
	/** Whether the mesh should deform to match the tracked data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	bool bShouldDeformMesh;

	/** List of tags required by the tracking solution for this animation to use that data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	TArray<FString> TrackingTagLimit;

	/** Offset rotation base applied before given rotation (will rotate input) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance", meta = (MakeEditWidget = true))
	FRotator PreBaseRotation;

	/** Transform applied after rotation changes to all bones in map. Consider this an offset */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance", meta = (MakeEditWidget = true))
	FTransform OffsetTransform;

	/** Matching list of body state bone keys mapped to local mesh bone names */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Anim Struct")
	TMap<EBodyStateBasicBoneType, FBPBoneReference> BoneMap;

	/** Skeleton driving mapped data */
	UPROPERTY(BlueprintReadWrite, Category = "Bone Anim Struct")
	UBodyStateSkeleton* BodyStateSkeleton;

	/** Estimated Elbow length */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Anim Struct")
	float ElbowLength;

	/** Flip the chirality of the hand model (for model re-use across left to right or right to left hands) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Anim Struct")
	bool FlipModelLeftRight;

	/** Calculated hand length by walking the bones from palm to middle fingertip */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Anim Struct")
	float HandModelLength;

	/** Model finger tip lengths, calculated on AutoMap */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<float> FingerTipLengths;
	
	/** Original scale of the model, used for auto scaling calculations */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	FVector OriginalScale;


	/** Auto calculated rotation to correct/normalize model rotation*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map", meta = (MakeEditWidget = true))
	FQuat AutoCorrectRotation;

	
	// Data structure containing a parent -> child ordered bone list
	UPROPERTY(BlueprintReadWrite, Category = "Bone Anim Struct")
	TArray<FCachedBoneLink> CachedBoneList;

	

	void SyncCachedList(const USkeleton* LinkedSkeleton);

	bool BoneHasValidTags(const UBodyStateBone* QueryBone);
	bool SkeletonHasValidTags();
};
USTRUCT(BlueprintType)
struct FBoneSearchNames
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<FString> ArmNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<FString> WristNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<FString> ThumbNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<FString> IndexNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<FString> MiddleNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<FString> RingNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	TArray<FString> PinkyNames;

	FBoneSearchNames()
	{
		ArmNames = {"elbow", "upperArm"};
		WristNames = {"wrist", "hand", "palm"};
		ThumbNames = {"thumb"};
		IndexNames = {"index"};
		MiddleNames = {"middle"};
		RingNames = {"ring"};
		PinkyNames = {"pinky", "little"};
	}
};
UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class BODYSTATE_API UBodyStateAnimInstance : public UAnimInstance, public IBodyStateDeviceChangeListener
{
public:
	GENERATED_UCLASS_BODY()
	virtual ~UBodyStateAnimInstance();
	/** Toggle to freeze the tracking at current state. Useful for debugging your anim instance*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BS Anim Instance - Debug")
	bool bFreezeTracking;

	/** Whether the anim instance should map the skeleton rotation on auto map*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	bool bDetectHandRotationDuringAutoMapping;

	/** Whether to include the metacarpels bones when auto mapping (this can distort the palm mesh)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	bool bIncludeMetaCarpels;

	/** Sort the bone names alphabetically when auto mapping rather than by bone order*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	bool bUseSortedBoneNames;

	/** Automatically scale the model to the user's hands */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling")
	bool ScaleModelToTrackingData;
	
	/** Ignore the wrist translation */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling")
	bool IgnoreWristTranslation;

	/** Derive the elbow position from the wrist (useful for Orion tracking)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling")
	bool GuessElbowPosition;

	/** User entered scale offset to fit to entire model for hand auto scaling */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling",
		meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "3.0", ClampMax = "3.0"))
	float ModelScaleOffset;

	/** User entered scale offset to fit to fingertip model to hand for hand auto scaling */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling",
		meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "3.0", ClampMax = "3.0"))
	float ThumbTipScaleOffset;

	/** User entered scale offset to fit to fingertip model to hand for hand auto scaling */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling",
		meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "3.0", ClampMax = "3.0"))
	float IndexTipScaleOffset;

	/** User entered scale offset to fit to fingertip model to hand for hand auto scaling */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling",
		meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "3.0", ClampMax = "3.0"))
	float MiddleTipScaleOffset;

	/** User entered scale offset to fit to fingertip model to hand for hand auto scaling */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling",
		meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "3.0", ClampMax = "3.0"))
	float RingTipScaleOffset;

	/** User entered scale offset to fit to fingertip model to hand for hand auto scaling */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Scaling",
		meta = (UIMin = "0.0", ClampMin = "0.0", UIMax = "3.0", ClampMax = "3.0"))
	float PinkyTipScaleOffset;


	/** Auto detection names (e.g. index thumb etc.)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	FBoneSearchNames SearchNames;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Auto Map")
	EBodyStateAutoRigType AutoMapTarget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	int32 DefaultBodyStateIndex;

	/** Skeleton driving our data */
	UPROPERTY(BlueprintReadWrite, Category = "Bone Anim Struct")
	class UBodyStateSkeleton* BodyStateSkeleton;

	// UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	TMap<EBodyStateBasicBoneType, FBPBoneReference> AutoDetectHandBones(
		USkeletalMeshComponent* Component, EBodyStateAutoRigType RigTargetType, bool& Success, TArray<FString>& FailedBones);

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance")
	FRotator DebugAddRotation;

	UFUNCTION(BlueprintPure, Category = "BS Anim Instance")
	FString BoneMapSummary();

	// Manual sync
	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	void SyncMappedBoneDataCache(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData);

	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	static FName GetBoneNameFromRef(const FBPBoneReference& BoneRef);

	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance")
	static const FName& GetMeshBoneNameFromCachedBoneLink(const FCachedBoneLink& CachedBoneLink);

	UFUNCTION()
	FTransform GetCurrentWristPose(const FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType) const;

	UFUNCTION()
	bool CalcIsTracking();

	FThreadSafeBool IsTracking;

	UFUNCTION()
	void ExecuteAutoMapping();
	
	/** Multidevice configuration, Singular subscribes to a single device.
	Combined subscribes to multiple devices combined into one device
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "BS Anim Instance - Multi device")
	TEnumAsByte<EBSMultiDeviceMode> MultiDeviceMode;


	/** Available device list
	 */
	UPROPERTY(BlueprintReadOnly, Category = "BS Anim Instance - Multi device")
	TArray<FString> AvailableDeviceSerials;
	
	/** Active Device (Singular mode only)
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BS Anim Instance - Multi device",
		meta = (GetOptions = "GetSerialOptions",
			EditCondition = "MultiDeviceMode == EBSMultiDeviceMode::BS_MULTI_DEVICE_SINGULAR"))
	FString ActiveDeviceSerial;

	UPROPERTY()
	TMap<FString, int32> DeviceSerialToDeviceID;

	UFUNCTION(CallInEditor)
	TArray<FString> GetSerialOptions() const
	{
		TArray<FString> Ret;
		for (auto& Serial : AvailableDeviceSerials)
		{
			// we don't want to select combined devices
			if (Serial.Contains("Combined"))
			{
				continue;
			}
			Ret.Add(Serial);
		}
	
		Ret.Insert(TEXT("None"), 0);
		return Ret;
	}
	/** Combined device list
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BS Anim Instance - Multi device",
		meta = (GetOptions = "GetSerialOptions",
			EditCondition = "MultiDeviceMode == EBSMultiDeviceMode::BS_MULTI_DEVICE_COMBINED"))
	TArray<FString> CombinedDeviceSerials;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "BS Anim Instance - Multi device",
		meta = (GetOptions = "GetSerialOptions",
			EditCondition = "MultiDeviceMode == EBSMultiDeviceMode::BS_MULTI_DEVICE_COMBINED"))
	TEnumAsByte<EBSDeviceCombinerClass> DeviceCombinerClass;

	UFUNCTION(BlueprintCallable, Category = "BS Anim Instance - Multi device")
	void SetActiveDeviceSerial(const FString& DeviceID);
	// IBodyStateDeviceChangeListener
	virtual void OnDeviceAdded(const FString& DeviceSerial, const uint32 DeviceID) override;
	virtual void OnDeviceRemoved(const uint32 DeviceID) override;
	virtual void OnDefaultDeviceChanged() override;

protected:
	// traverse a bone index node until you hit -1, count the hops
	int32 TraverseLengthForIndex(int32 Index);
	void AddFingerToMap(EBodyStateBasicBoneType BoneType, int32 BoneIndex,
		TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& BoneMap, int32 InBonesPerFinger = 3);

	static void AddEmptyFingerToMap(EBodyStateBasicBoneType BoneType, TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& BoneMap,
		int32 InBonesPerFinger = 3);

	// Internal Map with parent information
	FBodyStateIndexedBoneList BoneLookupList;
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> IndexedBoneMap;
	TMap<EBodyStateBasicBoneType, FBPBoneReference> ToBoneReferenceMap(
		TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> InIndexedMap);
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> AutoDetectHandIndexedBones(
		USkeletalMeshComponent* Component, EBodyStateAutoRigType RigTargetType, bool& Success, TArray<FString>& FailedBones);

	void EstimateAutoMapRotation(FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType);
	float CalculateElbowLength(const FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType);
	
	// Calculate and store the hand size for auto scaling (the distance from palm to middle finger of the model)
	void CalculateHandSize(FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType);

	// Calculate and store the hand size for auto scaling (the distance from palm to middle finger of the model)
	void CalculateFingertipSizes(FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType);


	// using node items, beware node items are NOT in component space
	FTransform GetTransformFromBoneEnum(const FMappedBoneAnimData& ForMap, const EBodyStateBasicBoneType BoneType,
		const TArray<FName>& Names, const TArray<FNodeItem>& NodeItems, bool& BoneFound) const;

	void AutoMapBoneDataForRigType(
		FMappedBoneAnimData& ForMap, EBodyStateAutoRigType RigTargetType, bool& Success, TArray<FString>& FailedBones);
	TArray<int32> SelectBones(const TArray<FString>& Definitions);
	int32 SelectFirstBone(const TArray<FString>& Definitions);

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void HandleLeftRightFlip(FMappedBoneAnimData& ForMap);

	static void CreateEmptyBoneMap(
		TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& AutoBoneMap, const EBodyStateAutoRigType HandType);

	static const int32 InvalidBone = -1;
	static const int32 NoMetaCarpelsFingerBoneCount = 3;

private:
	bool GetNamesAndTransforms(TArray<FTransform>& ComponentSpaceTransforms, TArray<FName>& Names, TArray<FNodeItem>& NodeItems) const;
	void UpdateDeviceList();

public:
#if WITH_EDITOR
	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyThatChanged the property that was modified
	 */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was
	 * actually modified is located at the tail of the list.  The head of the list of the FStructProperty member variable that
	 * contains the property that was modified.
	 */
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent);
#endif	  // WITH_EDITOR

	int32 GetDeviceIDFromDeviceSerial(const FString& DeviceSerial);
	int32 GetActiveDeviceID();
	UBodyStateSkeleton* GetCurrentSkeleton();
};