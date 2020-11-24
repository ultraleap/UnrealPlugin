// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "LeapComponent.h"
#include "ILeapMotionPlugin.h"

ULeapComponent::ULeapComponent(const FObjectInitializer &init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	bAddHmdOrigin = false;
	DeviceId = 1;	//default to first device
}

void ULeapComponent::SetShouldAddHmdOrigin(bool& bShouldAdd)
{
	//this needs to propagate to all other components with same id
}

void ULeapComponent::AreHandsVisible(bool& LeftIsVisible, bool& RightIsVisible)
{
	ILeapMotionPlugin::Get().AreHandsVisible(LeftIsVisible, RightIsVisible);
}

void ULeapComponent::GetLatestFrameData(FLeapFrameData& OutData)
{
	ILeapMotionPlugin::Get().GetLatestFrameData(OutData);
}

void ULeapComponent::InitializeComponent()
{
	Super::InitializeComponent();

	//Attach delegate references
	ILeapMotionPlugin::Get().AddEventDelegate(this);
}

void ULeapComponent::UninitializeComponent()
{
	//remove ourselves from the delegates
	ILeapMotionPlugin::Get().RemoveEventDelegate(this);

	Super::UninitializeComponent();
}