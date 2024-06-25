/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapHandFaceCamera, bool, bIsFacingCamera);


/**
 * This actor class has access to hand data and grab/pinch events 
 * This can be used for JumpGem (teleportation) with inheritance
 */
UCLASS(BlueprintType, Blueprintable)
class ULTRALEAPTRACKING_API ALeapHandActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALeapHandActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LeapHand")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LeapHand")
	FVector GrabPoseOffset;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "LeapHand")
	FVector ReleasePoseOffset;

	UPROPERTY(BlueprintAssignable, Category = "Leap Hand Events")
	FLeapHandFaceCamera OnLeapHandFaceCamera;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void RepeatingAction();

	ULeapSubsystem* LeapSubsystem;

	FTimerHandle TimerHandle;

	TArray<FLeapHandData> Hands;

	UWorld *World;

	bool bIsHandFacingCamera;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnGrabbed(AActor* GrabbedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight);

	void OnReleased(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName);

	void OnLeapTrackingData(const FLeapFrameData& Frame);

	bool IsLeftHandFacingCamera(FLeapHandData Hand);

};
