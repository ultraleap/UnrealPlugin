

#include "BodyStateAnimInstance.h"

#include "BodyStateBPLibrary.h"
#include "BodyStateUtility.h"
#include "Kismet/KismetMathLibrary.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#include "PersonaUtils.h"
#endif
// static
FName UBodyStateAnimInstance::GetBoneNameFromRef(const FBPBoneReference& BoneRef)
{
	return BoneRef.MeshBone.BoneName;
}

UBodyStateAnimInstance::UBodyStateAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Defaults
	DefaultBodyStateIndex = 0;
	bIncludeMetaCarpels = true;

	AutoMapTarget = EBodyStateAutoRigType::HAND_LEFT;
}

void UBodyStateAnimInstance::AddBSBoneToMeshBoneLink(
	UPARAM(ref) FMappedBoneAnimData& InMappedBoneData, EBodyStateBasicBoneType BSBone, FName MeshBone)
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
		// Re-cache our results
		SyncMappedBoneDataCache(Map);
	}
}

TMap<EBodyStateBasicBoneType, FBPBoneReference> UBodyStateAnimInstance::AutoDetectHandBones(
	USkeletalMeshComponent* Component, EBodyStateAutoRigType RigTargetType, bool& Success, TArray<FString>& FailedBones)
{
	auto IndexedMap = AutoDetectHandIndexedBones(Component, RigTargetType, Success, FailedBones);
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

	// Concatenate indexed bones
	for (auto Bone : IndexedBoneMap)
	{
		FBodyStateIndexedBone& IndexedBone = Bone.Value;
		Result += FString::Printf(
			TEXT("BoneString: %s:%d(%d)\n"), *IndexedBone.BoneName.ToString(), IndexedBone.Index, IndexedBone.ParentIndex);
	}
	return Result;
}

void UBodyStateAnimInstance::SyncMappedBoneDataCache(UPARAM(ref) FMappedBoneAnimData& InMappedBoneData)
{
	InMappedBoneData.SyncCachedList(CurrentSkeleton);
}

//////////////
/////Protected

// Local data only used for auto-mapping algorithm
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

	// run after filling our index
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

int32 UBodyStateAnimInstance::SelectFirstBone(const TArray<FString>& Definitions)
{
	TArray<int32> IndicesFound = SelectBones(Definitions);
	if (IndicesFound.Num())
	{
		return IndicesFound[0];
	}
	else
	{
		return -1;
	}
}
TArray<int32> UBodyStateAnimInstance::SelectBones(const TArray<FString>& Definitions)
{
	TArray<int32> BoneIndicesFound;

	for (auto& Definition : Definitions)
	{
		for (auto& Bone : BoneLookupList.SortedBones)
		{
			const FString& CompareString = Bone.BoneName.ToString().ToLower();
			if (CompareString.Contains(Definition))
			{
				BoneIndicesFound.Add(Bone.Index);
			}
		}
	}
	return BoneIndicesFound;
}

TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> UBodyStateAnimInstance::AutoDetectHandIndexedBones(
	USkeletalMeshComponent* Component, EBodyStateAutoRigType HandType, bool& Success, TArray<FString>& FailedBones)
{
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> AutoBoneMap;
	Success = true;
	if (Component == nullptr)
	{
		return AutoBoneMap;
	}

	// Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;

#if ENGINE_MAJOR_VERSION >= 5 || (ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 27)
	FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
#else
	FReferenceSkeleton& RefSkeleton = SkeletalMesh->RefSkeleton;
#endif

	// Finger roots
	int32 ThumbBone = InvalidBone;
	int32 IndexBone = InvalidBone;
	int32 MiddleBone = InvalidBone;
	int32 RingBone = InvalidBone;
	int32 PinkyBone = InvalidBone;

	// Re-organize our bone information
	BoneLookupList.SetFromRefSkeleton(RefSkeleton, bUseSortedBoneNames);

	int32 WristBone = InvalidBone;
	int32 LowerArmBone = InvalidBone;

	WristBone = SelectFirstBone(SearchNames.WristNames);
	LowerArmBone = SelectFirstBone(SearchNames.ArmNames);
	ThumbBone = SelectFirstBone(SearchNames.ThumbNames);
	IndexBone = SelectFirstBone(SearchNames.IndexNames);
	MiddleBone = SelectFirstBone(SearchNames.MiddleNames);
	RingBone = SelectFirstBone(SearchNames.RingNames);
	PinkyBone = SelectFirstBone(SearchNames.PinkyNames);

	int32 LongestChildTraverse = NoMetaCarpelsFingerBoneCount;
	if (IndexBone > InvalidBone)
	{
		LongestChildTraverse = BoneLookupList.LongestChildTraverseForBone(BoneLookupList.TreeIndexFromSortedIndex(IndexBone));
	}
	int32 BonesPerFinger = LongestChildTraverse + 1;

	// allow the user to turn off metacarpels
	if (!bIncludeMetaCarpels)
	{
		BonesPerFinger = NoMetaCarpelsFingerBoneCount;
	}
	// UE_LOG(LogTemp, Log, TEXT("T:%d, I:%d, M: %d, R: %d, P: %d"), ThumbBone, IndexBone, MiddleBone, RingBone, PinkyBone);

	// Based on the passed hand type map the indexed bones to our EBodyStateBasicBoneType enums
	if (HandType == EBodyStateAutoRigType::HAND_LEFT)
	{
		if (LowerArmBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_L, BoneLookupList.SortedBones[LowerArmBone]);
		}

		if (WristBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_L, BoneLookupList.SortedBones[WristBone]);
		}

		if (ThumbBone >= 0)
		{
			// Thumbs will always be ~ 3 bones
			AddFingerToMap(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_L, ThumbBone, AutoBoneMap);
		}
		if (IndexBone >= 0)
		{
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
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
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
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
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
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
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_L, PinkyBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_L, PinkyBone, AutoBoneMap);
			}
		}
	}
	// Right Hand
	else
	{
		if (LowerArmBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_R, BoneLookupList.SortedBones[LowerArmBone]);
		}
		if (WristBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_R, BoneLookupList.SortedBones[WristBone]);
		}

		if (ThumbBone >= 0)
		{
			// Thumbs will always be ~ 3 bones
			AddFingerToMap(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R, ThumbBone, AutoBoneMap);
		}

		if (IndexBone >= 0)
		{
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
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
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
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
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
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
			if (BonesPerFinger > NoMetaCarpelsFingerBoneCount)
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_R, PinkyBone, AutoBoneMap, BonesPerFinger);
			}
			else
			{
				AddFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_R, PinkyBone, AutoBoneMap);
			}
		}
	}
	// failure states, it's common not find an arm so don't flag that as fail to map
	if (IndexBone < 0 || RingBone < 0 || PinkyBone < 0 || MiddleBone < 0 || ThumbBone < 0 || WristBone < 0)
	{
		if (IndexBone < 0)
		{
			FailedBones.Add("Index");
		}
		if (RingBone < 0)
		{
			FailedBones.Add("Ring");
		}
		if (MiddleBone < 0)
		{
			FailedBones.Add("Middle");
		}
		if (PinkyBone < 0)
		{
			FailedBones.Add("Pinky");
		}
		if (ThumbBone < 0)
		{
			FailedBones.Add("Thumb");
		}
		if (WristBone < 0)
		{
			FailedBones.Add("Wrist");
		}
	}

	// create empty skeleton for easy config
	if (AutoBoneMap.Num() < 2)
	{
		UE_LOG(LogTemp, Log, TEXT("Auto mapping bone names - Cannot automatically find any bones, please manually map them"));
		Success = false;

		CreateEmptyBoneMap(AutoBoneMap, HandType);
	}
	return AutoBoneMap;
}
void UBodyStateAnimInstance::CreateEmptyBoneMap(
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& AutoBoneMap, const EBodyStateAutoRigType HandType)
{
	static const int BonesPerFinger = 4;
	if (HandType == EBodyStateAutoRigType::HAND_LEFT)
	{
		AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_L, FBodyStateIndexedBone());
		AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_L, FBodyStateIndexedBone());

		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_L, AutoBoneMap);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_L, AutoBoneMap, BonesPerFinger);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_L, AutoBoneMap, BonesPerFinger);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_L, AutoBoneMap, BonesPerFinger);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_L, AutoBoneMap, BonesPerFinger);
	}
	else
	{
		AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_R, FBodyStateIndexedBone());
		AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_R, FBodyStateIndexedBone());

		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_THUMB_0_METACARPAL_R, AutoBoneMap);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_INDEX_0_METACARPAL_R, AutoBoneMap, BonesPerFinger);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_MIDDLE_0_METACARPAL_R, AutoBoneMap, BonesPerFinger);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_RING_0_METACARPAL_R, AutoBoneMap, BonesPerFinger);
		AddEmptyFingerToMap(EBodyStateBasicBoneType::BONE_PINKY_0_METACARPAL_R, AutoBoneMap, BonesPerFinger);
	}
}
FQuat LookRotation(const FVector& lookAt, const FVector& upDirection)
{
	FVector forward = lookAt;
	FVector up = upDirection;

	forward = forward.GetSafeNormal();
	up = up - (forward * FVector::DotProduct(up, forward));
	up = up.GetSafeNormal();

	///////////////////////

	FVector vector = forward.GetSafeNormal();
	FVector vector2 = FVector::CrossProduct(up, vector);
	FVector vector3 = FVector::CrossProduct(vector, vector2);
	float m00 = vector2.X;
	float m01 = vector2.Y;
	float m02 = vector2.Z;
	float m10 = vector3.X;
	float m11 = vector3.Y;
	float m12 = vector3.Z;
	float m20 = vector.X;
	float m21 = vector.Y;
	float m22 = vector.Z;

	float num8 = (m00 + m11) + m22;
	FQuat quaternion = FQuat();
	if (num8 > 0.0f)
	{
		float num = (float) FMath::Sqrt(num8 + 1.0f);
		quaternion.W = num * 0.5f;
		num = 0.5f / num;
		quaternion.X = (m12 - m21) * num;
		quaternion.Y = (m20 - m02) * num;
		quaternion.Z = (m01 - m10) * num;
		return (quaternion);
	}
	if ((m00 >= m11) && (m00 >= m22))
	{
		float num7 = (float) FMath::Sqrt(((1.0f + m00) - m11) - m22);
		float num4 = 0.5f / num7;
		quaternion.X = 0.5f * num7;
		quaternion.Y = (m01 + m10) * num4;
		quaternion.Z = (m02 + m20) * num4;
		quaternion.W = (m12 - m21) * num4;
		return (quaternion);
	}
	if (m11 > m22)
	{
		float num6 = (float) FMath::Sqrt(((1.0f + m11) - m00) - m22);
		float num3 = 0.5f / num6;
		quaternion.X = (m10 + m01) * num3;
		quaternion.Y = 0.5f * num6;
		quaternion.Z = (m21 + m12) * num3;
		quaternion.W = (m20 - m02) * num3;
		return (quaternion);
	}
	float num5 = (float) FMath::Sqrt(((1.0f + m22) - m00) - m11);
	float num2 = 0.5f / num5;
	quaternion.X = (m20 + m02) * num2;
	quaternion.Y = (m21 + m12) * num2;
	quaternion.Z = 0.5f * num5;
	quaternion.W = (m01 - m10) * num2;

	return quaternion;
}
/* Find an orthonormal basis for the set of vectors q
 * using the Gram-Schmidt Orthogonalization process */
