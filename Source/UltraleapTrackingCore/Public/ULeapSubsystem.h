// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "ULeapSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapGrab, AActor*, GrabbedActor);
DECLARE_DELEGATE_ThreeParams(FLeapGrabNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapRelease, AActor*, ReleasedActor);
DECLARE_DELEGATE_FourParams(FLeapReleaseNative, AActor*, USkeletalMeshComponent*, USkeletalMeshComponent*, FName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLeapGrabAction, FVector, Location, FVector, ForwardVec);

/**
 * 
 */
UCLASS()
class ULTRALEAPTRACKING_API UULeapSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	/** Called when grabbing is called*/
	UPROPERTY(BlueprintAssignable, Category = "Leap grab Events")
	FLeapGrab OnLeapGrab;

	
	/** Called when release is called*/
	UPROPERTY(BlueprintAssignable, Category = "Leap release Events")
	FLeapRelease OnLeapRelease;

	UPROPERTY(BlueprintAssignable, Category = "Leap release Events")
	FLeapGrabAction OnLeapGrabAction;


	FLeapGrabNative OnLeapGrabNative;
	FLeapReleaseNative OnLeapReleaseNative;

	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void OnGrabCall(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight);


	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void OnReleaseCall(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName);


	UFUNCTION(BlueprintCallable, Category = "Leap grab Functions")
	void GrabActionCall(FVector Location, FVector ForwardVec);

	
};
