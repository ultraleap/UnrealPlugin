// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BodyStateBoneComponent.h"
#include "IBodyState.h"

UBodyStateBoneComponent::UBodyStateBoneComponent(const FObjectInitializer &init) : USceneComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	SkeletonId = 0;
	BoneToFollow = EBodyStateBasicBoneType::BONE_ROOT;
}

void UBodyStateBoneComponent::InitializeComponent()
{
	Super::InitializeComponent();

	//Attach our selves as a bone scene listener. This will auto update our transforms
	IBodyState::Get().AddBoneSceneListener(this);
}

void UBodyStateBoneComponent::UninitializeComponent()
{
	//remove ourselves from auto updating transform delegates
	IBodyState::Get().RemoveBoneSceneListener(this);

	Super::UninitializeComponent();
}