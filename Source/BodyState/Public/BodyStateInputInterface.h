// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BodyStateInputInterface.generated.h"

//For non-uobjects
class BODYSTATE_API IBodyStateInputRawInterface
{
public:
	virtual void UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton) = 0;
	virtual void OnDeviceDetach() = 0;
};

//for uobject and bps
UINTERFACE(Blueprintable, MinimalAPI)
class UBodyStateInputInterface : public UInterface
{
	GENERATED_BODY()
};

class BODYSTATE_API IBodyStateInputInterface
{
	GENERATED_BODY()

public:

	/* Requests your device to update the skeleton. You can use this BS polling method or push updates to your skeleton*/
	UFUNCTION(BlueprintNativeEvent, Category = "Body State Poll Interface")
	void UpdateInput(int32 DeviceID, class UBodyStateSkeleton* Skeleton);	//todo: define
};
