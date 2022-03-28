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
	
public:	
	/**  Logic for IE button from Unity, call on tick from BP */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void Update(UPARAM(Ref) bool& _pressedThisFrame);

	//UPROPERTY(BlueprintAssignable, EditAnywhere, Category = "Ultraleap IE")
	//FGrabClassifierGrabStateChanged OnIsGrabbingChanged;

	
		
};
