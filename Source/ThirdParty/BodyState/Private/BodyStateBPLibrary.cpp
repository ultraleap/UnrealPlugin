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

#include "BodyStateBPLibrary.h"

#include "Animation/AnimInstance.h"
#include "BodyStateUtility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FBodyState.h"
#include "Runtime/Launch/Resources/Version.h"

UBodyStateBPLibrary::UBodyStateBPLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

int32 UBodyStateBPLibrary::AttachDevice(
	const FBodyStateDeviceConfig& Configuration, TScriptInterface<IBodyStateInputInterface> InputCallbackDelegate)
{
	UE_LOG(BodyStateLog, Warning, TEXT("Not implemented yet, use raw for now."));
	return -1;
}

int32 UBodyStateBPLibrary::AttachDeviceNative(
	const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate)
{
	return IBodyState::Get().AttachDevice(Configuration, InputCallbackDelegate);
}

bool UBodyStateBPLibrary::DetachDevice(int32 DeviceID)
{
	if (IBodyState::IsAvailable())
	{
		return IBodyState::Get().DetachDevice(DeviceID);
	}
	else
	{
		return false;
	}
}

UBodyStateSkeleton* UBodyStateBPLibrary::SkeletonForDevice(UObject* WorldContextObject, int32 DeviceID /*= 0*/)
{
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 16
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
#else
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
#endif
	if (World == nullptr)
	{
		UE_LOG(BodyStateLog, Warning, TEXT("SkeletonForDevice:: Wrong world context"))
		return nullptr;
	}

	if (IBodyState::IsAvailable() && (World->IsGameWorld() || World->IsPreviewWorld()))
	{
		return IBodyState::Get().SkeletonForDevice(DeviceID);
	}
	else
	{
		return nullptr;
	}
}

FTransform UBodyStateBPLibrary::TransformForBoneNamedInAnimInstance(const FName& Bone, UAnimInstance* Instance)
{
	USkeletalMeshComponent* SkeletalMesh = Instance->GetSkelMeshComponent();
	int32 BoneIndex = SkeletalMesh->GetBoneIndex(Bone);

	if (BoneIndex != INDEX_NONE)
	{
		return SkeletalMesh->GetBoneTransform(BoneIndex);
	}
	else
	{
		return FTransform();
	}
}

bool UBodyStateBPLibrary::AttachMergeAlgorithm(TFunction<void(UBodyStateSkeleton*, float)> InFunction)
{
	IBodyState::Get().AttachMergingFunctionForSkeleton(InFunction);
	return true;
}
