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

	ULeapWidgetInteractionComponent(const FObjectInitializer& ObjectInitializer);

	~ULeapWidgetInteractionComponent();
	
	/**
	 * Called every fram to draw the cursor
	 * @param Hand - hand data from the api
	 */
	void DrawLeapCursor(FLeapHandData& Hand);
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializeComponent() override;

	UFUNCTION()
	void OnLeapTrackingData(const FLeapFrameData& Frame);
	UFUNCTION()
	void OnLeapPinch(const FLeapHandData& HandData);
	UFUNCTION()
	void OnLeapUnPinch(const FLeapHandData& HandData);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	int32 CursorDistanceFromHand;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	UMaterial* MaterialBase;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI",
		meta = (ClampMin = "0.01", ClampMax = "0.1", UIMin = "0.01", UIMax = "0.1"))
	float CursorSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	TEnumAsByte<EHandType> HandType;

	UULeapSubsystem* LeapSubsystem;


private:

	/**
	 * Used to spawn a mesh at a location
	 * @param InLocation - the spawn location
	 */
	void SpawnStaticMeshActor(const FVector& InLocation);

	/**
	 * Used to screat a static mesh for the cursor, same mesh will be driven by hand data
	 */
	void CreatStaticMeshForCursor();

	APawn* LeapPawn;
	AStaticMeshActor* PointerActor;
	UWorld* World;
	UMaterialInstanceDynamic* LeapDynMaterial;
	APlayerCameraManager* PlayerCameraManager;
	bool bIsPinched;

};