void OrthoNormalize2(TArray<FVector>& Vectors)
{
	int i, j;

	for (i = 1; i < Vectors.Num(); ++i)
	{
		for (j = 0; j < i; ++j)
		{
			double scaling_factor = FVector::DotProduct(Vectors[j], Vectors[i]) / FVector::DotProduct(Vectors[j], Vectors[j]);

			/* Subtract each scaled component of q_j from q_i */
			Vectors[i] -= scaling_factor * Vectors[j];
		}
	}

	/* Now normalize all the 'n' orthogonal vectors */
	for (i = 0; i < Vectors.Num(); ++i)
	{
		Vectors[i].Normalize();
	}
}
void OrthNormalize2(FVector& Normal, FVector& Tangent, FVector& Binormal)
{
	TArray<FVector> Vectors = {Normal, Tangent, Binormal};

	OrthoNormalize2(Vectors);

	Normal = Vectors[0];
	Tangent = Vectors[1];
	Binormal = Vectors[2];
}

FTransform UBodyStateAnimInstance::GetTransformFromBoneEnum(const FMappedBoneAnimData& ForMap,
	const EBodyStateBasicBoneType BoneType, const TArray<FName>& Names, const TArray<FNodeItem>& NodeItems, bool& BoneFound) const
{
	FBoneReference Bone = ForMap.BoneMap.Find(BoneType)->MeshBone;
	int Index = Names.Find(Bone.BoneName);

	BoneFound = false;
	if (Index > InvalidBone)
	{
		BoneFound = true;
		return NodeItems[Index].Transform;
	}
	return FTransform();
}
FTransform UBodyStateAnimInstance::GetTransformFromBoneEnum(const FMappedBoneAnimData& ForMap,
	const EBodyStateBasicBoneType BoneType, const TArray<FName>& Names, const TArray<FTransform>& ComponentSpaceTransforms,
	bool& BoneFound) const
{
	FBoneReference Bone = ForMap.BoneMap.Find(BoneType)->MeshBone;
	int Index = Names.Find(Bone.BoneName);

	BoneFound = false;
	if (Index > InvalidBone && Index < ComponentSpaceTransforms.Num())
	{
		BoneFound = true;
		return ComponentSpaceTransforms[Index];
	}
	return FTransform();
}

