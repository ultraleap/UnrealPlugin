// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Skeleton/BodyStateBone.h"
#include "BodyStateUtility.h"


UBodyStateBone::UBodyStateBone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FVector UBodyStateBone::Position()
{
	return BoneData.Transform.GetTranslation();
}


void UBodyStateBone::SetPosition(const FVector& InPosition)
{
	BoneData.Transform.SetTranslation(InPosition);
}

FRotator UBodyStateBone::Orientation()
{
	return BoneData.Transform.GetRotation().Rotator();
}


void UBodyStateBone::SetOrientation(const FRotator& InOrientation)
{
	BoneData.Transform.SetRotation(InOrientation.Quaternion());
}

FVector UBodyStateBone::Scale()
{
	return BoneData.Transform.GetScale3D();
}


FTransform UBodyStateBone::Transform()
{
	return BoneData.Transform;
}

void UBodyStateBone::SetScale(const FVector& InScale)
{
	BoneData.Transform.SetScale3D(InScale);
}


FBodyStateBoneMeta UBodyStateBone::UniqueMeta()
{
	//Is our meta unique?
	if (Meta.ParentDistinctMeta)
	{
		return Meta;
	}

	//Valid parent? go up the chain
	if (Parent != nullptr)
	{
		return Parent->UniqueMeta();
	}

	//No unique meta found
	FBodyStateBoneMeta InvalidMeta;
	InvalidMeta.ParentDistinctMeta = true;
	return InvalidMeta;
}

void UBodyStateBone::InitializeFromBoneData(const FBodyStateBoneData& InData)
{
	//Set the bone data
	BoneData = InData;

	//Re-initialize default values
	Initialize();
}


void UBodyStateBone::Initialize()
{
}


void UBodyStateBone::AddChild(UBodyStateBone* InChild)
{
	//Add child
	Children.Add(InChild);

	//Set parent link
	InChild->Parent = this;
}

bool UBodyStateBone::Enabled()
{
	return BoneData.Alpha == 1.f;
}

void UBodyStateBone::SetEnabled(bool enable)
{
	enable ? BoneData.Alpha = 1.f: BoneData.Alpha = 0.f;
}

void UBodyStateBone::ShiftBone(FVector Shift)
{
	BoneData.Transform.SetTranslation(BoneData.Transform.GetTranslation() + Shift);
}

void UBodyStateBone::ChangeBasis(const FRotator& PreBase, const FRotator& PostBase, bool AdjustVectors /*= true*/)
{
	//Adjust the orientation
	FRotator PostCombine = FBodyStateUtility::CombineRotators(Orientation(), PostBase);
	BoneData.Transform.SetRotation(FQuat(FBodyStateUtility::CombineRotators(PreBase, PostCombine)));

	//Rotate our vector/s
	if (AdjustVectors)
	{
		BoneData.Transform.SetTranslation(PostBase.RotateVector(Position()));
	}
}

bool UBodyStateBone::IsTracked()
{
	return Meta.Confidence > 0.01f;
}

void UBodyStateBone::SetTrackingConfidenceRecursively(float InConfidence)
{
	Meta.Confidence = InConfidence;

	for (auto& Child : Children)
	{
		Child->SetTrackingConfidenceRecursively(InConfidence);
	}
}
