// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "BodyStateAnimInstance.h"
#include "BodyStateUtility.h"
#include "BodyStateBPLibrary.h"


UBodyStateAnimInstance::UBodyStateAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//Defaults
	bAutoDetectBoneMapAtInit = false;
	DefaultBodyStateIndex = 0;
	BonesPerFinger = -1;
	AutoMapTarget = EBodyStateAutoRigType::HAND_LEFT;
}

void UBodyStateAnimInstance::AddBSBoneToMeshBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone, FName MeshBone)
{
	FBoneReference BoneRef;
	BoneRef.BoneName = MeshBone;
	BoneRef.Initialize(CurrentSkeleton);
	FBPBoneReference MeshBPBone;
	MeshBPBone.MeshBone = BoneRef;

	InMappedBoneData.BoneMap.Add(BSBone, MeshBPBone);
	SyncMappedBoneDataCache(InMappedBoneData);
}

void UBodyStateAnimInstance::RemoveBSBoneLink(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone)
{
	InMappedBoneData.BoneMap.Remove(BSBone);
	SyncMappedBoneDataCache(InMappedBoneData);
}

void UBodyStateAnimInstance::SetAnimSkeleton(UBodyStateSkeleton* InSkeleton)
{
	for (auto& Map : MappedBoneList)
	{
		Map.BodyStateSkeleton = InSkeleton;
		//Re-cache our results
		SyncMappedBoneDataCache(Map);
	}
}

TMap<EBodyStateBasicBoneType, FBPBoneReference> UBodyStateAnimInstance::AutoDetectHandBones(USkeletalMeshComponent* Component, EBodyStateAutoRigType RigTargetType /*= EBodyStateAutoRigType::HAND_LEFT*/)
{
	auto IndexedMap = AutoDetectHandIndexedBones(Component, RigTargetType);
	return ToBoneReferenceMap(IndexedMap);
}

FRotator UBodyStateAnimInstance::AdjustRotationByMapBasis(const FRotator& InRotator, const FMappedBoneAnimData& ForMap)
{
	const FRotator Temp = FBodyStateUtility::CombineRotators(ForMap.PreBaseRotation, InRotator);
	return FBodyStateUtility::CombineRotators(Temp, ForMap.OffsetTransform.Rotator());
}

FVector UBodyStateAnimInstance::AdjustPositionByMapBasis(const FVector& InPosition, const FMappedBoneAnimData& ForMap)
{
	return ForMap.OffsetTransform.GetRotation().RotateVector(InPosition);
}

FString UBodyStateAnimInstance::BoneMapSummary()
{
	FString Result = TEXT("+== Bone Summary ==+");

	//Concatenate indexed bones
	for (auto Bone : IndexedBoneMap)
	{
		FBodyStateIndexedBone& IndexedBone = Bone.Value;
		Result += FString::Printf(TEXT("BoneString: %s:%d(%d)\n"), *IndexedBone.BoneName.ToString(), IndexedBone.Index, IndexedBone.ParentIndex);
	}
	return Result;
}

void UBodyStateAnimInstance::SyncMappedBoneDataCache(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData)
{
	InMappedBoneData.SyncCachedList(CurrentSkeleton);
}

//////////////
/////Protected

//Local data only used for auto-mapping algorithm
struct FChildIndices
{
	int32 Index;
	TArray<int32> ChildIndices;
};

struct FIndexParentsCount
{
	TArray<FChildIndices> ParentsList;

	int32 FindCount(int32 ParentIndex)
	{
		int32 DidFindIndex = -1;
		for (int32 i = 0; i < ParentsList.Num(); i++)
		{
			if (ParentsList[i].Index == ParentIndex)
			{
				return i;
			}
		}
		return DidFindIndex;
	}

