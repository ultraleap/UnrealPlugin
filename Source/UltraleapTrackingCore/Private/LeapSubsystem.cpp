// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapSubsystem.h"

ULeapSubsystem::ULeapSubsystem() 
	: bUseOpenXR(false)
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
	}
}

void ULeapSubsystem::OnReleaseCall(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName)
{
	if (ReleasedActor != nullptr && HandLeft != nullptr && HandRight != nullptr)
	{
		OnLeapRelease.Broadcast(ReleasedActor, HandLeft, HandRight, BoneName);
	}
}

void ULeapSubsystem::GrabActionCall(FVector Location, FVector ForwardVec)
{
	OnLeapGrabAction.Broadcast(Location, ForwardVec);
}

void ULeapSubsystem::LeapTrackingDataCall(const FLeapFrameData& Frame)
{
	//OnLeapTrackingDatanative.ExecuteIfBound(Frame);
	OnLeapFrameMulti.Broadcast(Frame);
}

void ULeapSubsystem::LeapPinchCall(const FLeapHandData& HandData)
{
	//OnLeapPinch.ExecuteIfBound(HandData);
	OnLeapPinchMulti.Broadcast(HandData);
}

void ULeapSubsystem::LeapUnPinchCall(const FLeapHandData& HandData)
{
	//OnLeapUnpinched.ExecuteIfBound(HandData);
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
