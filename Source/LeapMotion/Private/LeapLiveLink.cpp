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

	SubjectBoneTransforms.Empty();
	SubjectBoneNames.Empty();
	SubjectBoneParents.Empty();

	for (UBodyStateBone* Bone : Bones)
	{
		SubjectBoneNames.Add(FName(*Bone->Name));
		SubjectBoneParents.Add(Bones.IndexOfByKey(Bone->Parent));
		SubjectBoneTransforms.Add(Bone->Transform());
	}

	//Initialize the subject
	LiveLinkProvider->UpdateSubject(SubjectName, SubjectBoneNames, SubjectBoneParents);
}

void FLeapLiveLinkProducer::UpdateFromBodyState(const UBodyStateSkeleton* Skeleton)
{
	TArray<UBodyStateBone*>& Bones = ((UBodyStateSkeleton*)Skeleton)->Bones;

	//older iterator for efficiency
	for (int i=0; i < Bones.Num(); i++)
	{
		UBodyStateBone* Bone = Bones[i];
		if (Bone->IsTracked())
		{
			SubjectBoneTransforms[i] = Bone->Transform();
		}
	}

	LiveLinkProvider->UpdateSubjectFrame(SubjectName, SubjectBoneTransforms, SubjectCurves, FApp::GetCurrentTime());
}