	//run after filling our index
	TArray<int32> FindParentsWithCount(int32 Count)
	{
		TArray<int32> ResultArray;

		for (auto Parent : ParentsList)
		{
			if (Parent.ChildIndices.Num() == Count)
			{
				ResultArray.Add(Parent.Index);
			}
		}
		return ResultArray;
	}
};

TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> UBodyStateAnimInstance::AutoDetectHandIndexedBones(USkeletalMeshComponent* Component, EBodyStateAutoRigType HandType /*= EBodyStateAutoRigType::HAND_LEFT*/)
{
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> AutoBoneMap;

	if (Component == nullptr)
	{
		return AutoBoneMap;
	}

	//Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	FReferenceSkeleton& RefSkeleton = SkeletalMesh->RefSkeleton;

	//Root bone
	int32 RootBone = -2;

	//Palm bone
	int32 PalmBone = -2;

	//Finger roots
	int32 ThumbBone = -2;
	int32 IndexBone = -2;
	int32 MiddleBone = -2;
	int32 RingBone = -2;
	int32 PinkyBone = -2;

	//Re-organize our bone information
	BoneLookupList.SetFromRefSkeleton(RefSkeleton);

	//Find our palm bone
	TArray<int32> HandParents = BoneLookupList.FindBoneWithChildCount(5);

	//Multiple hand parent bones found, let's pick the closest type
	if (HandParents.Num() > 1)
	{
		FString SearchString;
		FString AltSearchString;
		if (HandType == EBodyStateAutoRigType::HAND_RIGHT)
		{
			SearchString = SearchStrings.RightSearchString;
			AltSearchString = SearchStrings.RightSearchStringAlt;
		}
		else
		{
			SearchString = SearchStrings.LeftSearchString;
			AltSearchString = SearchStrings.LeftSearchStringAlt;
		}

		//Check the multiple bones for L/R (or lowercase) match
		for (int32 Index : HandParents)
		{
			FBodyStateIndexedBone& IndexedBone = BoneLookupList.Bones[Index];
			bool HasSearchString = IndexedBone.BoneName.ToString().Contains(SearchString, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			HasSearchString = HasSearchString || IndexedBone.BoneName.ToString().Contains(AltSearchString, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

			if (HasSearchString)
			{
				PalmBone = Index;
				break;
			}
		}
		//Palm bone still not set?
		if (PalmBone == -2)
		{
			//Pick the first one
			PalmBone = HandParents[0];
		}
	}
	//Single bone of this type found, set that one
	else if (HandParents.Num() == 1)
	{
		PalmBone = HandParents[0];
	}
	else
	{
		//We couldn't figure out where the palm is, return an empty map
		return AutoBoneMap;
	}

	int32 WristBone = RefSkeleton.GetParentIndex(PalmBone);
	int32 LowerArmBone = -1;
	bool bWristIsValid = BoneLookupList.Bones[WristBone].BoneName.ToString().ToLower().Contains(SearchStrings.WristSearchString);
	
	//We likely got the lower arm
	if (!bWristIsValid)
	{
		LowerArmBone = WristBone;
		WristBone = -1;
	}

	//Get all the child bones with that parent index
	for (auto& Bone : BoneLookupList.Bones)
	{
		bool IsPalmChild = (Bone.ParentIndex == PalmBone);

		if (IsPalmChild)
		{
			const FString& CompareString = Bone.BoneName.ToString().ToLower();

			if((ThumbBone == -2) && CompareString.Contains(TEXT("thumb")))
			{
				ThumbBone = Bone.Index;
			}
			else if ((IndexBone == -2) && CompareString.Contains(TEXT("index")))
			{
				IndexBone = Bone.Index;
			}
			else if ((MiddleBone == -2) && CompareString.Contains(TEXT("middle")))
			{
				MiddleBone = Bone.Index;
			}
			else if ((RingBone == -2) && CompareString.Contains(TEXT("ring")))
			{
				RingBone = Bone.Index;
			}
			else if ((PinkyBone == -2) && CompareString.Contains(TEXT("pinky")))
			{
				PinkyBone = Bone.Index;
			}
		}
	}

	//Test the index finger to determine bones per finger
	if (BonesPerFinger == -1)
	{
		int32 LongestChildTraverse = BoneLookupList.LongestChildTraverseForBone(IndexBone);
		BonesPerFinger = LongestChildTraverse + 1;
	}

	//UE_LOG(LogTemp, Log, TEXT("Palm: %d"), PalmBone);
	//UE_LOG(LogTemp, Log, TEXT("T:%d, I:%d, M: %d, R: %d, P: %d"), ThumbBone, IndexBone, MiddleBone, RingBone, PinkyBone);

	//Based on the passed hand type map the indexed bones to our EBodyStateBasicBoneType enums
	if (HandType == EBodyStateAutoRigType::HAND_LEFT)
	{
		if (bWristIsValid)
		{
			if (WristBone >= 0)
			{
				AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_L, BoneLookupList.Bones[WristBone]);
			}
		}
		else
		{
			if (PalmBone >= 0)
			{
				AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_L, BoneLookupList.Bones[PalmBone]);
			}
			if (LowerArmBone >= 0)
			{
				AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_L, BoneLookupList.Bones[LowerArmBone]);
			}
		}
		if (ThumbBone >= 0)
		{
			//Thumbs will always be ~ 3 bones
			AddFingerToMap(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_L, ThumbBone, AutoBoneMap);
		}
		if (IndexBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_L, IndexBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_L, IndexBone, AutoBoneMap);
			}
		}
		if (MiddleBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_L, MiddleBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_L, MiddleBone, AutoBoneMap);
			}
		}
		if (RingBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_L, RingBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_L, RingBone, AutoBoneMap);
			}
		}
		if (PinkyBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_L, PinkyBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_L, PinkyBone, AutoBoneMap);
			}
		}
	}
	//Right Hand
	else
	{
		if (bWristIsValid)
		{
			if (WristBone >= 0)
			{
				AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_R, BoneLookupList.Bones[WristBone]);
			}
		}
		else
		{
			if (PalmBone >= 0)
			{
				AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_R, BoneLookupList.Bones[PalmBone]);
			}
			if (LowerArmBone >= 0)
			{
				AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_R, BoneLookupList.Bones[LowerArmBone]);
			}
		}
		if (ThumbBone >= 0)
		{
			//Thumbs will always be ~ 3 bones
			AddFingerToMap(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R, ThumbBone, AutoBoneMap);
		}
		if (IndexBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_R, IndexBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_R, IndexBone, AutoBoneMap);
			}
		}
		if (MiddleBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_R, MiddleBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_R, MiddleBone, AutoBoneMap);
			}
		}
		if (RingBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_R, RingBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_RING_1_PROXIMAL_R, RingBone, AutoBoneMap);
			}
		}
		if (PinkyBone >= 0)
		{
			if (BonesPerFinger > 3)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_R, PinkyBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_R, PinkyBone, AutoBoneMap);
			}
		}
	}

	return AutoBoneMap;
}

