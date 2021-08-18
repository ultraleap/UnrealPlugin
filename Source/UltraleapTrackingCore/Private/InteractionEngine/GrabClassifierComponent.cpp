// // Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "InteractionEngine/GrabClassifierComponent.h"

// Sets default values for this component's properties
UIEGrabClassifierComponent::UIEGrabClassifierComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UIEGrabClassifierComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UIEGrabClassifierComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UIEGrabClassifierComponent::UpdateClassifier(const USceneComponent* Hand, const TArray<UGrabClassifierProbe*>& Probes,
	const TArray<USceneComponent*>& CollidingCandidates, const bool IgnoreTemporal)
{
}