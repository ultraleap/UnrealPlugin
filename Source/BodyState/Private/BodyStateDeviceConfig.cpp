// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "BodyStateDeviceConfig.h"

FBodyStateDeviceConfig::FBodyStateDeviceConfig() :
	InputType(INERTIAL_INPUT_TYPE)
{
	DeviceName = FString(TEXT("Unknown"));
}