void UBodyStateAnimInstance::AutoMapBoneDataForRigType(FMappedBoneAnimData& ForMap, EBodyStateAutoRigType RigTargetType)
{
	//Grab our skel mesh component
	USkeletalMeshComponent* Component = GetSkelMeshComponent();

	IndexedBoneMap = AutoDetectHandIndexedBones(Component, RigTargetType);
	auto OldMap = ForMap.BoneMap;
	ForMap.BoneMap = ToBoneReferenceMap(IndexedBoneMap);

	//Default preset rotations - Note order is different than in BP
	if (ForMap.PreBaseRotation.IsNearlyZero())
	{
		if (RigTargetType == EBodyStateAutoRigType::HAND_LEFT)
		{
			ForMap.PreBaseRotation = FRotator(0, 0, -90);
		}
		else if (RigTargetType == EBodyStateAutoRigType::HAND_RIGHT)
		{
			ForMap.PreBaseRotation = FRotator(0, 180, 90);
		}
	}

	//Reset specified keys from defaults
	for (auto Pair : OldMap)
	{
		Pair.Value.MeshBone.Initialize(CurrentSkeleton);
		ForMap.BoneMap.Add(Pair.Key, Pair.Value);
	}
}

int32 UBodyStateAnimInstance::TraverseLengthForIndex(int32 Index)
{
	if (Index == -1 || Index>= BoneLookupList.Bones.Num())
	{
		return 0;	//this is the root or invalid bone
	}
	else
	{
		FBodyStateIndexedBone& Bone = BoneLookupList.Bones[Index];
		
		//Add our parent traversal + 1
		return TraverseLengthForIndex(Bone.ParentIndex) + 1;
	}
}

