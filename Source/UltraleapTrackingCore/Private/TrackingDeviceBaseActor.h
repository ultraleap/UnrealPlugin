#pragma once
/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/
#include "CoreMinimal.h"
#include "UltraleapTickInEditorBaseActor.h"
#include "TrackingDeviceBaseActor.generated.h"

/**
 * 
 */
UCLASS()
class ATrackingDeviceBaseActor : public AUltraleapTickInEditorBaseActor
{
	GENERATED_BODY()

public:
	ATrackingDeviceBaseActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Leap Devices")
	class ULeapComponent* LeapComponent;

};
