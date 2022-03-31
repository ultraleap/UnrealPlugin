/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "IEPhysicsTickStaticMeshComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIEPhysicsTickNotify, float, DeltaTime,FBodyInstance&, BodyInstance);
/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UIEPhysicsTickStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
	UIEPhysicsTickStaticMeshComponent();

public:

	FIEPhysicsTickNotify IEPhysicsTickNotify;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	FCalculateCustomPhysics OnCalculateCustomPhysics;

	void SubstepTick(float DeltaTime, FBodyInstance* BodyInstance);
};
