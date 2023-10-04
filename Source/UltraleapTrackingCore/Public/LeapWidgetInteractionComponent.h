// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetInteractionComponent.h"
#include "ULeapSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "LeapWidgetInteractionComponent.generated.h"

/**
 * This component will provide far field widgets with interactions 
 * Will need to add 2 components, for the left hand and the right one
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

	/** This will add an offset between the hands and the widget
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	int32 CursorDistanceFromHand;
	/** The default static mesh is a sphere, but can be changed to anything
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	UStaticMeshComponent* StaticMesh;
	/** This can be used to change the cursor's color 
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	UMaterial* MaterialBase;
	/** This can be used to change the cursor's size
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI",
		meta = (ClampMin = "0.01", ClampMax = "0.1", UIMin = "0.01", UIMax = "0.1"))
	float CursorSize;
	/** Hand type, for left and right hands
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	TEnumAsByte<EHandType> HandType;

	/** Interpolation setting delta time since last tick
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	float InterpolationDelta;
	/** Interpolation setting if 0 then no interp is applied 
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	float InterpolationSpeed;

	/** WidgetInteraction type, this requires the InteractionDistance to be <= 30 in order to change to NEAR interactions
	 *  Changing this to NEAR will enable interactions with widgets by direct touch
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI", 
		meta = (EditCondition = "InteractionDistance <= 30"))
	TEnumAsByte <EUIType> WidgetInteraction;

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

	void NearClickLeftMouse();
	void NearReleaseLeftMouse();

	void ScaleUpAndClickButton(const FKey Button = EKeys::LeftMouseButton);
	void ScaleDownAndUnClickButton(const FKey Button = EKeys::LeftMouseButton);

	bool NearlyEqualVectors(FVector A, FVector B, float ErrorTolerance = SMALL_NUMBER);

	bool LessVectors(FVector A, FVector B, float ErrorTolerance = SMALL_NUMBER);

	APawn* LeapPawn;
	AStaticMeshActor* PointerActor;
	UWorld* World;
	UMaterialInstanceDynamic* LeapDynMaterial;
	APlayerCameraManager* PlayerCameraManager;
	bool bIsPinched;

	bool bHandTouchWidget;
};
