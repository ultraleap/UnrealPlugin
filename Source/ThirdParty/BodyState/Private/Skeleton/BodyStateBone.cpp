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

#include "Skeleton/BodyStateBone.h"

#include "BodyStateUtility.h"

UBodyStateBone::UBodyStateBone(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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
	// Is our meta unique?
	if (Meta.ParentDistinctMeta)
	{
		return Meta;
	}

	// Valid parent? go up the chain
	if (Parent != nullptr)
	{
		return Parent->UniqueMeta();
	}

	// No unique meta found
	FBodyStateBoneMeta InvalidMeta;
	InvalidMeta.ParentDistinctMeta = true;
	return InvalidMeta;
}

void UBodyStateBone::InitializeFromBoneData(const FBodyStateBoneData& InData)
{
	// Set the bone data
	BoneData = InData;

	// Re-initialize default values
	Initialize();
}

void UBodyStateBone::Initialize()
{
}

void UBodyStateBone::AddChild(UBodyStateBone* InChild)
{
	// Add child
	Children.Add(InChild);

	// Set parent link
	InChild->Parent = this;
}

bool UBodyStateBone::Enabled()
{
	return BoneData.Alpha == 1.f;
}

void UBodyStateBone::SetEnabled(bool enable)
{
	enable ? BoneData.Alpha = 1.f : BoneData.Alpha = 0.f;
}

void UBodyStateBone::ShiftBone(FVector Shift)
{
	BoneData.Transform.SetTranslation(BoneData.Transform.GetTranslation() + Shift);
}

void UBodyStateBone::ChangeBasis(const FRotator& PreBase, const FRotator& PostBase, bool AdjustVectors /*= true*/)
{
	// Adjust the orientation
	FRotator PostCombine = FBodyStateUtility::CombineRotators(Orientation(), PostBase);
	BoneData.Transform.SetRotation(FQuat(FBodyStateUtility::CombineRotators(PreBase, PostCombine)));

	// Rotate our vector/s
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
