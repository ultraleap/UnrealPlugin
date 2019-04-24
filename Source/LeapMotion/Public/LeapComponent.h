// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LeapMotionData.h"
#include "Components/ActorComponent.h"
#include "LeapComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLeapEventSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapDeviceSignature, FString, DeviceName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapVisibilityBoolSignature, bool, bIsVisible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapFrameSignature, const FLeapFrameData&, Frame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapHandSignature, const FLeapHandData&, Hand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeapPolicySignature, TArray<TEnumAsByte<ELeapPolicyFlag>>, Flags);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLeapImageEventSignature, UTexture2D*, Texture, ELeapImageType, ImageType);

UCLASS(ClassGroup = "Input Controller", meta = (BlueprintSpawnableComponent))

class LEAPMOTION_API ULeapComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:

	/** Called when a device connects to the leap service, this may happen before the game starts and you may not get the call*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapDeviceSignature OnLeapDeviceAttached;

	/** Called when a device disconnects from the leap service*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapDeviceSignature OnLeapDeviceDetatched;

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

	/** Event called when the leap service connects. Will likely be called before game begin play so some component won't receive this call.*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapEventSignature OnLeapServiceConnected;

	/** Event called if leap service connection gets lost. Track won't work if this event gets called.*/
	UPROPERTY(BlueprintAssignable, Category = "Leap Events")
	FLeapEventSignature OnLeapServiceDisconnected;

	/** Tracking mode optimization */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Properties")
	TEnumAsByte<ELeapMode> TrackingMode;

	/** Utility function to check if a hand is visible and tracked at this moment */
	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void AreHandsVisible(bool& LeftIsVisible, bool& RightIsVisible);

	/** Polling function to get latest data */
	UFUNCTION(BlueprintCallable, Category = "Leap Functions")
	void GetLatestFrameData(FLeapFrameData& OutData);

protected:

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
};