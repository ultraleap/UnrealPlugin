#include "LeapLiveLink.h"
#include "CoreMinimal.h"
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

bool FLeapLiveLinkProducer::HasConnection()
{
	return LiveLinkProvider->HasConnection();
}
