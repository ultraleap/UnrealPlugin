// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LeapLiveLink.h"
#include "CoreMinimal.h"
#include "Misc/App.h"
#include "Animation/AnimInstance.h"
#include "LiveLinkProvider.h"

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
	TArray<UBodyStateBone*>& Bones = ((UBodyStateSkeleton*)Skeleton)->Bones;

	SubjectBoneTransforms.Empty();
	SubjectBoneNames.Empty();
	SubjectBoneParents.Empty();
	TrackedBones.Empty();
	
	TArray<FName> ParentsNames;

	for (UBodyStateBone* Bone : Bones)
	{
		if (Bone->IsTracked())
		{
			SubjectBoneNames.Add(FName(*Bone->Name));
			ParentsNames.Add(FName(*Bone->Parent->Name));
			SubjectBoneTransforms.Add(Bone->Transform());
			TrackedBones.Add(Bone);
		}
	}

	//Correct the parent index to the current live list
	for (int i=0; i<ParentsNames.Num(); i++)
	{
		SubjectBoneParents.Add(SubjectBoneNames.IndexOfByKey(ParentsNames[i]));
	}

	//Initialize the subject
	LiveLinkProvider->UpdateSubject(SubjectName, SubjectBoneNames, SubjectBoneParents);
}

void FLeapLiveLinkProducer::UpdateFromBodyState(const UBodyStateSkeleton* Skeleton)
{
	//TArray<UBodyStateBone*>& Bones = ((UBodyStateSkeleton*)Skeleton)->Bones;

	//older iterator for efficiency
	for (int i=0; i < TrackedBones.Num(); i++)
	{
		UBodyStateBone* Bone = TrackedBones[i];
		if (Bone->IsTracked())
		{
			SubjectBoneTransforms[i] = Bone->Transform();
		}
	}

	LiveLinkProvider->UpdateSubjectFrame(SubjectName, SubjectBoneTransforms, SubjectCurves, FApp::GetCurrentTime());
}

bool FLeapLiveLinkProducer::HasConnection()
{
	return LiveLinkProvider->HasConnection();
}
