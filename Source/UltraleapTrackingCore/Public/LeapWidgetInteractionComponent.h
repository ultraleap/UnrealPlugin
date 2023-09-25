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
	
	void DrawLeapCircles(FLeapHandData& Hand);
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	APawn* LeapPawn = nullptr;


protected:

	UULeapSubsystem* LeapSubsystem;

private:


	void OnLeapTrackingData(const FLeapFrameData& Frame);
	void SpawnStaticMeshActor(const FVector& InLocation);
	void CreatStaticMeshForCursor();

	UWorld* World = nullptr;
	AStaticMeshActor* PointerActor = nullptr;
	UStaticMeshComponent* StaticMesh = nullptr;
	
};
