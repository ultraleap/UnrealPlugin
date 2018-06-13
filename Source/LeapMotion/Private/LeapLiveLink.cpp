#include "LeapLiveLink.h"
#include "CoreMinimal.h"
#include "LiveLinkProvider.h"

void OnConnectionStatusChanged()
{
	//refresh?
}

void FLeapLiveLinkProducer::Startup()
{
	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(TEXT("Leap Motion Live Link"));
	ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic(&OnConnectionStatusChanged));

	SubjectName = TEXT("Leap Motion");
}

void FLeapLiveLinkProducer::ShutDown()
{
	LiveLinkProvider->UnregisterConnStatusChangedHandle(ConnectionStatusChangedHandle);
}

void FLeapLiveLinkProducer::LinkToSkeleton(const UBodyStateSkeleton* Skeleton)
{
	//Initialize the subject
	LiveLinkProvider->UpdateSubject(SubjectName, SubjectBoneNames, SubjectBoneParents);
}

void FLeapLiveLinkProducer::UpdateFromBodyState(const UBodyStateSkeleton* Skeleton)
{
	LiveLinkProvider->UpdateSubjectFrame(SubjectName, SubjectBoneTransforms, SubjectCurves, FApp::GetCurrentTime());
}