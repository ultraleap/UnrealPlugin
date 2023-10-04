// Fill out your copyright notice in the Description page of Project Settings.


#include "LeapSubsystem.h"

void ULeapSubsystem::OnGrabCall(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight)
{
	if (GrabbedActor!=nullptr)
	{
		OnLeapGrab.Broadcast(GrabbedActor);
		OnLeapGrabNative.Execute(GrabbedActor, HandLeft, HandRight);
	}
}

void ULeapSubsystem::OnReleaseCall(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName)
{
	if (ReleasedActor!=nullptr)
	{
		OnLeapRelease.Broadcast(ReleasedActor);
		OnLeapReleaseNative.Execute(ReleasedActor, HandLeft, HandRight, BoneName);
	}
}

void ULeapSubsystem::GrabActionCall(FVector Location, FVector ForwardVec)
{
	OnLeapGrabAction.Broadcast(Location, ForwardVec);
}

void ULeapSubsystem::LeapTrackingDataCall(const FLeapFrameData& Frame)
{
	OnLeapTrackingDatanative.ExecuteIfBound(Frame);
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

