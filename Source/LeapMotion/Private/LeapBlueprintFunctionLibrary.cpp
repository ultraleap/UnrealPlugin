// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LeapBlueprintFunctionLibrary.h"
#include "ILeapMotionPlugin.h"

ULeapBlueprintFunctionLibrary::ULeapBlueprintFunctionLibrary(const class FObjectInitializer& Initializer)
	: Super(Initializer)
{

}

void ULeapBlueprintFunctionLibrary::SetLeapMode(ELeapMode InMode, ELeapTrackingFidelity InTrackingFidelity)
{
	FLeapOptions Options = ILeapMotionPlugin::Get().GetOptions();
	
	Options.Mode = InMode;
	Options.TrackingFidelity = InTrackingFidelity;

	SetLeapOptions(Options);
}

void ULeapBlueprintFunctionLibrary::SetLeapOptions(const FLeapOptions& InOptions)
{
	ILeapMotionPlugin::Get().SetOptions(InOptions);
}

void ULeapBlueprintFunctionLibrary::GetLeapOptions(FLeapOptions& OutOptions)
{
	OutOptions = ILeapMotionPlugin::Get().GetOptions();
}

void ULeapBlueprintFunctionLibrary::GetLeapStats(FLeapStats& OutStats)
{
	OutStats = ILeapMotionPlugin::Get().GetLeapStats();
}

void ULeapBlueprintFunctionLibrary::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	ILeapMotionPlugin::Get().SetLeapPolicy(Flag, Enable);
}

//Debug Functions
void ULeapBlueprintFunctionLibrary::ShutdownLeap()
{
	ILeapMotionPlugin::Get().ShutdownLeap();
}
