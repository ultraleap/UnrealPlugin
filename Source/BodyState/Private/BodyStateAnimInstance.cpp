// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "BodyStateAnimInstance.h"

#include "BodyStateBPLibrary.h"
#include "BodyStateUtility.h"
#include "Kismet/KismetMathLibrary.h"

// static
FName UBodyStateAnimInstance::GetBoneNameFromRef(const FBPBoneReference& BoneRef)
{
	return BoneRef.MeshBone.BoneName;
}

UBodyStateAnimInstance::UBodyStateAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Defaults
	bAutoDetectBoneMapAtInit = false;
	DefaultBodyStateIndex = 0;

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
	USkeletalMeshComponent* Component, EBodyStateAutoRigType RigTargetType /*= EBodyStateAutoRigType::HAND_LEFT*/)
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
		for (auto& Bone : BoneLookupList.Bones)
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
	USkeletalMeshComponent* Component, EBodyStateAutoRigType HandType /*= EBodyStateAutoRigType::HAND_LEFT*/)
{
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone> AutoBoneMap;

	if (Component == nullptr)
	{
		return AutoBoneMap;
	}

	// Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	FReferenceSkeleton& RefSkeleton = SkeletalMesh->RefSkeleton;

	// Finger roots
	int32 ThumbBone = -2;
	int32 IndexBone = -2;
	int32 MiddleBone = -2;
	int32 RingBone = -2;
	int32 PinkyBone = -2;

	// Re-organize our bone information
	BoneLookupList.SetFromRefSkeleton(RefSkeleton);

	int32 WristBone = -1;
	int32 LowerArmBone = -1;

	TArray<FString> WristNames = {"wrist", "hand", "palm"};
	TArray<FString> ArmNames = {"elbow", "upperArm"};

	WristBone = SelectFirstBone(WristNames);
	LowerArmBone = SelectFirstBone(ArmNames);
	// Get all the child bones with that parent index
	for (auto& Bone : BoneLookupList.Bones)
	{
		const FString& CompareString = Bone.BoneName.ToString().ToLower();

		if ((ThumbBone == -2) && CompareString.Contains(TEXT("thumb")))
		{
			ThumbBone = Bone.Index;
		}
		if ((IndexBone == -2) && CompareString.Contains(TEXT("index")))
		{
			IndexBone = Bone.Index;
		}
		if ((MiddleBone == -2) && CompareString.Contains(TEXT("middle")))
		{
			MiddleBone = Bone.Index;
		}
		if ((RingBone == -2) && CompareString.Contains(TEXT("ring")))
		{
			RingBone = Bone.Index;
		}
		if ((PinkyBone == -2) && (CompareString.Contains(TEXT("pinky")) || CompareString.Contains(TEXT("little"))))
		{
			PinkyBone = Bone.Index;
		}
	}
	int32 LongestChildTraverse = BoneLookupList.LongestChildTraverseForBone(IndexBone);
	int32 BonesPerFinger = LongestChildTraverse + 1;

	// UE_LOG(LogTemp, Log, TEXT("T:%d, I:%d, M: %d, R: %d, P: %d"), ThumbBone, IndexBone, MiddleBone, RingBone, PinkyBone);

	// Based on the passed hand type map the indexed bones to our EBodyStateBasicBoneType enums
	if (HandType == EBodyStateAutoRigType::HAND_LEFT)
	{
		if (LowerArmBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_L, BoneLookupList.Bones[LowerArmBone]);
		}

		if (WristBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_L, BoneLookupList.Bones[WristBone]);
		}

		if (ThumbBone >= 0)
		{
			// Thumbs will always be ~ 3 bones
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
	// Right Hand
	else
	{
		if (LowerArmBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_LOWERARM_R, BoneLookupList.Bones[LowerArmBone]);
		}
		if (WristBone >= 0)
		{
			AutoBoneMap.Add(EBodyStateBasicBoneType::BONE_HAND_WRIST_R, BoneLookupList.Bones[WristBone]);
		}
		if (ThumbBone >= 0)
		{
			// Thumbs will always be ~ 3 bones
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
FQuat MyLookRotation(const FVector& lookAt, const FVector& upDirection)
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
void OrthoNormalize(FVector& Normal, FVector& Tangent)
{
	Normal = Normal.GetUnsafeNormal();
	Tangent = Tangent - (Normal * FVector::DotProduct(Tangent, Normal));
	Tangent = Tangent.GetUnsafeNormal();
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

FRotator UBodyStateAnimInstance::EstimateAutoMapRotation(FMappedBoneAnimData& ForMap, const EBodyStateAutoRigType RigTargetType)
{
	USkeletalMeshComponent* Component = GetSkelMeshComponent();
	// Get bones and parent indices
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	TArray<FName> Names;
	TArray<FNodeItem> NodeItems;
	INodeMappingProviderInterface* INodeMapping = Cast<INodeMappingProviderInterface>(SkeletalMesh);

	if (!INodeMapping)
	{
		UE_LOG(LogTemp, Log, TEXT("UBodyStateAnimInstance::EstimateAutoMapRotation INodeMapping is NULL so cannot proceed"));
		return FRotator();
	}

	INodeMapping->GetMappableNodeData(Names, NodeItems);

	bool IsFlippedModel = false;

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
	FBoneReference IndexBone = ForMap.BoneMap.Find(Index)->MeshBone;
	FBoneReference MiddleBone = ForMap.BoneMap.Find(Middle)->MeshBone;
	FBoneReference PinkyBone = ForMap.BoneMap.Find(Pinky)->MeshBone;
	FBoneReference WristBone = ForMap.BoneMap.Find(Wrist)->MeshBone;

	EBodyStateAutoRigType RigMeshType = EBodyStateAutoRigType::HAND_LEFT;

	FString IndexName(IndexBone.BoneName.ToString().ToLower());
	if (IndexName.Contains("_r") || IndexName.Contains("r_"))
	{
		RigMeshType = EBodyStateAutoRigType::HAND_RIGHT;
	}
	if (RigMeshType != RigTargetType)
	{
		IsFlippedModel = true;
	}

	int32 IndexBoneIndex = Names.Find(IndexBone.BoneName);
	int32 MiddleBoneIndex = Names.Find(MiddleBone.BoneName);
	int32 PinkyBoneIndex = Names.Find(PinkyBone.BoneName);
	int32 WristBoneIndex = Names.Find(WristBone.BoneName);

	FTransform IndexPose = NodeItems[IndexBoneIndex].Transform;
	FTransform MiddlePose = NodeItems[MiddleBoneIndex].Transform;
	FTransform PinkyPose = NodeItems[PinkyBoneIndex].Transform;
	FTransform WristPose = NodeItems[WristBoneIndex].Transform;

	// Calculate the Model's rotation
	// direct port from c# Unity version HandBinderAutoBinder.cs
	FVector Forward = MiddlePose.GetLocation() - WristPose.GetLocation();
	FVector Right = IndexPose.GetLocation() - PinkyPose.GetLocation();

	if (RigTargetType == EBodyStateAutoRigType::HAND_RIGHT)
	{
		Right = -Right;
	}
	FVector Up = FVector::CrossProduct(Forward, Right);
	// we need a three param versions of this.
	// OrthoNormalize(Forward, Up);
	OrthNormalize2(Forward, Up, Right);

	// in Unity this was Quat.LookRotation(forward,up).
	FQuat ModelRotation;
	ModelRotation = MyLookRotation(Up, Forward);
	// Calculate the difference between the Calculated hand basis and the wrist's rotation
	FQuat ModelQuat(ModelRotation);

	// In unity, this came from the wrist in the world/scene coords
	// FQuat WristPoseQuat(FRotator(0, 180, -90));
	FQuat WristPoseQuat(WristPose.GetRotation());
	FRotator WristRotation = (ModelQuat.Inverse() * WristPoseQuat).Rotator();

	// In unreal this is the reference direction for the anim system
	if (RigTargetType == EBodyStateAutoRigType::HAND_RIGHT)
	{
		if (IsFlippedModel)
		{
			WristRotation += FRotator(0, -90, 90);
			Component->SetRelativeScale3D(FVector(-1, 1, 1));
		}
		else
		{
			WristRotation += FRotator(0, 90, -90);
		}
	}
	else
	{
		if (IsFlippedModel)
		{
			WristRotation += FRotator(0, 90, -90);
			Component->SetRelativeScale3D(FVector(-1, 1, 1));
		}
		else
		{
			WristRotation += FRotator(0, -90, 90);
		}
	}
	ForMap.bShouldDeformMesh = !IsFlippedModel;
	return WristRotation;
}
void UBodyStateAnimInstance::AutoMapBoneDataForRigType(FMappedBoneAnimData& ForMap, EBodyStateAutoRigType RigTargetType)
{
	// Grab our skel mesh component
	USkeletalMeshComponent* Component = GetSkelMeshComponent();

	IndexedBoneMap = AutoDetectHandIndexedBones(Component, RigTargetType);
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
	if (bDetectHandRotationDuringAutoMapping)
	{
		FRotator WristRotation = EstimateAutoMapRotation(ForMap, RigTargetType);
		ForMap.OffsetTransform.SetRotation(FQuat(WristRotation));
	}
	// Reset specified keys from defaults
	for (auto Pair : OldMap)
	{
		Pair.Value.MeshBone.Initialize(CurrentSkeleton);
		ForMap.BoneMap.Add(Pair.Key, Pair.Value);
	}
}

int32 UBodyStateAnimInstance::TraverseLengthForIndex(int32 Index)
{
	if (Index == -1 || Index >= BoneLookupList.Bones.Num())
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

void UBodyStateAnimInstance::AddFingerToMap(EBodyStateBasicBoneType BoneType, int32 BoneIndex,
	TMap<EBodyStateBasicBoneType, FBodyStateIndexedBone>& BoneMap, int32 InBonesPerFinger /*= BonesPerFinger*/)
{
	int32 FingerRoot = (int32) BoneType;
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot), BoneLookupList.Bones[BoneIndex]);
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 1), BoneLookupList.Bones[BoneIndex + 1]);
	BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 2), BoneLookupList.Bones[BoneIndex + 2]);
	if (InBonesPerFinger > 3)
	{
		BoneMap.Add(EBodyStateBasicBoneType(FingerRoot + 3), BoneLookupList.Bones[BoneIndex + 3]);
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

void UBodyStateAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Get our default bodystate skeleton
	UBodyStateSkeleton* Skeleton = UBodyStateBPLibrary::SkeletonForDevice(this, 0);
	SetAnimSkeleton(Skeleton);

	// Try to auto-detect our bones
	if (bAutoDetectBoneMapAtInit)
	{
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

			AutoMapBoneDataForRigType(OneHandMap, AutoMapTarget);
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
			AutoMapBoneDataForRigType(LeftHandMap, EBodyStateAutoRigType::HAND_LEFT);

			FMappedBoneAnimData& RightHandMap = MappedBoneList[1];
			AutoMapBoneDataForRigType(RightHandMap, EBodyStateAutoRigType::HAND_RIGHT);
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
	}
}

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
		CachedBoneLink TraverseResult;

		TraverseResult.MeshBone = Pair.Value.MeshBone;
		TraverseResult.MeshBone.Initialize(LinkedSkeleton);
		TraverseResult.BSBone = BodyStateSkeleton->BoneForEnum(Pair.Key);

		// Costly function and we don't need it after all, and it won't work anymore now that it depends on external data
		// TraverseResult.TraverseCount = TraverseLengthForIndex(TraverseResult.MeshBone.BoneIndex);

		CachedBoneList.Add(TraverseResult);
	}

	// 2) reorder according to shortest traverse list
	CachedBoneList.Sort([](const CachedBoneLink& One, const CachedBoneLink& Two) {
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
