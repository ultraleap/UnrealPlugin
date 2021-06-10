// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LeapBlueprintFunctionLibrary.h"

#include "IUltraleapTrackingPlugin.h"

ULeapBlueprintFunctionLibrary::ULeapBlueprintFunctionLibrary(const class FObjectInitializer& Initializer) : Super(Initializer)
{
}

void ULeapBlueprintFunctionLibrary::SetLeapMode(ELeapMode InMode, ELeapTrackingFidelity InTrackingFidelity)
{
	FLeapOptions Options = IUltraleapTrackingPlugin::Get().GetOptions();

	Options.Mode = InMode;
	Options.TrackingFidelity = InTrackingFidelity;

	SetLeapOptions(Options);
}

void ULeapBlueprintFunctionLibrary::SetLeapOptions(const FLeapOptions& InOptions)
{
	IUltraleapTrackingPlugin::Get().SetOptions(InOptions);
}

void ULeapBlueprintFunctionLibrary::GetLeapOptions(FLeapOptions& OutOptions)
{
	OutOptions = IUltraleapTrackingPlugin::Get().GetOptions();
}

void ULeapBlueprintFunctionLibrary::GetLeapStats(FLeapStats& OutStats)
{
	OutStats = IUltraleapTrackingPlugin::Get().GetLeapStats();
}

void ULeapBlueprintFunctionLibrary::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	IUltraleapTrackingPlugin::Get().SetLeapPolicy(Flag, Enable);
}
void ULeapBlueprintFunctionLibrary::GetAttachedLeapDevices(TArray<FString>& Devices)
{
	IUltraleapTrackingPlugin::Get().GetAttachedDevices(Devices);
}

// Debug Functions
void ULeapBlueprintFunctionLibrary::ShutdownLeap()
{
	IUltraleapTrackingPlugin::Get().ShutdownLeap();
}
