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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIEPostPhysicsTickNotify, float, DeltaTime);

USTRUCT()
struct FIESecondaryTickFunction : public FTickFunction
{
	GENERATED_USTRUCT_BODY()

	class UIEPhysicsTickStaticMeshComponent* Target;

	ULTRALEAPTRACKING_API virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread,
		const FGraphEventRef& MyCompletionGraphEvent) override;
};

template <>
struct TStructOpsTypeTraits<FIESecondaryTickFunction> : public TStructOpsTypeTraitsBase2<FIESecondaryTickFunction>
{
	enum
	{
		WithCopy = false
	};
};

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
	FIEPostPhysicsTickNotify IEPostPhysicsTickNotify;

	virtual void BeginPlay() override;
	// pre physics tick
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tick")
	FIESecondaryTickFunction SecondaryComponentTick;

private:
	FCalculateCustomPhysics OnCalculateCustomPhysics;

	void SubstepTick(float DeltaTime, FBodyInstance* BodyInstance);
};
