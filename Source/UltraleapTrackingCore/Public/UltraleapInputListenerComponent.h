/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "UltraleapInputListenerComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnInputActionUL);

/**
 * Customisable input event listener, used to dynamically subscribe to input events from blueprint
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ULTRALEAPTRACKING_API UUltraleapInputListenerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUltraleapInputListenerComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	uint8 bStopAction : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 Priority;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Transient, DuplicateTransient)
	class UInputComponent* InputComponent;

public:
	/**
	 * Listens for a particular Player Input Action by name.  This requires that those actions are being executed, and
	 * that we're not currently in UI-Only Input Mode.
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ListenForInputAction(FName ActionName, TEnumAsByte<EInputEvent> EventType, bool bConsume, FOnInputActionUL Callback);

	/**
	 * Removes the binding for a particular action's callback.
	 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void StopListeningForInputAction(FName ActionName, TEnumAsByte<EInputEvent> EventType);

	void OnInputAction(FOnInputActionUL Callback);

	APlayerController* GetOwningPlayer();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void InitializeInputComponent();
	void RegisterInputComponent();
	void UnregisterInputComponent();
};
