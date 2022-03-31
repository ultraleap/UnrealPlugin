// Fill out your copyright notice in the Description page of Project Settings.

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
	UPROPERTY()
	bool EnableLogging;

	FIEPhysicsTickNotify IEPhysicsTickNotify;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	FCalculateCustomPhysics OnCalculateCustomPhysics;

	void SubstepTick(float DeltaTime, FBodyInstance* BodyInstance);
	void DoPhysics(float DeltaTime, bool InSubstep);
};
