// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LeapLiveLink.h"
#include "CoreMinimal.h"
#include "Misc/App.h"
#include "Animation/AnimInstance.h"


#include "LiveLinkProvider.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"

FLeapLiveLinkProducer::FLeapLiveLinkProducer()
{

}

void FLeapLiveLinkProducer::Startup()
{
	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(TEXT("Leap Motion Live Link"));

	TFunction<void()> StatusChangeLambda = [this]
	{
		if (LiveLinkProvider->HasConnection())
		{
			UE_LOG(LogTemp, Log, TEXT("Leap Live Link Source Connected."));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Leap Live Link Source Disconnected."));
		}
	};
	ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateLambda(StatusChangeLambda));

	SubjectName = TEXT("Leap Motion");
}

void FLeapLiveLinkProducer::ShutDown()
{
	LiveLinkProvider->UnregisterConnStatusChangedHandle(ConnectionStatusChangedHandle);
}

void FLeapLiveLinkProducer::SyncSubjectToSkeleton(const UBodyStateSkeleton* Skeleton)
{
	const TArray<UBodyStateBone*>& Bones = Skeleton->Bones;

	LiveLinkProvider->RemoveSubject(SubjectName);

	//Initialize the subject
	FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
	FLiveLinkSkeletonStaticData* AnimationStaticData = StaticData.Cast<FLiveLinkSkeletonStaticData>();

	TrackedBones.Reset();

	TArray<FName> ParentsNames;
	ParentsNames.Reserve(Bones.Num());

	for (UBodyStateBone* Bone : Bones)
	{
		if (Bone->IsTracked())
		{
			AnimationStaticData->BoneNames.Add(FName(*Bone->Name));
			ParentsNames.Add(FName(*Bone->Parent->Name));
			TrackedBones.Add(Bone);
		}
	}

	//Correct the parent index to the current live list
	for (FName ParentName : ParentsNames)
	{
		AnimationStaticData->BoneParents.Add(AnimationStaticData->BoneNames.IndexOfByKey(ParentName));
	}

	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
}

void FLeapLiveLinkProducer::UpdateFromBodyState(const UBodyStateSkeleton* Skeleton)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData* AnimationFrameData = FrameData.Cast<FLiveLinkAnimationFrameData>();

	AnimationFrameData->Transforms.Reserve(TrackedBones.Num());

	for (TWeakObjectPtr<UBodyStateBone> WeakBone : TrackedBones)
	{
		if (UBodyStateBone* Bone = WeakBone.Get())
		{
			AnimationFrameData->Transforms.Add(Bone->Transform());
		}
		else
		{
			AnimationFrameData->Transforms.Add(FTransform::Identity);
		}
	}

	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}

bool FLeapLiveLinkProducer::HasConnection()
{
	return LiveLinkProvider->HasConnection();
}
