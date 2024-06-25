/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "LeapSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

#include "LeapTeleportComponent.generated.h"

/**
 * This Actor Component can be used for teleportation, on begin play
 * the camera needs to be set to the vr pawn camera
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ULTRALEAPTRACKING_API ULeapTeleportComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULeapTeleportComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable, Category = "Ultrleap teleportation")
	void SetTeleportCamera(UCameraComponent* Camera)
	{
		if (Camera)
		{
			CameraComponent = Camera;
		}
	}

	void OnLeapGrabAction(FVector Location, FVector ForwardVec);
	void OnLeapRelease(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName);
	/** Used to start teleportaton trace, needs to be called once everytime teleportation 
	* is about to start, it will initialise the Niagara Particle Systems
	*/
	UFUNCTION(BlueprintCallable, Category = "Ultrleap TeleportComponent")
	void StartTeleportTrace();
	/** Used to update the teleportation trace on tick, requires real time updated 
	* Start teleportation location and the Direction of the teleport trace
	*/
	UFUNCTION(BlueprintCallable, Category = "Ultrleap TeleportComponent")
	void TeleportTrace(const FVector Location, const FVector Direction);
	/** Used to teleport in case a valid teleportation location is found
	 */
	UFUNCTION(BlueprintCallable, Category = "Ultrleap TeleportComponent")
	void TryTeleport();
	/** Used to end teleport trace, it will destroy the Niagara Particle Systems
	 */
	UFUNCTION(BlueprintCallable, Category = "Ultrleap TeleportComponent")
	void EndTeleportTrace();

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap | TeleportComponent")
	float LocalTeleportLaunchSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap | TeleportComponent")
	UCameraComponent* CameraComponent;

	AActor* TeleportVisualizerReference;

private:
	bool IsValidTeleportLocation(FHitResult OutHit, FNavLocation& OutLocation);

private: 

	UNiagaraSystem* LeapTeleportTraceNS;
	UNiagaraComponent *TeleportTraceNSComponent;
	bool bValidTeleportationLocation;
	bool bTeleportTraceActive;
	FVector TeleportProjectPointToNavigationQueryExtent;
	FVector ProjectedTeleportLocation;
	TArray<FVector> TeleportTracePathPositions;
	AActor* Owner;
	UWorld* WorldContextObject;
	ULeapSubsystem* LeapSubsystem;
	UNavigationSystemV1* NavSys;
	bool bTeleportOnce;
};
