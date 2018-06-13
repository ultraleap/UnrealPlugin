#include "LeapLiveLink.h"
#include "LiveLinkProvider.h"

void FLeapLiveLinkProducer::Startup()
{
	LiveLinkProvider = ILiveLinkProvider::CreateLiveLinkProvider(TEXT("Leap Motion Live Link"));
	//ConnectionStatusChangedHandle = LiveLinkProvider->RegisterConnStatusChangedHandle(FLiveLinkProviderConnectionStatusChanged::FDelegate::CreateStatic(&OnConnectionStatusChanged));
}

void FLeapLiveLinkProducer::ShutDown()
{

}

