/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "MultiDeviceAlignment.h"

// Sets default values for this component's properties
UMultiDeviceAlignment::UMultiDeviceAlignment()
{
	AlignmentVariance = 0.2;
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMultiDeviceAlignment::BeginPlay()
{
	Super::BeginPlay();

	UpdateTrackingDevices();
	
}


// Called every frame
void UMultiDeviceAlignment::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
void UMultiDeviceAlignment::UpdateTrackingDevices()
{

}
#if WITH_EDITOR
// Property notifications
void UMultiDeviceAlignment::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UMultiDeviceAlignment, SourceDevice))
	{
		UpdateTrackingDevices();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UMultiDeviceAlignment, TargetDevice))
	{
		UpdateTrackingDevices();
	}
	Super::PostEditChangeProperty(e);
}
#endif