// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LiveLinkTypes.h"
#include "ILiveLinkClient.h"
#include "LiveLinkProvider.h"
#include "Skeleton/BodyStateSkeleton.h"

class FLeapLiveLinkProducer
{
public:
	FLeapLiveLinkProducer();
	void Startup();
	void ShutDown();

	//Linkup initial information of the skeleton
	void SyncSubjectToSkeleton(const UBodyStateSkeleton* Skeleton);

	//Update transforms from bodystate skeleton data
	void UpdateFromBodyState(const UBodyStateSkeleton* Skeleton);

	//Whether it's connected as a live link source, use this to determine if we should pay the live link data cost
	bool HasConnection();

protected:
	FDelegateHandle ConnectionStatusChangedHandle;
	TSharedPtr<ILiveLinkProvider> LiveLinkProvider;
	FName SubjectName;
	TArray<UBodyStateBone*> TrackedBones;
};