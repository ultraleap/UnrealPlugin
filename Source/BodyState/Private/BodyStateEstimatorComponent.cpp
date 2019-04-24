// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BodyStateEstimatorComponent.h"
#include "IBodyState.h"
#include "Engine/World.h"

UBodyStateEstimatorComponent::UBodyStateEstimatorComponent(const FObjectInitializer &init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	MergingFunctionId = -1;
	MergingFunction = nullptr;
}

void UBodyStateEstimatorComponent::InitializeComponent()
{
	Super::InitializeComponent();

	//Only allow game world estimators
	if (!GetWorld()->IsGameWorld())
	{
		return;
	}
	
	//Wrapper function which calls the broadcast function
	WrapperMergingFunction = [&](UBodyStateSkeleton* SkeletonToUpdate, float DeltaTime)
	{
		if (MergingFunction != nullptr)
		{
			MergingFunction(SkeletonToUpdate, DeltaTime);
		}
		OnUpdateSkeletonEstimation.Broadcast(SkeletonToUpdate);
	};

	//Attach our selves as a bone scene listener. This will auto update our transforms
	MergingFunctionId = IBodyState::Get().AttachMergingFunctionForSkeleton(WrapperMergingFunction);
}

void UBodyStateEstimatorComponent::UninitializeComponent()
{
	//remove ourselves from auto updating transform delegates
	IBodyState::Get().RemoveMergingFunction(MergingFunctionId);

	Super::UninitializeComponent();
}