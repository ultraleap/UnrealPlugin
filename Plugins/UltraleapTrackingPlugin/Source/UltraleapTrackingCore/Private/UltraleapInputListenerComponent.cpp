/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "UltraleapInputListenerComponent.h"

#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
// Sets default values for this component's properties
UUltraleapInputListenerComponent::UUltraleapInputListenerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UUltraleapInputListenerComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UUltraleapInputListenerComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
APlayerController* UUltraleapInputListenerComponent::GetOwningPlayer()
{
	AActor* OwningActor = GetOwner();
	if (OwningActor)
	{
		return UGameplayStatics::GetPlayerController(OwningActor->GetWorld(), 0);
	}
	return nullptr;
}
void UUltraleapInputListenerComponent::InitializeInputComponent()
{
	if (APlayerController* Controller = GetOwningPlayer())
	{
		InputComponent = NewObject<UInputComponent>(this, UInputSettings::GetDefaultInputComponentClass(), NAME_None, RF_Transient);
		InputComponent->bBlockInput = bStopAction;
		InputComponent->Priority = Priority;
		Controller->PushInputComponent(InputComponent);
	}
}
void UUltraleapInputListenerComponent::ListenForInputAction(
	FName ActionName, TEnumAsByte<EInputEvent> EventType, bool bConsume, FOnInputActionUL Callback)
{
	if (!InputComponent)
	{
		InitializeInputComponent();
	}

	if (InputComponent)
	{
		FInputActionBinding NewBinding(ActionName, EventType.GetValue());
		NewBinding.bConsumeInput = bConsume;
		NewBinding.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &ThisClass::OnInputAction, Callback);

		InputComponent->AddActionBinding(NewBinding);
	}
}

void UUltraleapInputListenerComponent::StopListeningForInputAction(FName ActionName, TEnumAsByte<EInputEvent> EventType)
{
	if (InputComponent)
	{
		for (int32 ExistingIndex = InputComponent->GetNumActionBindings() - 1; ExistingIndex >= 0; --ExistingIndex)
		{
			const FInputActionBinding& ExistingBind = InputComponent->GetActionBinding(ExistingIndex);
			if (ExistingBind.GetActionName() == ActionName && ExistingBind.KeyEvent == EventType)
			{
				InputComponent->RemoveActionBinding(ExistingIndex);
			}
		}
	}
}
void UUltraleapInputListenerComponent::RegisterInputComponent()
{
	if (InputComponent)
	{
		if (APlayerController* Controller = GetOwningPlayer())
		{
			Controller->PushInputComponent(InputComponent);
		}
	}
}

void UUltraleapInputListenerComponent::UnregisterInputComponent()
{
	if (InputComponent)
	{
		if (APlayerController* Controller = GetOwningPlayer())
		{
			Controller->PopInputComponent(InputComponent);
		}
	}
}
void UUltraleapInputListenerComponent::OnInputAction(FOnInputActionUL Callback)
{
	Callback.ExecuteIfBound();
}