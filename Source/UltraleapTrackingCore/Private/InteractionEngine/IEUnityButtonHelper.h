// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IEUnityButtonHelper.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIEUnityButtonHelper : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIEUnityButtonHelper();

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

	UPROPERTY()
	USceneComponent* LockedInteractingController;

	UPROPERTY()
	FVector LocalPhysicsPositionConstrained;

	/**  Logic for IE button from Unity, call on tick from BP */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void Update(
	UPARAM(Ref) bool& IgnoreGrasping, UPARAM(Ref) bool& InitialIgnoreGrasping, const bool& IsPrimaryHovered, const bool& IsGrasped,
	const bool& ControlEnabled, UPARAM(Ref) bool& IgnoreContact, UPrimitiveComponent* Rigidbody,
	const FRotator& InitialLocalRotation, const float PrimaryHoverDistance, 
	const float SpringForce, const FVector2D& MinMaxHeight, const float RestingHeight, const float WorldDelta, const FVector& InitialLocalPosition,
		UPARAM(Ref) float& PressedAmount, USceneComponent* PrimaryHoveringController, const FTransform& ParentWorldTransform);

	//UPROPERTY(BlueprintAssignable, EditAnywhere, Category = "Ultraleap IE")
	//FGrabClassifierGrabStateChanged OnIsGrabbingChanged;

	


private:
	void OnPress(){}
	void OnUnpress(){}

	FVector ConstrainDepressedLocalPosition(
		const FVector& InitialLocalPosition, const FVector& LocalPosition);

	// in Unity this handles the physics tick
	// in Unreal we can't get an independent physics tick without editing engine source
	void FixedUpdate(const bool IsGrasped, UPrimitiveComponent* Rigidbody, const FVector& InitialLocalPosition,
		const FVector2D& MinMaxHeight, const float RestingHeight, const FTransform& ParentWorldTransform);


};