FTransform UBodyStateAnimInstance::GetCurrentWristPose(
	const FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType) const
{
	FTransform Ret;
	USkeletalMeshComponent* Component = GetSkelMeshComponent();
	// Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;

	TArray<FName> Names;
	TArray<FNodeItem> NodeItems;

	INodeMappingProviderInterface* INodeMapping = Cast<INodeMappingProviderInterface>(SkeletalMesh);

	if (!INodeMapping)
	{
		UE_LOG(LogTemp, Log, TEXT("UBodyStateAnimInstance::GetCurrentWristPose INodeMapping is NULL so cannot proceed"));
		return Ret;
	}

	INodeMapping->GetMappableNodeData(Names, NodeItems);

	EBodyStateBasicBoneType Wrist = EBodyStateBasicBoneType::BONE_HAND_WRIST_L;
	if (RigTargetType == EBodyStateAutoRigType::HAND_RIGHT)
	{
		Wrist = EBodyStateBasicBoneType::BONE_HAND_WRIST_R;
	}
	bool WristBoneFound = false;
	Ret = GetTransformFromBoneEnum(ForMap, Wrist, Names, NodeItems, WristBoneFound);
	return Ret;
}
// for debugging only, calcs debug rotations normalized for Unity (has no effect on the main path through the code)
#define DEBUG_ROTATIONS_AS_UNITY 0
#if DEBUG_ROTATIONS_AS_UNITY
// useful for comparing with Unity when debugging (unused)
void Normalize360(FRotator& InPlaceRot)
{
	if (InPlaceRot.Yaw < 0)
	{
		InPlaceRot.Yaw += 360;
	}
	if (InPlaceRot.Pitch < 0)
	{
		InPlaceRot.Pitch += 360;
	}
	if (InPlaceRot.Roll < 0)
	{
		InPlaceRot.Roll += 360;
	}
}
#endif	  // DEBUG_ROTATIONS_AS_UNITY

