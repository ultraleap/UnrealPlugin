// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Runtime/LiveLinkInterface/Public/LiveLinkTypes.h"
#include "Runtime/LiveLinkInterface/Public/ILiveLinkClient.h"
#include "Runtime/LiveLinkMessageBusFramework/Public/LiveLinkProvider.h"
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
	TArray<FName> SubjectBoneNames;
	TArray<int32> SubjectBoneParents;
	TArray<FTransform> SubjectBoneTransforms;
	TArray<FLiveLinkCurveElement> SubjectCurves;
	TArray<UBodyStateBone*> TrackedBones;
};