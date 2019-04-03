// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BodyStateSelectorComponent.generated.h"

//Disable this component in bp until v3.1 when the multi-player selection and replication functionality gets implemented
UCLASS( ClassGroup=(Custom)) //, meta=(BlueprintSpawnableComponent) 
class BODYSTATE_API UBodyStateSelectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBodyStateSelectorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