// based on the logic in HandBinderAutoBinder.cs from the Unity Hand Modules.
void UBodyStateAnimInstance::EstimateAutoMapRotation(FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType)
{
	USkeletalMeshComponent* Component = GetSkelMeshComponent();
	const TArray<FTransform>& ComponentSpaceTransforms = Component->GetComponentSpaceTransforms();
	// Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	TArray<FName> Names;
	TArray<FNodeItem> NodeItems;
	INodeMappingProviderInterface* INodeMapping = Cast<INodeMappingProviderInterface>(SkeletalMesh);

	if (!INodeMapping)
	{
		UE_LOG(LogTemp, Log, TEXT("UBodyStateAnimInstance::EstimateAutoMapRotation INodeMapping is NULL so cannot proceed"));
		return;
	}

	INodeMapping->GetMappableNodeData(Names, NodeItems);

	EBodyStateBasicBoneType Index = EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_L;
	EBodyStateBasicBoneType Middle = EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_L;
	EBodyStateBasicBoneType Pinky = EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_L;
	EBodyStateBasicBoneType Wrist = EBodyStateBasicBoneType::BONE_HAND_WRIST_L;
	if (RigTargetType == EBodyStateAutoRigType::HAND_RIGHT)
	{
		Index = EBodyStateBasicBoneType::BONE_INDEX_1_PROXIMAL_R;
		Middle = EBodyStateBasicBoneType::BONE_MIDDLE_1_PROXIMAL_R;
		Pinky = EBodyStateBasicBoneType::BONE_PINKY_1_PROXIMAL_R;
		Wrist = EBodyStateBasicBoneType::BONE_HAND_WRIST_R;
	}

	auto RefIndex = ForMap.BoneMap.Find(Index);

	if (!RefIndex)
	{
		ForMap.AutoCorrectRotation = FQuat(FRotator(ForceInitToZero));

		UE_LOG(LogTemp, Log, TEXT("UBodyStateAnimInstance::EstimateAutoMapRotation Cannot find the index bone"));

		return;
	}
	FBoneReference IndexBone = RefIndex->MeshBone;
	bool IndexBoneFound = false;
	bool MiddleBoneFound = false;
	bool PinkyBoneFound = false;
	bool WristBoneFound = false;

	FTransform IndexPose = GetTransformFromBoneEnum(ForMap, Index, Names, ComponentSpaceTransforms, IndexBoneFound);
	FTransform MiddlePose = GetTransformFromBoneEnum(ForMap, Middle, Names, ComponentSpaceTransforms, MiddleBoneFound);
	FTransform PinkyPose = GetTransformFromBoneEnum(ForMap, Pinky, Names, ComponentSpaceTransforms, PinkyBoneFound);
	FTransform WristPose = GetTransformFromBoneEnum(ForMap, Wrist, Names, ComponentSpaceTransforms, WristBoneFound);

	if (!(IndexBoneFound && MiddleBoneFound && PinkyBoneFound && WristBoneFound))
	{
		ForMap.AutoCorrectRotation = FQuat(FRotator(ForceInitToZero));
		UE_LOG(LogTemp, Log, TEXT("UBodyStateAnimInstance::EstimateAutoMapRotation Cannot find all finger bones"));

		return;
	}
	FBoneReference Bone = ForMap.BoneMap.Find(Wrist)->MeshBone;

	// Calculate the Model's rotation
	// direct port from c# Unity version HandBinderAutoBinder.cs
	FVector Forward = MiddlePose.GetLocation() - WristPose.GetLocation();
	FVector Right = IndexPose.GetLocation() - PinkyPose.GetLocation();

	if (RigTargetType == EBodyStateAutoRigType::HAND_RIGHT)
	{
		Right = -Right;
	}
	FVector Up = FVector::CrossProduct(Forward, Right);
	// we need a three param version of this.
	OrthNormalize2(Forward, Up, Right);

	// in Unity this was Quat.LookRotation(forward,up).
	FQuat ModelRotation;
	ModelRotation = LookRotation(Forward, Up);

	// In unity, this came from the wrist in the world/scene coords
	FQuat WristPoseQuat(WristPose.GetRotation());

#if DEBUG_ROTATIONS_AS_UNITY
	// debug
	FRotator WristSourceRotation = WristPose.GetRotation().Rotator();
	Normalize360(WristSourceRotation);

	FRotator ModelDebugRotation = ModelRotation.Rotator();
	Normalize360(ModelDebugRotation);
	// end debug
#endif	  // DEBUG_ROTATIONS_AS_UNITY

	FRotator WristRotation = (ModelRotation.Inverse() * WristPoseQuat).Rotator();

#if DEBUG_ROTATIONS_AS_UNITY
	FRotator WristDebugRotation = WristRotation;
	Normalize360(WristDebugRotation);
#endif	  // DEBUG_ROTATIONS_AS_UNITY

	// correct to UE space as defined by control hands
	if (ForMap.FlipModelLeftRight)
	{
		WristRotation += FRotator(-90, 0, -180);
	}
	else
	{
		WristRotation += FRotator(90, 0, 0);
	}
#if DEBUG_ROTATIONS_AS_UNITY
	WristDebugRotation = WristRotation;
	Normalize360(WristDebugRotation);
#endif	  // DEBUG_ROTATIONS_AS_UNITY
	ForMap.AutoCorrectRotation = FQuat(WristRotation);
}
float UBodyStateAnimInstance::CalculateElbowLength(const FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType)
{
	float ElbowLength = 0;
	USkeletalMeshComponent* Component = GetSkelMeshComponent();
	// Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	TArray<FName> Names;
	TArray<FNodeItem> NodeItems;
	INodeMappingProviderInterface* INodeMapping = Cast<INodeMappingProviderInterface>(SkeletalMesh);

	if (!INodeMapping)
	{
		UE_LOG(LogTemp, Log, TEXT("UBodyStateAnimInstance::EstimateAutoMapRotation INodeMapping is NULL so cannot proceed"));
		return 0;
	}

	INodeMapping->GetMappableNodeData(Names, NodeItems);

	EBodyStateBasicBoneType LowerArm = EBodyStateBasicBoneType::BONE_LOWERARM_L;
	EBodyStateBasicBoneType Wrist = EBodyStateBasicBoneType::BONE_HAND_WRIST_L;

	if (RigTargetType == EBodyStateAutoRigType::HAND_RIGHT)
	{
		LowerArm = EBodyStateBasicBoneType::BONE_LOWERARM_R;
		Wrist = EBodyStateBasicBoneType::BONE_HAND_WRIST_R;
	}
	FBoneReference LowerArmBone;

	const FBPBoneReference* MapRef = ForMap.BoneMap.Find(LowerArm);
	if (MapRef)
	{
		LowerArmBone = MapRef->MeshBone;
	}
	FBoneReference WristBone;

	MapRef = ForMap.BoneMap.Find(Wrist);
	if (MapRef)
	{
		WristBone = MapRef->MeshBone;
	}
	int32 LowerArmBoneIndex = Names.Find(LowerArmBone.BoneName);
	int32 WristBoneIndex = Names.Find(WristBone.BoneName);

	if (LowerArmBoneIndex > InvalidBone && WristBoneIndex > InvalidBone)
	{
		FTransform LowerArmPose = NodeItems[LowerArmBoneIndex].Transform;
		FTransform WristPose = NodeItems[WristBoneIndex].Transform;

		ElbowLength = FVector::Distance(WristPose.GetLocation(), LowerArmPose.GetLocation());
	}
	return ElbowLength;
}
void UBodyStateAnimInstance::AutoMapBoneDataForRigType(
	FMappedBoneAnimData& ForMap, EBodyStateAutoRigType RigTargetType, bool& Success, TArray<FString>& FailedBones)
{
	// Grab our skel mesh component
	USkeletalMeshComponent* Component = GetSkelMeshComponent();

	IndexedBoneMap = AutoDetectHandIndexedBones(Component, RigTargetType, Success, FailedBones);
	auto OldMap = ForMap.BoneMap;
	ForMap.BoneMap = ToBoneReferenceMap(IndexedBoneMap);

	// Default preset rotations - Note order is different than in BP
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
	ForMap.ElbowLength = CalculateElbowLength(ForMap, RigTargetType);
	// Reset specified keys from defaults
	/*for (auto Pair : OldMap)
	{
		Pair.Value.MeshBone.Initialize(CurrentSkeleton);
		ForMap.BoneMap.Add(Pair.Key, Pair.Value);
	}*/
}

