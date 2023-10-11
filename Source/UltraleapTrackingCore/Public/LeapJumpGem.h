// Fill out your copyright notice in the Description page of Project Settings.
/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2023.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LeapSubsystem.h"
#include "UltraleapTrackingData.h"

#include "LeapJumpGem.generated.h"

class UStaticMeshComponent;

UCLASS()
class ULTRALEAPTRACKING_API ALeapJumpGem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALeapJumpGem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void RepeatingAction();

	ULeapSubsystem* LeapSubsystem;

	FTimerHandle TimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnGrabbed(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight);
	UFUNCTION()
	void OnReleased(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName);
	UFUNCTION()
	void OnLeapTrackingData(const FLeapFrameData& Frame);

	bool IsLeftHandFacingCamera(FLeapHandData Hand);

};
