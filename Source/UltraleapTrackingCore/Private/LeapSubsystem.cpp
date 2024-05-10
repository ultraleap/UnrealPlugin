/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapSubsystem.h"

#include "Runtime/Launch/Resources/Version.h"

#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4)
#include "Engine/Engine.h"
#endif


ULeapSubsystem::ULeapSubsystem() 
	: bUseOpenXR(false), bUseDeviceOrigin(false), LeapPawn(nullptr)
{
}

ULeapSubsystem* ULeapSubsystem::Get()
{
	return GEngine->GetEngineSubsystem<ULeapSubsystem>();
}

void ULeapSubsystem::OnGrabCall(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight)
{
	if (GrabbedActor != nullptr && HandLeft != nullptr && HandRight != nullptr)
	{
		OnLeapGrab.Broadcast(GrabbedActor, HandLeft, HandRight);
		OnLeapGrabNative.Broadcast(GrabbedActor, HandLeft, HandRight);
	}
}

void ULeapSubsystem::OnReleaseCall(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName)
{
	if (ReleasedActor != nullptr && HandLeft != nullptr && HandRight != nullptr)
	{
		OnLeapRelease.Broadcast(ReleasedActor, HandLeft, HandRight, BoneName);
		OnLeapReleaseNative.Broadcast(ReleasedActor, HandLeft, HandRight, BoneName);
	}
}

void ULeapSubsystem::GrabActionCall(FVector Location, FVector ForwardVec)
{
	OnLeapGrabAction.Broadcast(Location, ForwardVec);
	OnLeapGrabActionNative.Broadcast(Location, ForwardVec);
}

void ULeapSubsystem::LeapTrackingDataCall(const FLeapFrameData& Frame)
{
	if (!IsInGameThread())
	{
		return;
	}
	FLeapFrameData TmpFrame = Frame;
	if (LeapPawn!=nullptr && bUseDeviceOrigin)
	{
		FVector PawnLocation = LeapPawn->GetActorLocation();
		FRotator PawnRot = LeapPawn->GetActorRotation();
		TmpFrame.RotateFrame(PawnRot);
		TmpFrame.TranslateFrame(PawnLocation);
	}
	OnLeapFrameMulti.Broadcast(TmpFrame);
}

void ULeapSubsystem::LeapPinchCall(const FLeapHandData& HandData)
{
	OnLeapPinchMulti.Broadcast(HandData);
}

void ULeapSubsystem::LeapUnPinchCall(const FLeapHandData& HandData)
{
	OnLeapUnPinchMulti.Broadcast(HandData);
}

bool ULeapSubsystem::GetUseOpenXR()
{
	return bUseOpenXR;
}

void ULeapSubsystem::SetUseOpenXR(bool UseXR)
{
	bUseOpenXR = UseXR;
}