int32 UBodyStateAnimInstance::TraverseLengthForIndex(int32 Index)
{
	if (Index == InvalidBone || Index >= BoneLookupList.Bones.Num())
	{
		return 0;	 // this is the root or invalid bone
	}
	else
	{
		FBodyStateIndexedBone& Bone = BoneLookupList.Bones[Index];

		// Add our parent traversal + 1
		return TraverseLengthForIndex(Bone.ParentIndex) + 1;
	}
}
void UBodyStateAnimInstance::AddEmptyFingerToMap(EBodyStateBasicBoneType BoneType,
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& BoneMap, int32 InBonesPerFinger /*= BonesPerFinger*/)
{
	int32 FingerRoot = (int32) BoneType;
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot), FBodyStateIndexedBone());
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 1), FBodyStateIndexedBone());
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 2), FBodyStateIndexedBone());
	if (InBonesPerFinger > NoMetaCarpelsFingerBoneCount)
	{
		BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 3), FBodyStateIndexedBone());
	}
}

void UBodyStateAnimInstance::AddFingerToMap(EBodyStateBasicBoneType BoneType, int32 BoneIndex,
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& BoneMap, int32 InBonesPerFinger /*= BonesPerFinger*/)
{
	int32 FingerRoot = (int32) BoneType;
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot), BoneLookupList.SortedBones[BoneIndex]);
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 1), BoneLookupList.SortedBones[BoneIndex + 1]);
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 2), BoneLookupList.SortedBones[BoneIndex + 2]);
	if (InBonesPerFinger > NoMetaCarpelsFingerBoneCount)
	{
		BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 3), BoneLookupList.SortedBones[BoneIndex + 3]);
	}
}

