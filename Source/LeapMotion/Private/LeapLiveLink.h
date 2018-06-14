#pragma once

#include "Runtime/LiveLinkInterface/Public/LiveLinkTypes.h"
#include "Runtime/LiveLinkInterface/Public/ILiveLinkClient.h"
#include "Runtime/LiveLinkMessageBusFramework/Public/LiveLinkProvider.h"
#include "BodyStateSkeleton.h"

class FLeapLiveLinkProducer
{
public:
	FLeapLiveLinkProducer();
	void Startup();
	void ShutDown();

	void LinkToSkeleton(const UBodyStateSkeleton* Skeleton);
	void UpdateFromBodyState(const UBodyStateSkeleton* Skeleton);

protected:
	FDelegateHandle ConnectionStatusChangedHandle;
	TSharedPtr<ILiveLinkProvider> LiveLinkProvider;
	FName SubjectName;
	TArray<FName> SubjectBoneNames;
	TArray<int32> SubjectBoneParents;
	TArray<FTransform> SubjectBoneTransforms;
	TArray<FLiveLinkCurveElement> SubjectCurves;
};