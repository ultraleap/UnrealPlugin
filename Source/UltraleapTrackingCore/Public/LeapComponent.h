/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "Components/ActorComponent.h"
#include "LeapWrapper.h"
#include "UltraleapTrackingData.h"
#include "IUltraleapTrackingPlugin.h"

#if WITH_EDITOR
#include "DetailLayoutBuilder.h"
#endif
#include "LeapComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLeapEventSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapDeviceSignature, FString, DeviceName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapVisibilityBoolSignature, bool, bIsVisible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapFrameSignature, const FLeapFrameData&, Frame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapHandSignature, const FLeapHandData&, Hand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapPolicySignature, TArray<TEnumAsByte<ELeapPolicyFlag>>, Flags);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLeapImageEventSignature, UTexture2D*, Texture, ELeapImageType, ImageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapTrackingModeSignature, ELeapMode, Flag);

UCLASS(ClassGroup = "Input Controller", meta = (BlueprintSpawnableComponent))

class ULTRALEAPTRACKING_API ULeapComponent : public UActorComponent, public ILeapConnectorCallbacks
{
	GENERATED_UCLASS_BODY()
public:
	~ULeapComponent();

	/** Called when a device connects to the leap service, this may happen before the game starts and you may not get the call*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapDeviceSignature OnLeapDeviceAttached;

	/** Called when a device disconnects from the leap service*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapDeviceSignature OnLeapDeviceDetached;

	/** Event called when new tracking data is available, typically every game tick. */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapFrameSignature OnLeapTrackingData;

	/** Event called when a leap hand grab gesture is detected */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapHandSignature OnHandGrabbed;

	/** Event called when a leap hand release gesture is detected */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapHandSignature OnHandReleased;

	/** Event called when a leap hand pinch gesture is detected */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapHandSignature OnHandPinched;

	/** Event called when a leap hand unpinch gesture is detected */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapHandSignature OnHandUnpinched;

	/** Event called when a leap hand enters the field of view and begins tracking */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapHandSignature OnHandBeginTracking;

	/** Event called when a leap hand exits the field of view and stops tracking */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapHandSignature OnHandEndTracking;

	/** Event called when the left hand tracking changes*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapVisibilityBoolSignature OnLeftHandVisibilityChanged;

	/** Event called when the right hand begins tracking */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapVisibilityBoolSignature OnRightHandVisibilityChanged;

	/** Event called when leap policies have changed */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapPolicySignature OnLeapPoliciesUpdated;

	/** Event called when a device image is ready. Requires setting image policy first*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapImageEventSignature OnImageEvent;

	/** Event called when the leap service connects. Will likely be called before game begin play so some component won't receive
	 * this call.*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapEventSignature OnLeapServiceConnected;

	/** Event called if leap service connection gets lost. Track won't work if this event gets called.*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapEventSignature OnLeapServiceDisconnected;

	/** Tracking mode optimization */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Properties")
	TEnumAsByte<ELeapMode> TrackingMode;

	/** Event called when leap policies have changed */
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapTrackingModeSignature OnLeapTrackingModeUpdated;

	// By default in vr mode the first/primary device has this set to true
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Leap Properties")
	bool bAddHmdOrigin;

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void SetShouldAddHmdOrigin(bool& bShouldAdd);

	/** Utility function to check if a hand is visible and tracked at this moment */
	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void AreHandsVisible(bool& LeftIsVisible, bool& RightIsVisible);

	/** Polling function to get latest data */
	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void GetLatestFrameData(FLeapFrameData& OutData, const bool ApplyDeviceOrigin = false);

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void SetSwizzles(ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW);
	

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void SetTrackingMode(ELeapMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable);
	
	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	bool GetLeapOptions(FLeapOptions& Options);

	/**
	* Get the hand size, by default it will get the left hand size
	* In this method that we measure the middle finger length + palm position 
	* to start of finger as an indication of the hand size
	* @param OutHandSize - returns the hand size
	*/
	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void GetHandSize(float& OutHandSize);


	/** Multidevice configuration, Singular subscribes to a single device. 
	Combined subscribes to multiple devices combined into one device
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere,
		Category = "Leap Devices", meta = (
			EditCondition = "DisableEditMultiDeviceMode == false"))
	TEnumAsByte<ELeapMultiDeviceMode> MultiDeviceMode;

	/** Available device list
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Leap Devices")
	TArray<FString> AvailableDeviceSerials;

	/** Active Device (Singular mode only)
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Leap Devices", meta = (GetOptions = "GetSerialOptions", EditCondition = "MultiDeviceMode == ELeapMultiDeviceMode::LEAP_MULTI_DEVICE_SINGULAR"))
	FString ActiveDeviceSerial;

	UFUNCTION(CallInEditor)
	TArray<FString> GetSerialOptions() const
	{
		TArray<FString> Ret(AvailableDeviceSerials);
		Ret.Insert(NameConstantNone, 0);
		return Ret;
	}

	/** Combined device list
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Leap Devices", meta = (GetOptions = "GetSerialOptions", EditCondition = "MultiDeviceMode == ELeapMultiDeviceMode::LEAP_MULTI_DEVICE_COMBINED"))
	TArray<FString> CombinedDeviceSerials;


	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Leap Devices",
		meta = (GetOptions = "GetSerialOptions" , EditCondition = "MultiDeviceMode == ELeapMultiDeviceMode::LEAP_MULTI_DEVICE_COMBINED"))
	TEnumAsByte<ELeapDeviceCombinerClass> DeviceCombinerClass;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Leap Devices")
	ELeapDeviceType GetDeviceTypeFromSerial(const FString& DeviceSerial);


	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Leap Devices")
	void UpdateDeviceOrigin(const FTransform& DeviceOriginIn);

	// Setup multidevice programmatically in one call
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Leap Devices")
	void SetupMultidevice(
		const TArray<FString>& DeviceSerials, const ELeapMultiDeviceMode MultiDeviceModeIn, const ELeapDeviceCombinerClass CombinerClass);

	/** Disable/Grey out setting the multidevice mode.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leap Devices")
	bool DisableEditMultiDeviceMode;


#if WITH_EDITOR
	// property change handlers
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void SetCustomDetailsPanel(IDetailLayoutBuilder* DetailBuilder);
#endif

	// ILeapConnectorCallbacks implementation
	virtual void OnDeviceAdded(IHandTrackingWrapper* DeviceWrapper) override;
	virtual void OnDeviceRemoved(IHandTrackingWrapper* DeviceWrapper) override;

	
	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	bool IsActiveDevicePluggedIn();

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void GetMultiDeviceDebugInfo(int32& NumLeftTracked, int32& NumRightTracked);


	class IHandTrackingDevice* GetCombinedDeviceBySerials(const TArray<FString>& DeviceSerials);

	UFUNCTION()
	bool GetDeviceOrigin(FTransform& DeviceOrigin);

 protected:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	bool UpdateMultiDeviceMode(const ELeapMultiDeviceMode DeviceMode);

	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	bool UpdateActiveDevice(const FString& DeviceSerial);

private:
	void RefreshDeviceList(const bool NotifyChangeToUI = false);
	bool SubscribeToDevice();
	bool UnsubscribeFromCurrentDevice();

#if WITH_EDITOR
	// custom detail panel interface
	IDetailLayoutBuilder* DetailBuilder;
#endif
	void ConnectToInputEvents();
	bool IsConnectedToInputEvents;

	IHandTrackingWrapper* CurrentHandTrackingDevice = nullptr;

	static const FString NameConstantNone;
};