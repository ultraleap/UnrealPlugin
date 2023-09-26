// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetInteractionComponent.h"
#include "ULeapSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "LeapWidgetInteractionComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = "LeapUserInterface", meta = (BlueprintSpawnableComponent))
class ULTRALEAPTRACKING_API ULeapWidgetInteractionComponent : public UWidgetInteractionComponent
{
	GENERATED_BODY()

public:

	ULeapWidgetInteractionComponent();
	
	void DrawLeapCursor(FLeapHandData& Hand);
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	APawn* LeapPawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	int32 CursorDistanceFromHand;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	UMaterial* MaterialBase;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	UMaterialInstanceDynamic* LeapDynMaterial;

protected:

	UULeapSubsystem* LeapSubsystem;

private:


	void OnLeapTrackingData(const FLeapFrameData& Frame);
	void OnLeapPinch(const FLeapHandData& HandData);
	void OnLeapUnPinch(const FLeapHandData& HandData);

	void SpawnStaticMeshActor(const FVector& InLocation);
	void CreatStaticMeshForCursor();

	UWorld* World;
	AStaticMeshActor* PointerActor;
	
	APlayerCameraManager* PlayerCameraManager;
	
};
