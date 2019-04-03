// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Skeleton/BodyStateSkeleton.h"
#include "BodyStateEstimatorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBodyStateSkeletonSignature, UBodyStateSkeleton*, Skeleton);

/** 
* Offers an estimation event with a skeleton which will be applied to the merged bodystate.
* A good place to experiment with derived bodystate bones
*/
UCLASS(ClassGroup = "BodyState", meta = (BlueprintSpawnableComponent))
class BODYSTATE_API UBodyStateEstimatorComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:

	/** Called when the skeleton should be updated before it propagates to all listeners */
	UPROPERTY(BlueprintAssignable, Category = "BodyState Estimation Events")
	FBodyStateSkeletonSignature OnUpdateSkeletonEstimation;

protected:

	/** Override this function inside your initialize component and it will get called correctly in the merging chain */
	TFunction<void(UBodyStateSkeleton*, float)> MergingFunction;
	int32 MergingFunctionId;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

private:
	TFunction<void(UBodyStateSkeleton*, float)> WrapperMergingFunction;
};