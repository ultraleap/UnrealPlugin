/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FKabschSolver.h"
#include "TrackingDeviceBaseActor.h"
#include "MultiDeviceAlignment.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UMultiDeviceAlignment : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMultiDeviceAlignment();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap Devices")
	ATrackingDeviceBaseActor* SourceDevice;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap Devices")
	ATrackingDeviceBaseActor* TargetDevice;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap Devices")
	float AlignmentVariance;

	

#if WITH_EDITOR
	// property change handlers
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void UpdateTrackingDevices();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FKabschSolver Solver;

private:

	bool PositioningComplete = false;

	void ReAlignProvider();
	void Update();

};
