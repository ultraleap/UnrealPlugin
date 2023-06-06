/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "LeapWrapper.h"
#include "SceneManagement.h"


/**
 * Transforms that are tracked on the hand with the ultraleap extension (has elbow/forearm).
 *
 */
UENUM(BlueprintType)
enum class EHandKeypointUL : uint8
{
	Palm,
	Wrist,
	ThumbMetacarpal,
	ThumbProximal,
	ThumbDistal,
	ThumbTip,
	IndexMetacarpal,
	IndexProximal,
	IndexIntermediate,
	IndexDistal,
	IndexTip,
	MiddleMetacarpal,
	MiddleProximal,
	MiddleIntermediate,
	MiddleDistal,
	MiddleTip,
	RingMetacarpal,
	RingProximal,
	RingIntermediate,
	RingDistal,
	RingTip,
	LittleMetacarpal,
	LittleProximal,
	LittleIntermediate,
	LittleDistal,
	LittleTip,
	Elbow
};
/**
 *
 */
class FOpenXRToLeapWrapper : public FLeapWrapperBase
{
public:
	FOpenXRToLeapWrapper();
	virtual ~FOpenXRToLeapWrapper();

	void UpdateHandState();

	// FLeapWrapperBase overrides (base stubs out old leap calls)
	virtual LEAP_CONNECTION* OpenConnection(LeapWrapperCallbackInterface* InCallbackDelegate, bool UseMultiDeviceMode) override;
	virtual void CloseConnection() override;
	virtual LEAP_TRACKING_EVENT* GetInterpolatedFrameAtTime(int64 TimeStamp) override;
	virtual LEAP_TRACKING_EVENT* GetFrame() override;
	virtual LEAP_DEVICE_INFO* GetDeviceProperties() override;
	virtual int64_t GetNow() override
	{
		return GetDummyLeapTime();
	}
	virtual void SetWorld(UWorld* World) override;

	virtual void SetSwizzles(
		ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW) override
	{
		SwizzleX = ToX;
		SwizzleY = ToY;
		SwizzleZ = ToZ;
		SwizzleW = ToW;
	}
	virtual void SetTrackingMode(eLeapTrackingMode TrackingMode) override;
	virtual uint32_t GetDeviceID() override
	{
		return DeviceID;
	}
	virtual FString GetDeviceSerial() override
	{
		// NOTE: When we support device serials in OpenXR we can return them here
		return FString::Printf(TEXT("OpenXR Device %d"), DeviceID - OpenXRBaseDeviceID);
	}
	virtual EDeviceType GetDeviceType()
	{
		return DEVICE_TYPE_OPENXR;
	}
	virtual IHandTrackingDevice* GetDevice() override;
	
	virtual void PostLeapHandUpdate(FLeapFrameData& Frame) override;

	

private:
	class IXRTrackingSystem* XRTrackingSystem = nullptr;
	class IHandTracker* HandTracker = nullptr;
	void InitOpenXRHandTrackingModule();
	LEAP_TRACKING_EVENT DummyLeapFrame;
	LEAP_HAND DummyLeapHands[2];
	LEAP_DEVICE_INFO DummyDeviceInfo;

	void SetHandJointFromKeypoint(const int Keypoint, LEAP_HAND& LeapHand, const FVector& Position, const FQuat& Rotation);
	void SetHandJointFromKeypointExt(const int Keypoint, LEAP_HAND& LeapHand, const FVector& Position, const FQuat& Rotation);

	LEAP_QUATERNION ConvertOrientationToLeap(const FQuat& FromOpenXR);
	void ConvertToLeapSpace(LEAP_HAND& LeapHand, const TArray<FVector>& Positions, const TArray<FQuat>& Rotations);
	int64_t GetDummyLeapTime();


	// port of unity leap grab and pinch implementations (in turn ported from LeapC)
	static float CalculatePinchStrength(const FLeapHandData& Hand, float PalmWidth);
	static float CalculateBoneDistanceSquared(const FLeapBoneData& BoneA, const FLeapBoneData& BoneB);
	static float CalculatePinchDistance(const FLeapHandData& Hand);
	static float CalculateGrabStrength(const FLeapHandData& Hand);
	static float CalculateGrabAngle(const FLeapHandData& Hand);
	void UpdatePinchAndGrab(FLeapHandData& Hand);
	float GetGameTimeInSeconds();

	ELeapQuatSwizzleAxisB SwizzleX = ELeapQuatSwizzleAxisB::MinusY;
	ELeapQuatSwizzleAxisB SwizzleY = ELeapQuatSwizzleAxisB::MinusZ;
	ELeapQuatSwizzleAxisB SwizzleZ = ELeapQuatSwizzleAxisB::X;
	ELeapQuatSwizzleAxisB SwizzleW = ELeapQuatSwizzleAxisB::W;

	int32 DeviceID = 0;
	TSharedPtr<class FUltraleapDevice> Device;
	
	// prevent overlap with Leap Device IDs just in case we want to mix the two
	static const int32 OpenXRBaseDeviceID = 10000;
	bool UsingUltraleapExtension = false;
	bool LeftHandVisible = false;
	bool RightHandVisible = false;
	int32 HandID = 0;
	float FirstSeenLeft = 0.0f;
	float FirstSeenRight = 0.0f;
};
