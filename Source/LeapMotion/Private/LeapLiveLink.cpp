// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

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

	//Create Data structures for LiveLink
	FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
	FLiveLinkSkeletonStaticData& AnimationData = *StaticData.Cast<FLiveLinkSkeletonStaticData>();

	TrackedBones.Reset();

	TArray<FName> ParentsNames;
	for(int i = 0; i < Bones.Num(); i++)
	{
		if (Bones[i]->IsTracked())
		{
			AnimationData.BoneNames.Add(FName(*Bones[i]->Name));
			ParentsNames.Add(FName(*Bones[i]->Parent->Name));
			TrackedBones.Add(Bones[i]);
		}
	}

	//Add bone parents
	for (int j = 0; j < ParentsNames.Num(); j++) {
		AnimationData.BoneParents.Add(AnimationData.BoneNames.IndexOfByKey(ParentsNames[j]));
	}

	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
}

void FLeapLiveLinkProducer::UpdateFromBodyState(const UBodyStateSkeleton* Skeleton)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData* AnimationFrameData = FrameData.Cast<FLiveLinkAnimationFrameData>();

	for (int i=0; i < TrackedBones.Num(); i++)
	{
		UBodyStateBone* Bone = TrackedBones[i];
		if (Bone->IsTracked())
		{
			AnimationFrameData->Transforms.Add(Bone->Transform());
		}
	}

	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}

bool FLeapLiveLinkProducer::HasConnection()
{
	return LiveLinkProvider->HasConnection();
}
