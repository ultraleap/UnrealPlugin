// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BodyStateBPLibrary.h"
#include "BodyStateUtility.h"
#include "FBodyState.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

UBodyStateBPLibrary::UBodyStateBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

int32 UBodyStateBPLibrary::AttachDevice(const FBodyStateDeviceConfig& Configuration, TScriptInterface<IBodyStateInputInterface> InputCallbackDelegate)
{
	UE_LOG(BodyStateLog, Warning, TEXT("Not implemented yet, use raw for now."));
	return -1;
}


int32 UBodyStateBPLibrary::AttachDeviceNative(const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate)
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

	if (IBodyState::IsAvailable() && 
		(World->IsGameWorld() || World->IsPreviewWorld()))
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

bool UBodyStateBPLibrary::AttachMergeAlgorithm(TFunction< void(UBodyStateSkeleton*, float)> InFunction)
{
	IBodyState::Get().AttachMergingFunctionForSkeleton(InFunction);
	return true;
}

