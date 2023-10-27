/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2023.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetInteractionComponent.h"
#include "LeapSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"

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


	/** Hand type, for left and right hands
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	TEnumAsByte<EHandType> LeapHandType;

	/** WidgetInteraction type, this requires the InteractionDistance to be <= 30 in order to change to NEAR interactions
	 *  Changing this to NEAR will enable interactions with widgets by direct touch
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI" /* , meta = (EditCondition = "InteractionDistance <= 30")*/)
	TEnumAsByte<EUIType> WidgetInteraction;

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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI", meta = (ClampMin = "0.01", ClampMax = "0.1", UIMin = "0.01", UIMax = "0.1"))
	float CursorSize;

	/** This will add an offset between the hands and the widget
	 */
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	int32 CursorDistanceFromHand;

	/** Interpolation setting delta time since last tick
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	float InterpolationDelta;
	/** Interpolation setting if 0 then no interp is applied 
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	float InterpolationSpeed;	

	/** This will automatically enable near distance interactions mode when the 
	* Distance between the hand and widget is less than 30 cm 
	* and far mode when the ditance is more than 35 cm
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI")
	bool bAutoMode;

	/** The distance in cm betweenn index and UI to trigger touch interaction
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap UI | Near")
	float IndexDitanceFromUI;
	ULeapSubsystem* LeapSubsystem;

	#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif

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

	void NearClickLeftMouse(TEnumAsByte<EHandType> HandType);
	void NearReleaseLeftMouse(TEnumAsByte<EHandType> HandType);

	void ScaleUpAndClickButton(const FKey Button = EKeys::LeftMouseButton);
	void ScaleDownAndUnClickButton(const FKey Button = EKeys::LeftMouseButton);

	/**
	 * Used to switch between FAR/NEAR modes, depending on the distance of the hand 
	 * from the widget
	 * @param Dist - distance of the hand from the widget
	 */
	void HandleAutoMode(float Dist);

	APawn* LeapPawn;
	AStaticMeshActor* PointerActor;
	UWorld* World;
	UMaterialInstanceDynamic* LeapDynMaterial;
	APlayerCameraManager* PlayerCameraManager;
	bool bIsPinched;

	bool bHandTouchWidget;
	bool bAutoModeTrigger;
};
