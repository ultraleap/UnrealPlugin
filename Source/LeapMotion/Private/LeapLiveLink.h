#pragma once

#include "Runtime/LiveLinkInterface/Public/LiveLinkTypes.h"
#include "Runtime/LiveLinkInterface/Public/ILiveLinkClient.h"
#include "Runtime/LiveLinkMessageBusFramework/Public/LiveLinkProvider.h"

class FLeapLiveLinkProducer
{
	TSharedPtr<ILiveLinkProvider> LiveLinkProvider;

	void Startup();
	void ShutDown();
};