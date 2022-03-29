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
	bool _pressedThisFrame;
	
	UPROPERTY()
	bool _unpressedThisFrame;
		
	UPROPERTY()
	bool _isPressed;

	UPROPERTY()
	FVector _localPhysicsPosition;

	UPROPERTY()
	USceneComponent* _lastDepressor;

	UPROPERTY()
	FVector _localDepressorPosition;

	UPROPERTY()
	FVector _physicsPosition;

	UPROPERTY()
	FVector _physicsVelocity;

	/**  Logic for IE button from Unity, call on tick from BP */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void Update(
	UPARAM(Ref) bool& ignoreGrasping, UPARAM(Ref) bool& _initialIgnoreGrasping, const bool& isPrimaryHovered, const bool& isGrasped,
	const bool& controlEnabled, UPARAM(Ref) bool& ignoreContact, UPrimitiveComponent* rigidbody,
	const FRotator& _initialLocalRotation, const float primaryHoverDistance, 
	const float _springForce, const FVector2D& minMaxHeight, const float restingHeight, const float WorldDelta, const FVector& initialLocalPosition, UPARAM(Ref) float& _pressedAmount, USceneComponent* primaryHoveringController);

	//UPROPERTY(BlueprintAssignable, EditAnywhere, Category = "Ultraleap IE")
	//FGrabClassifierGrabStateChanged OnIsGrabbingChanged;

	UPROPERTY()
	USceneComponent* _lockedInteractingController;
	
	UPROPERTY()
	FVector _localPhysicsPositionConstrained;



private:
	void OnPress(){}
	void OnUnpress(){}

	FVector constrainDepressedLocalPosition(
		const FVector& initialLocalPosition, const FVector& localPosition);


};