void UBodyStateAnimInstance::AddFingerToMap(EBodyStateBasicBoneType BoneType, int32 BoneIndex, TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& BoneMap, int32 InBonesPerFinger /*= BonesPerFinger*/)
{
	int32 FingerRoot = (int32)BoneType;
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot), BoneLookupList.Bones[BoneIndex]);
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 1), BoneLookupList.Bones[BoneIndex + 1]);
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 2), BoneLookupList.Bones[BoneIndex + 2]);
	if (InBonesPerFinger > 3)
	{
		BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 3), BoneLookupList.Bones[BoneIndex + 3]);
	}
}

TMap<EBodyStateBasicBoneType, FBPBoneReference> UBodyStateAnimInstance::ToBoneReferenceMap(TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> InIndexedMap)
{
	TMap<EBodyStateBasicBoneType, FBPBoneReference> ReferenceMap;

	for (auto BonePair : IndexedBoneMap)
	{
		FBPBoneReference BoneBPReference;
		FBoneReference BoneReference;
		BoneReference.BoneName = BonePair.Value.BoneName;
		BoneReference.Initialize(CurrentSkeleton);

		BoneBPReference.MeshBone = BoneReference;
		ReferenceMap.Add(BonePair.Key, BoneBPReference);
	}
	return ReferenceMap;
}


void UBodyStateAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//Get our default bodystate skeleton
	UBodyStateSkeleton* Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
	SetAnimSkeleton(Skeleton);

	//Try to auto-detect our bones
	if (bAutoDetectBoneMapAtInit)
	{
		//One hand mapping
		if (AutoMapTarget != EBodyStateAutoRigType::BOTH_HANDS)
		{
			//Make one map if missing
			if (MappedBoneList.Num() < 1)
			{
				FMappedBoneAnimData Map;
				MappedBoneList.Add(Map);
			}

			FMappedBoneAnimData& OneHandMap = MappedBoneList[0];

			AutoMapBoneDataForRigType(OneHandMap, AutoMapTarget);
		}
		//Two hand mapping
		else
		{
			//Make two maps if missing
			while (MappedBoneList.Num() < 2)
			{
				FMappedBoneAnimData Map;
				MappedBoneList.Add(Map);
			}
			//Map one hand each
			FMappedBoneAnimData& LeftHandMap = MappedBoneList[0];
			AutoMapBoneDataForRigType(LeftHandMap, EBodyStateAutoRigType::HAND_LEFT);

			FMappedBoneAnimData& RightHandMap = MappedBoneList[1];
			AutoMapBoneDataForRigType(RightHandMap, EBodyStateAutoRigType::HAND_RIGHT);
		}
	}

	//Cache all results
	if (BodyStateSkeleton == nullptr)
	{
		BodyStateSkeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		SetAnimSkeleton(BodyStateSkeleton);	//this will sync all the bones
	}
	else
	{
		for (auto& BoneMap : MappedBoneList)
		{
			SyncMappedBoneDataCache(BoneMap);
		}
	}
}

void UBodyStateAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//SN: may want to optimize this at some pt
	if (BodyStateSkeleton == nullptr)
	{
		BodyStateSkeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		SetAnimSkeleton(BodyStateSkeleton);  
	}

	if (BodyStateSkeleton)
	{
		BodyStateSkeleton->bTrackingActive = !bFreezeTracking;
	}
}

void FMappedBoneAnimData::SyncCachedList(const USkeleton* LinkedSkeleton)
{
	//Clear our current list
	CachedBoneList.Empty();

	//We require a bodystate skeleton to do the mapping
	if (BodyStateSkeleton == nullptr)
	{
		return;
	}

	//Todo: optimize multiple calls with no / small changes

	//1) traverse indexed bone list, store all the traverse lengths

	for (auto Pair : BoneMap)
	{
		CachedBoneLink TraverseResult;

		TraverseResult.MeshBone = Pair.Value.MeshBone;
		TraverseResult.MeshBone.Initialize(LinkedSkeleton);
		TraverseResult.BSBone = BodyStateSkeleton->BoneForEnum(Pair.Key);

		//Costly function and we don't need it after all, and it won't work anymore now that it depends on external data
		//TraverseResult.TraverseCount = TraverseLengthForIndex(TraverseResult.MeshBone.BoneIndex);

		CachedBoneList.Add(TraverseResult);
	}

	//2) reorder according to shortest traverse list
	CachedBoneList.Sort([](const CachedBoneLink& One, const CachedBoneLink& Two) {
		//return One.TraverseCount < Two.TraverseCount;
		return One.MeshBone.BoneIndex < Two.MeshBone.BoneIndex;
	});

	UE_LOG(LogTemp, Log, TEXT("Bone cache synced: %d"), CachedBoneList.Num());
}

bool FMappedBoneAnimData::BoneHasValidTags(const UBodyStateBone* QueryBone)
{
	//Early exit optimization
	if (TrackingTagLimit.Num() == 0)
	{
		return true;
	}

	FBodyStateBoneMeta UniqueMeta = ((UBodyStateBone*)QueryBone)->UniqueMeta();

	for (FString& LimitTag : TrackingTagLimit)
	{
		if (!UniqueMeta.TrackingTags.Contains(LimitTag))
		{
			return false;
		}
	}
	return true;
}

bool FMappedBoneAnimData::SkeletonHasValidTags()
{
	return BodyStateSkeleton->HasValidTrackingTags(TrackingTagLimit);
}

TArray<int32> FBodyStateIndexedBoneList::FindBoneWithChildCount(int32 Count)
{
	TArray<int32> ResultArray;

	for (auto& Bone : Bones)
	{
		if (Bone.Children.Num() == Count)
		{
			ResultArray.Add(Bone.Index);
		}
	}
	return ResultArray;
}

void FBodyStateIndexedBoneList::SetFromRefSkeleton(const FReferenceSkeleton& RefSkeleton)
{
	Bones.Empty(RefSkeleton.GetNum());
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FBodyStateIndexedBone Bone;
		Bone.BoneName = RefSkeleton.GetBoneName(i);
		Bone.ParentIndex = RefSkeleton.GetParentIndex(i);
		Bone.Index = i;

		Bones.Add(Bone);

		//If we're not the root bone, add ourselves to the parent's child list
		if (Bone.ParentIndex != -1)
		{
			Bones[Bone.ParentIndex].Children.Add(Bone.Index);
		}
		else
		{
			RootBoneIndex = i;
		}
	}
}

int32 FBodyStateIndexedBoneList::LongestChildTraverseForBone(int32 BoneIndex)
{
	auto& Bone = Bones[BoneIndex];
	if (Bone.Children.Num() == 0)
	{
		return 0;
	}
	else
	{
		int32 Count = 0;
		for (int32 Child : Bone.Children)
		{
			int32 ChildCount = LongestChildTraverseForBone(Child);
			if (ChildCount > Count)
			{
				Count = ChildCount;
			}
		}
		return Count + 1;
	}
}