TMap<EBodyStateBasicBoneType, FBPBoneReference> UBodyStateAnimInstance::ToBoneReferenceMap(
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> InIndexedMap)
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
void UBodyStateAnimInstance::HandleLeftRightFlip(const FMappedBoneAnimData& ForMap)
{
	// the user can manually specify that the model is flipped (left to right or right to left)
	// Setting the scale on the component flips the view
	// if we do this here, the anim preview works as well as in scene and in actors
	USkeletalMeshComponent* Component = GetSkelMeshComponent();

	if (!Component)
	{
		return;
	}
	if (ForMap.FlipModelLeftRight)
	{
		Component->SetRelativeScale3D(FVector(1, 1, -1));
		// Unreal doesn't deal with mirrored scale when in comes to updating the mesh bounds
		// this means that at 1 bounds scale, the skeletel mesh gets occluded as the bounds are not following the skeleton
		// Until this is fixed in the engine, we have to force the bounds to be huge to always render.
		Component->SetBoundsScale(10);
	}
	else
	{
		Component->SetRelativeScale3D(FVector(1, 1, 1));
		Component->SetBoundsScale(1);
	}
}
void UBodyStateAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Get our default bodystate skeleton
	UBodyStateSkeleton* Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
	SetAnimSkeleton(Skeleton);

	// One hand mapping
	if (AutoMapTarget != EBodyStateAutoRigType::BOTH_HANDS)
	{
		if (MappedBoneList.Num() > 0)
		{
			FMappedBoneAnimData& OneHandMap = MappedBoneList[0];
			HandleLeftRightFlip(OneHandMap);
			if (bDetectHandRotationDuringAutoMapping)
			{
				EstimateAutoMapRotation(OneHandMap, AutoMapTarget);
			}
			else
			{
				OneHandMap.AutoCorrectRotation = FQuat(FRotator(ForceInitToZero));
			}
		}
	}
	else
	{
		// Make two maps if missing
		if (MappedBoneList.Num() > 1)
		{
			// Map one hand each
			FMappedBoneAnimData& LeftHandMap = MappedBoneList[0];
			FMappedBoneAnimData& RightHandMap = MappedBoneList[1];

			HandleLeftRightFlip(LeftHandMap);
			HandleLeftRightFlip(RightHandMap);

			if (bDetectHandRotationDuringAutoMapping)
			{
				EstimateAutoMapRotation(LeftHandMap, EBodyStateAutoRigType::HAND_LEFT);
				EstimateAutoMapRotation(RightHandMap, EBodyStateAutoRigType::HAND_RIGHT);
			}
			else
			{
				RightHandMap.AutoCorrectRotation = LeftHandMap.AutoCorrectRotation = FQuat(FRotator(ForceInitToZero));
			}
		}
	}

	// Cache all results
	if (BodyStateSkeleton == nullptr)
	{
		BodyStateSkeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		SetAnimSkeleton(BodyStateSkeleton);	   // this will sync all the bones
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

	// SN: may want to optimize this at some pt
	if (BodyStateSkeleton == nullptr)
	{
		BodyStateSkeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		SetAnimSkeleton(BodyStateSkeleton);
	}

	if (BodyStateSkeleton)
	{
		BodyStateSkeleton->bTrackingActive = !bFreezeTracking;
		IsTracking = CalcIsTracking();
	}
}
// static
const FName& UBodyStateAnimInstance::GetMeshBoneNameFromCachedBoneLink(const FCachedBoneLink& CachedBoneLink)
{
	return CachedBoneLink.MeshBone.BoneName;
}
// do not call this from the anim thread
bool UBodyStateAnimInstance::CalcIsTracking()
{
	if (!BodyStateSkeleton)
	{
		return false;
	}

	bool Ret = false;
	switch (AutoMapTarget)
	{
		case EBodyStateAutoRigType::HAND_LEFT:
		{
			Ret = BodyStateSkeleton->LeftArm()->Hand->Wrist->IsTracked();
		}
		break;
		case EBodyStateAutoRigType::HAND_RIGHT:
		{
			Ret = BodyStateSkeleton->RightArm()->Hand->Wrist->IsTracked();
		}
		break;
		case EBodyStateAutoRigType::BOTH_HANDS:
		{
			Ret = BodyStateSkeleton->LeftArm()->Hand->Wrist->IsTracked() || BodyStateSkeleton->RightArm()->Hand->Wrist->IsTracked();
		}
		break;
	}
	return Ret;
}
void UBodyStateAnimInstance::ExecuteAutoMapping()
{
	bool AutoMapSuccess = false;
	TArray<FString> FailedBones;
	// One hand mapping
	if (AutoMapTarget != EBodyStateAutoRigType::BOTH_HANDS)
	{
		// Make one map if missing
		if (MappedBoneList.Num() < 1)
		{
			FMappedBoneAnimData Map;
			MappedBoneList.Add(Map);
		}

		FMappedBoneAnimData& OneHandMap = MappedBoneList[0];

		// reset on auto map button, otherwise flipping left to right won't set this up
		OneHandMap.PreBaseRotation = FRotator::ZeroRotator;
		AutoMapBoneDataForRigType(OneHandMap, AutoMapTarget, AutoMapSuccess, FailedBones);
	}
	// Two hand mapping
	else
	{
		// Make two maps if missing
		while (MappedBoneList.Num() < 2)
		{
			FMappedBoneAnimData Map;
			MappedBoneList.Add(Map);
		}
		// Map one hand each
		FMappedBoneAnimData& LeftHandMap = MappedBoneList[0];
		AutoMapBoneDataForRigType(LeftHandMap, EBodyStateAutoRigType::HAND_LEFT, AutoMapSuccess, FailedBones);

		FMappedBoneAnimData& RightHandMap = MappedBoneList[1];
		AutoMapBoneDataForRigType(RightHandMap, EBodyStateAutoRigType::HAND_RIGHT, AutoMapSuccess, FailedBones);
	}

	// One hand mapping
	if (AutoMapTarget != EBodyStateAutoRigType::BOTH_HANDS)
	{
		if (MappedBoneList.Num() > 0)
		{
			FMappedBoneAnimData& OneHandMap = MappedBoneList[0];
			HandleLeftRightFlip(OneHandMap);
			if (bDetectHandRotationDuringAutoMapping)
			{
				EstimateAutoMapRotation(OneHandMap, AutoMapTarget);
			}
			else
			{
				OneHandMap.AutoCorrectRotation = FQuat(FRotator(ForceInitToZero));
			}
		}
	}
	else
	{
		// Make two maps if missing
		if (MappedBoneList.Num() > 1)
		{
			// Map one hand each
			FMappedBoneAnimData& LeftHandMap = MappedBoneList[0];
			FMappedBoneAnimData& RightHandMap = MappedBoneList[1];

			HandleLeftRightFlip(LeftHandMap);
			HandleLeftRightFlip(RightHandMap);

			if (bDetectHandRotationDuringAutoMapping)
			{
				EstimateAutoMapRotation(LeftHandMap, EBodyStateAutoRigType::HAND_LEFT);
				EstimateAutoMapRotation(RightHandMap, EBodyStateAutoRigType::HAND_RIGHT);
			}
			else
			{
				RightHandMap.AutoCorrectRotation = LeftHandMap.AutoCorrectRotation = FQuat(FRotator(ForceInitToZero));
			}
		}
	}

	// Cache all results
	if (BodyStateSkeleton == nullptr)
	{
		BodyStateSkeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
		SetAnimSkeleton(BodyStateSkeleton);	   // this will sync all the bones
	}
	else
	{
		for (auto& BoneMap : MappedBoneList)
		{
			SyncMappedBoneDataCache(BoneMap);
		}
	}

#if WITH_EDITOR
	// this is what happens when the user clicks apply in the property previewer
	PersonaUtils::CopyPropertiesToCDO(this);

	FString Title("Ultraleap auto bone mapping");
	FText TitleText = FText::FromString(*Title);
	FString Message("Auto mapping succeeded! Compile to continue");

	if (!AutoMapSuccess)
	{
		Message = "We couldn't automatically map all bones.\n\nThe following bones couldn't be auto mapped:\n\n";

		for (auto BoneName : FailedBones)
		{
			Message += BoneName;
			Message += "\n";
		}
		Message += "\n\nEdit the mappings manually in 'Mapped Bone List -> Bone Map' in the preview panel to continue.";
	}
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(*Message), &TitleText);

