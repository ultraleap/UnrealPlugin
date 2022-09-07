/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapLiveLink.h"

#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "LeapBlueprintFunctionLibrary.h"
#include "LiveLinkProvider.h"
#include "Misc/App.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"

FLeapLiveLinkProducer::FLeapLiveLinkProducer()
{
}

void FLeapLiveLinkProducer::Startup(const FString& DeviceSerial)
{
	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(TEXT("Ultraleap Tracking Live Link: ") + DeviceSerial);

	TFunction<void()> StatusChangeLambda = [this] {
		if (LiveLinkProvider->HasConnection())
		{
			UE_LOG(LogTemp, Log, TEXT("Leap Live Link Source Connected. "));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Leap Live Link Source Disconnected."));
		}
	};
	ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(
		FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateLambda(StatusChangeLambda));

	SubjectName = TEXT("Ultraleap Tracking");
}

void FLeapLiveLinkProducer::ShutDown()
{
	LiveLinkProvider->UnregisterConnStatusChangedHandle(ConnectionStatusChangedHandle);
}

void FLeapLiveLinkProducer::SyncSubjectToSkeleton(const UBodyStateSkeleton* Skeleton)
{
	const TArray<UBodyStateBone*>& Bones = Skeleton->Bones;

	// Create Data structures for LiveLink
	FLiveLinkStaticDataStruct StaticData(FLiveLinkSkeletonStaticData::StaticStruct());
	FLiveLinkSkeletonStaticData& AnimationData = *StaticData.Cast<FLiveLinkSkeletonStaticData>();

	TrackedBones.Reset();

	TArray<FName> ParentsNames;
	for (int i = 0; i < Bones.Num(); i++)
	{
		if (Bones[i]->IsTracked())
		{
			AnimationData.BoneNames.Add(FName(*Bones[i]->Name));
			ParentsNames.Add(FName(*Bones[i]->Parent->Name));
			TrackedBones.Add(Bones[i]);
		}
	}

	// Add bone parents
	for (int j = 0; j < ParentsNames.Num(); j++)
	{
		AnimationData.BoneParents.Add(AnimationData.BoneNames.IndexOfByKey(ParentsNames[j]));
	}

	LiveLinkProvider->UpdateSubjectStaticData(SubjectName, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticData));
}

void FLeapLiveLinkProducer::UpdateFromBodyState(const UBodyStateSkeleton* Skeleton)
{
	FLiveLinkFrameDataStruct FrameData(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData* AnimationFrameData = FrameData.Cast<FLiveLinkAnimationFrameData>();

	const TArray<UBodyStateBone*>& Bones = Skeleton->Bones;

	for (int i = 0; i < Bones.Num(); i++)
	{
		if (Bones[i]->IsTracked())
		{
			FTransform BoneTransform = Bones[i]->Transform();
			FTransform ParentTransform;

			// The live link node outputs in local space (this means each bone transform must be relative to its parent)
			// so convert from component space here
			ConvertComponentTransformToLocalTransform(BoneTransform, Bones[i]->Parent->Transform());
			AnimationFrameData->Transforms.Add(BoneTransform);
		}
	}

	LiveLinkProvider->UpdateSubjectFrameData(SubjectName, MoveTemp(FrameData));
}

bool FLeapLiveLinkProducer::HasConnection()
{
	return LiveLinkProvider->HasConnection();
}
void FLeapLiveLinkProducer::ConvertComponentTransformToLocalTransform(FTransform& BoneTransform, const FTransform& ParentTransform)
{
	BoneTransform.SetToRelativeTransform(ParentTransform);
	BoneTransform.NormalizeRotation();
}