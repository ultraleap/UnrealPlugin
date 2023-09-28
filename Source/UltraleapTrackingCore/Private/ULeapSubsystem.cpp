// Fill out your copyright notice in the Description page of Project Settings.


#include "ULeapSubsystem.h"

void UULeapSubsystem::OnGrabCall(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight)
{
	if (GrabbedActor!=nullptr)
	{
		OnLeapGrab.Broadcast(GrabbedActor);
		OnLeapGrabNative.Execute(GrabbedActor, HandLeft, HandRight);
	}
}

void UULeapSubsystem::OnReleaseCall(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName)
{
	if (ReleasedActor!=nullptr)
	{
		OnLeapRelease.Broadcast(ReleasedActor);
		OnLeapReleaseNative.Execute(ReleasedActor, HandLeft, HandRight, BoneName);
	}
}

void UULeapSubsystem::GrabActionCall(FVector Location, FVector ForwardVec)
{
	OnLeapGrabAction.Broadcast(Location, ForwardVec);
}

void UULeapSubsystem::LeapTrackingDataCall(const FLeapFrameData& Frame)
{
	OnLeapTrackingDatanative.ExecuteIfBound(Frame);
	OnLeapFrameMulti.Broadcast(Frame);
}

void UULeapSubsystem::LeapPinchCall(const FLeapHandData& HandData)
{
	//OnLeapPinch.ExecuteIfBound(HandData);
	OnLeapPinchMulti.Broadcast(HandData);
}

void UULeapSubsystem::LeapUnPinchCall(const FLeapHandData& HandData)
{
	//OnLeapUnpinched.ExecuteIfBound(HandData);
	OnLeapUnPinchMulti.Broadcast(HandData);
}