#endif
}
#if WITH_EDITOR
void UBodyStateAnimInstance::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UBodyStateAnimInstance::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// if we want to trigger an action when the user manually changed a bone mapping , do it in here
	if (PropertyChangedEvent.GetPropertyName() == "BoneName")
	{
	}
}
#endif	  // WITH_EDITOR
void FMappedBoneAnimData::SyncCachedList(const USkeleton* LinkedSkeleton)
{
	// Clear our current list
	CachedBoneList.Empty();

	// We require a bodystate skeleton to do the mapping
	if (BodyStateSkeleton == nullptr)
	{
		return;
	}

	// Todo: optimize multiple calls with no / small changes

	// 1) traverse indexed bone list, store all the traverse lengths
	// this can happen on an animation worker thread
	// set bUseMultiThreadedAnimationUpdate = false if we want to do everything on engine tick
	FScopeLock ScopeLock(&BodyStateSkeleton->BoneDataLock);

	for (auto Pair : BoneMap)
	{
		FCachedBoneLink TraverseResult;

		TraverseResult.MeshBone = Pair.Value.MeshBone;
		TraverseResult.MeshBone.Initialize(LinkedSkeleton);
		TraverseResult.BSBone = BodyStateSkeleton->BoneForEnum(Pair.Key);

		// Costly function and we don't need it after all, and it won't work anymore now that it depends on external data
		// TraverseResult.TraverseCount = TraverseLengthForIndex(TraverseResult.MeshBone.BoneIndex);

		CachedBoneList.Add(TraverseResult);
	}

	// 2) reorder according to shortest traverse list
	CachedBoneList.Sort([](const FCachedBoneLink& One, const FCachedBoneLink& Two) {
		// return One.TraverseCount < Two.TraverseCount;
		return One.MeshBone.BoneIndex < Two.MeshBone.BoneIndex;
	});

	UE_LOG(LogTemp, Log, TEXT("Bone cache synced: %d"), CachedBoneList.Num());
}

bool FMappedBoneAnimData::BoneHasValidTags(const UBodyStateBone* QueryBone)
{
	// Early exit optimization
	if (TrackingTagLimit.Num() == 0)
	{
		return true;
	}

	FBodyStateBoneMeta UniqueMeta = ((UBodyStateBone*) QueryBone)->UniqueMeta();

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

void FBodyStateIndexedBoneList::SetFromRefSkeleton(const FReferenceSkeleton& RefSkeleton, bool SortBones)
{
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FBodyStateIndexedBone Bone;
		Bone.BoneName = RefSkeleton.GetBoneName(i);
		Bone.ParentIndex = RefSkeleton.GetParentIndex(i);
		Bone.Index = i;
		SortedBones.Add(Bone);
	}
	if (SortBones)
	{
		SortedBones.Sort();

		for (int i = 0; i < SortedBones.Num(); ++i)
		{
			SortedBones[i].Index = i;
		}
	}
	Bones.Empty(RefSkeleton.GetNum());
	for (int32 i = 0; i < RefSkeleton.GetNum(); i++)
	{
		FBodyStateIndexedBone Bone;
		Bone.BoneName = RefSkeleton.GetBoneName(i);
		Bone.ParentIndex = RefSkeleton.GetParentIndex(i);
		Bone.Index = i;

		Bones.Add(Bone);

		// If we're not the root bone, add ourselves to the parent's child list
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
int32 FBodyStateIndexedBoneList::TreeIndexFromSortedIndex(int32 SortedIndex)
{
	auto& Bone = SortedBones[SortedIndex];
	for (auto& BoneTreeBone : Bones)
	{
		if (BoneTreeBone.BoneName == Bone.BoneName)
		{
			return BoneTreeBone.Index;
		}
	}
	return 0;
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
FORCEINLINE bool FBodyStateIndexedBone::operator<(const FBodyStateIndexedBone& Other) const
{
	return BoneName.FastLess(Other.BoneName);
}
