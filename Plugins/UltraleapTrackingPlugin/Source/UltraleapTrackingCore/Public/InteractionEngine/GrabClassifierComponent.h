/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "GrabClassifierComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FGrabClassifierGrabStateChanged, UIEGrabClassifierComponent*, Source, bool, IsGrabbing);

USTRUCT(BlueprintType)
struct FGrabClassifierParams
{
	GENERATED_BODY()
	FGrabClassifierParams()
		: FingerStickiness(0.0f)
		, ThumbStickiness(0.04f)
		, MaximumCurl(0.65f)
		, MinimumCurl(-0.1f)
		, FingerTipRadius(1.2f)
		, ThumbTipRadius(1.7f)
		, GrabCooldown(0.2f)
		, MaximumCurlVelocity(0.0f)
		, GrabbedMaximumCurlVelocity(-0.025f)
		, MaximumDistanceFromHand(5.0f)
		, UseGrabCooldown(false)
	{
	}
	/** <summary> The amount of curl hysteresis on each finger type </summary> */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float FingerStickiness;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float ThumbStickiness;
	/** <summary> The minimum and maximum curl values fingers are allowed to "Grab" within </summary> */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float MaximumCurl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float MinimumCurl;
	/** <summary> The radius considered for intersection around the fingertips </summary> */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float FingerTipRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float ThumbTipRadius;
	/** <summary> The minimum amount of time between repeated grabs of a single object </summary> */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float GrabCooldown;
	/** <summary> The maximum rate that the fingers are extending where grabs are considered. </summary> */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float MaximumCurlVelocity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float GrabbedMaximumCurlVelocity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float MaximumDistanceFromHand;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	bool UseGrabCooldown;
};
/** Represents the state of a contact probe, filled in in blueprint from skeleton bones */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UGrabClassifierProbe : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	FVector Location;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	FVector Direction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float Curl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float PrevCurl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	bool IsInside;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	TArray<USceneComponent*> CandidateColliders;
};

/** Manages logic for Grabbing, based on the Unity Interaction Engine equivalent */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UIEGrabClassifierComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UIEGrabClassifierComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	/**  Grab classifier behavior, initialised in blueprint */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	FGrabClassifierParams Params;

	UPROPERTY(BlueprintReadOnly, Category = "Ultraleap IE")
	bool IsThisControllerGrabbing;

	UPROPERTY()
	bool PrevThisControllerGrabbing;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float CoolDownProgress;

	UPROPERTY(BlueprintReadOnly, Category = "Ultraleap IE")
	int NumInside;

	/**  called when the grab state has changed */
	UPROPERTY(BlueprintAssignable, EditAnywhere, Category = "Ultraleap IE")
	FGrabClassifierGrabStateChanged OnIsGrabbingChanged;

	/**  Logic for grabbing, called from blueprint, implemented in C++ for simplicity and parity with C# */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void UpdateClassifier(const USceneComponent* Hand, const TArray<UGrabClassifierProbe*>& Probes, const bool IgnoreTemporal,
		const bool IsLeftHand, const float DeltaTime, const bool IsGrabbed);

	/**  Force reset of the grab state, used when grab state in BP is forced on tracking lost
	 *  without this, a spurious ungrab event from the grab classifier will occur on tracking found */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void ForceReset();

private:
	/**  notify if changed */
	void NotifyControllerGrabbing();
};
