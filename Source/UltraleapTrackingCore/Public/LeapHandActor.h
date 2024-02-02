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
#include "Components/WidgetComponent.h"

#include "LeapHandActor.generated.h"

class UStaticMeshComponent;
//class UWidgetComponent;

UCLASS()
class ULTRALEAPTRACKING_API ALeapHandActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALeapHandActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMesh;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//UWidgetComponent* WidgetComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void RepeatingAction();

	ULeapSubsystem* LeapSubsystem;

	FTimerHandle TimerHandle;

	TArray<FLeapHandData> Hands;

	UWorld *World;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UFUNCTION()
	void OnGrabbed(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight);
	//UFUNCTION()
	void OnReleased(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName);
	//UFUNCTION()
	void OnLeapTrackingData(const FLeapFrameData& Frame);

	bool IsLeftHandFacingCamera(FLeapHandData Hand);

};
