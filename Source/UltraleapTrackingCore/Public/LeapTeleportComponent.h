// Fill out your copyright notice in the Description page of Project Settings.

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
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Ultrleap teleportation")
	void SetTeleportCamera(UCameraComponent* Camera)
	{
		if (Camera)
		{
			CameraComponent = Camera;
		}
	}

	void TeleportTrace(const FVector Location, const FVector Direction);


	void OnLeapGrabAction(FVector Location, FVector ForwardVec);
	void OnLeapRelease(AActor* ReleasedActor, USkeletalMeshComponent* HandLeft, USkeletalMeshComponent* HandRight, FName BoneName);

	void StartTeleportTrace();
	void TryTeleport();
	void EndTeleportTrace();

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TeleportComponent | Ultraleap")
	float LocalTeleportLaunchSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TeleportComponent | Ultraleap")
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
