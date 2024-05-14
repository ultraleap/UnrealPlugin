/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2024.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "LeapSubsystem.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

#include "LeapWidgetInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapRayComponentVisible, bool, Visible);

/**
 * This component will provide far field widgets with interactions
 * Will need to add 2 components, one for the left hand and one for the right one
 * For further information check our docs: https://docs.ultraleap.com/xr-and-tabletop/xr/unreal/plugin/features/UI-input-modules.html
 */
UCLASS(ClassGroup = "LeapUserInterface", meta = (BlueprintSpawnableComponent))
class ULTRALEAPTRACKING_API ULeapWidgetInteractionComponent : public UWidgetInteractionComponent
{
	GENERATED_BODY()

public:
	ULeapWidgetInteractionComponent();

	~ULeapWidgetInteractionComponent();

	/**
	 * Called every frame to draw the cursor
	 * @param Hand - hand data from the api
	 */
	void DrawLeapCursor(FLeapHandData& Hand);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializeComponent() override;

	void OnLeapTrackingData(const FLeapFrameData& Frame);

	void HandleVisibilityChange(const FLeapFrameData& Frame);

	void OnLeapPinch(const FLeapHandData& HandData);

	void OnLeapUnPinch(const FLeapHandData& HandData);

	/** Hand chirality, for left and right hands
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI")
	TEnumAsByte<EHandType> LeapHandType;

	/** WidgetInteraction type, this requires the InteractionDistance to be <= 30 in order to change to NEAR interactions
	 *  Changing this to NEAR will enable interactions with widgets by direct touch
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI")
	TEnumAsByte<EUIInteractionType> WidgetInteraction;

	/** The default static mesh is a sphere, but can be changed to anything
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI")
	UStaticMeshComponent* CursorStaticMesh;

	/** This can be used to change the cursor's color
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI")
	UMaterial* MaterialBase;

	/** This can be used to change the cursor's size
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI", meta = (ClampMin = "0.01", ClampMax = "0.1", UIMin =
	 "0.01", UIMax = "0.1"))
	float CursorSize;

	/** This will automatically enable near distance interactions mode when the
	 * Distance between the hand and widget is less than 40 cm
	 * and far mode when the distance is more than 45 cm
	 */
	 UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI")
	bool bAutoMode;

	/** The distance in cm betweenn index and UI to trigger touch interaction
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI Near")
	float IndexDistanceFromUI;

	UPROPERTY(BlueprintReadOnly, Category = "UltraLeap UI" )
	bool HandVisibility;

	ULeapSubsystem* LeapSubsystem;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/** Controls how much rotation wrist movment adds to the ray
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI Far",
	 meta = (ClampMin = "0", ClampMax = "5", UIMin = "0", UIMax = "5"))
	float WristRotationFactor;
	/** Interpolation param, lower values will reduce jitter but add lag
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI Far")
	float InterpolationSpeed;
	/** For counter-acting the camera rotation to stabilize the rays
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI Far")
	float YAxisCalibOffset;
	/** For counter-acting the camera rotation to stabilize the rays
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UltraLeap UI Far")
	float ZAxisCalibOffset;
	/** The threshold used to automatically switch between far/near interactions
	 */
	UPROPERTY(BlueprintReadOnly, Category = "UltraLeap UI")
	float ModeChangeThreshold;


	/** Event on rays visibility changed
	 */
	UPROPERTY(BlueprintAssignable, EditAnywhere, Category = "UltraLeap UI")
	FLeapRayComponentVisible OnRayComponentVisible;
	/** Gets the rays direction 
	* @param TmpHand - Hand data 
	* @param Position - will return the position updated with the relative neck offset
	 */
	FVector GetHandRayDirection(FLeapHandData& TmpHand, FVector& Position);
	/** Estimates the relative neck offset
	 */
	FVector GetNeckOffset();

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

	void ScaleUpCursorAndClickButton(const FKey Button = EKeys::LeftMouseButton);
	void ScaleDownCursorAndUnclickButton(const FKey Button = EKeys::LeftMouseButton);

	/**
	 * Used to switch between FAR/NEAR modes, depending on the distance of the hand
	 * from the widget
	 * @param Dist - distance of the hand from the widget
	 */
	void HandleDistanceChange(float Dist, float MinDistance = 20.0f);
	void CleanUpEvents();
	void InitCalibrationArrays();
	void ResetCursorScale();

	/**
	 * Used to check if a widget actor has no tag "UltraleapUMG" then 
	 * disbales interactions on it
	 */
	void HandleWidgetChange();

	APawn* LeapPawn;
	AStaticMeshActor* PointerActor;
	UWorld* World;
	UMaterialInstanceDynamic* LeapDynMaterial;
	APlayerCameraManager* PlayerCameraManager;
	bool bIsPinched;

	bool bHandTouchWidget;
	bool bAutoModeTrigger;

	// Params used to compute the neck offset
	TArray<FRotator> CalibratedHeadRot;
	TArray<FVector> CalibratedHeadPos;
	// Max rotation when head is rolling or pitching
	float AxisRotOffset;
	// Adding this to the distance before we exit near field interactions
	float TriggerFarOffset;
	float FingerJointEstimatedLen;
	float ShoulderWidth;
	float PinchOffsetX;
	float PinchOffsetY;

	bool bHidden;

};
