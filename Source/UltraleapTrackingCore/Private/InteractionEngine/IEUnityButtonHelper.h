/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionEngine/IEPhysicsTickStaticMeshComponent.h"
#include "IEUnityButtonHelper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIEButtonStateChanged, UIEUnityButtonHelper*, Source, bool, IsPressed);

USTRUCT()
struct FIEUnityButtonState
{
	GENERATED_USTRUCT_BODY()

	// used in fixed update
	bool IsGrasped;

	UPROPERTY()
	UPrimitiveComponent* Rigidbody;

	FVector InitialLocalPosition;
	FVector2D MinMaxHeight;
	float RestingHeight;
	FTransform ParentWorldTransform;

	// used on post physics tick
	bool* IgnoreGrasping;
	bool InitialIgnoreGrasping;
	bool IsPrimaryHovered;
	bool* IgnoreContact;
		
	FRotator InitialLocalRotation;
	float PrimaryHoverDistance;
	float SpringForce;
	
	float* PressedAmount;

	UPROPERTY()
	USceneComponent* PrimaryHoveringController;

	FVector ContactPoint;

	FIEUnityButtonState()
	{
		IsGrasped = false;
		Rigidbody = nullptr;

		InitialLocalPosition = FVector::ZeroVector;
		MinMaxHeight = FVector2D::ZeroVector;
		RestingHeight = 0.5f;
		ParentWorldTransform = FTransform::Identity;

		IgnoreGrasping = nullptr;
		InitialIgnoreGrasping = nullptr;
		IsPrimaryHovered = false;
		IgnoreContact = nullptr;

		InitialLocalRotation = FRotator::ZeroRotator;
		PrimaryHoverDistance = 0.0f;
		SpringForce = 0.0f;

		PressedAmount = nullptr;
		PrimaryHoveringController = nullptr;
		ContactPoint = FVector::ZeroVector;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIEUnityButtonHelper : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIEUnityButtonHelper();


	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);



protected:
	UPROPERTY()
	bool PressedThisFrame;
	
	UPROPERTY()
	bool UnpressedThisFrame;
		
	UPROPERTY()
	bool IsPressed;

	UPROPERTY()
	FVector LocalPhysicsPosition;

	UPROPERTY()
	USceneComponent* LastDepressor;

	UPROPERTY()
	FVector LocalDepressorPosition;

	UPROPERTY()
	FVector PhysicsPosition;

	UPROPERTY()
	FVector PhysicsVelocity;

	FThreadSafeBool FixedUpdateCalled;

	UPROPERTY()
	USceneComponent* LockedInteractingController;

	UPROPERTY()
	FVector LocalPhysicsPositionConstrained;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float FrictionCoefficient;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float DragCoefficient;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	bool SweepOnMove;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	FVector AdditionalDelta;

	UPROPERTY()
	bool UseSeparateTick;

	UPROPERTY()
	bool UsePhysicsCallback;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	float InterpSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ultraleap IE")
	bool InterpFinalLocation;
	
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void SetPhysicsTickablePrimitive(UIEPhysicsTickStaticMeshComponent* Primitive);


	/**  Logic for IE button from Unity, call on tick from BP */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void Update(
	UPARAM(Ref) bool& IgnoreGrasping, UPARAM(Ref) bool& InitialIgnoreGrasping, const bool& IsPrimaryHovered, const bool& IsGrasped,
	UPARAM(Ref) bool& IgnoreContact, UPrimitiveComponent* Rigidbody,
	const FRotator& InitialLocalRotation, const float PrimaryHoverDistance, 
	const float SpringForce, const FVector2D& MinMaxHeight, const float RestingHeight, const float WorldDelta, const FVector& InitialLocalPosition,
		UPARAM(Ref) float& PressedAmount, USceneComponent* PrimaryHoveringController, const FTransform& ParentWorldTransform,
		const FVector& ContactPoint, const bool SetStateOnly);

	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void UpdateState(UPARAM(Ref) bool& IgnoreGrasping, const bool& InitialIgnoreGrasping, const bool& IsPrimaryHovered,
		const bool& IsGrasped, UPARAM(Ref) bool& IgnoreContact, UPrimitiveComponent* Rigidbody,
		const FRotator& InitialLocalRotation, const float PrimaryHoverDistance, const float SpringForce,
		const FVector2D& MinMaxHeight, const float RestingHeight, const FVector& InitialLocalPosition,
		UPARAM(Ref) float& PressedAmount, USceneComponent* PrimaryHoveringController, const FTransform& ParentWorldTransform,
		const FVector& ContactPoint);


	UPROPERTY(BlueprintAssignable, EditAnywhere, Category = "Ultraleap IE")
	FIEButtonStateChanged OnButtonStateChanged;

private:
	void OnPress()
	{
		OnButtonStateChanged.Broadcast(this, true);
	}
	void OnUnpress()
	{
		OnButtonStateChanged.Broadcast(this, false);
	}
	FIEUnityButtonState State;

	FVector ConstrainDepressedLocalPosition(
		const FVector& InitialLocalPosition, const FVector& LocalPosition);

	// in Unity this handles the physics tick
	// in Unreal we can't get an independent physics tick without editing engine source
	void FixedUpdate(const float DeltaSeconds);

	void SetRelativeLocationAsWorld(USceneComponent* Rigidbody, const FVector& RelativeLocation, const FTransform& WorldTransform);
	void SetRelativeRotationAsWorld(USceneComponent* Rigidbody, const FRotator& RelativeRotation, const FTransform& WorldTransform);


	

	FCalculateCustomPhysics OnCalculateCustomPhysics;

	void SubstepTick(float DeltaTime, FBodyInstance* BodyInstance);
	void DoPhysics(float DeltaTime, bool InSubstep);

	UPROPERTY()
	UIEPhysicsTickStaticMeshComponent* PhysicsTickablePrimitive;

	UFUNCTION()
	void OnIEPhysicsNotify(float DeltaTime, FBodyInstance& BodyInstance);
	
	UFUNCTION()
	void OnIEPostPhysicsNotify(float DeltaTime);
};
