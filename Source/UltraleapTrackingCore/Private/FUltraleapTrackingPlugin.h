

#pragma once

#include "IUltraleapTrackingPlugin.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class FUltraleapTrackingPlugin : public IUltraleapTrackingPlugin
{
public:
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void AddEventDelegate(const ULeapComponent* EventDelegate) override;
	virtual void RemoveEventDelegate(const ULeapComponent* EventDelegate) override;
	virtual FLeapStats GetLeapStats() override;
	virtual void SetOptions(const FLeapOptions& Options) override;
	virtual FLeapOptions GetOptions() override;
	virtual void AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible) override;
	virtual void GetLatestFrameData(FLeapFrameData& OutData) override;
	virtual void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable) override;
	virtual void GetAttachedDevices(TArray<FString>& Devices) override;

	virtual void ShutdownLeap() override;
	virtual void SetSwizzles(
		ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW) override;

	bool IsActive();

private:
	TSharedPtr<class FUltraleapTrackingInputDevice> LeapInputDevice;
	TArray<ULeapComponent*> DeferredComponentList;

	bool bActive = false;
	void* LeapDLLHandle;

	void* GetLeapHandle();
};
