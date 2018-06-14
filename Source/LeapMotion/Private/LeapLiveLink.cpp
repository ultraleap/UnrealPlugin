#include "LeapLiveLink.h"
#include "CoreMinimal.h"
#include "LiveLinkProvider.h"

void OnConnectionStatusChanged()
{
	//refresh?
}

FLeapLiveLinkProducer::FLeapLiveLinkProducer()
{

}

void FLeapLiveLinkProducer::Startup()
{
	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(TEXT("Leap Motion Live Link"));
	ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic(&OnConnectionStatusChanged));

	SubjectName = TEXT("Leap Motion BodyState");
}

void FLeapLiveLinkProducer::ShutDown()
{
	LiveLinkProvider->UnregisterConnStatusChangedHandle(ConnectionStatusChangedHandle);
}

void FLeapLiveLinkProducer::LinkToSkeleton(const UBodyStateSkeleton* Skeleton)
{
	TArray<UBodyStateBone*>& Bones = ((UBodyStateSkeleton*)Skeleton)->Bones;

	for (UBodyStateBone* Bone : Bones)
	{
		SubjectBoneNames.Add(FName(*Bone->Name));
		SubjectBoneParents.Add(Bones.IndexOfByKey(Bone->Parent));
	}

	//Initialize the subject
	LiveLinkProvider->UpdateSubject(SubjectName, SubjectBoneNames, SubjectBoneParents);
}

void FLeapLiveLinkProducer::UpdateFromBodyState(const UBodyStateSkeleton* Skeleton)
{
	SubjectBoneTransforms.Empty();
	TArray<UBodyStateBone*>& Bones = ((UBodyStateSkeleton*)Skeleton)->Bones;

	for (UBodyStateBone* Bone : Bones)
	{
		SubjectBoneTransforms.Add(Bone->Transform());
	}

	LiveLinkProvider->UpdateSubjectFrame(SubjectName, SubjectBoneTransforms, SubjectCurves, FApp::GetCurrentTime());
}