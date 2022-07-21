/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once
#include "InputCoreTypes.h" // for FKey
#include "UltraleapTrackingData.generated.h"

UENUM(BlueprintType)
enum EHandType
{
	LEAP_HAND_LEFT,
	LEAP_HAND_RIGHT
};

UENUM(BlueprintType)
enum ELeapMode
{
	LEAP_MODE_VR,		   // The tracking mode optimised for head-mounted devices.
	LEAP_MODE_DESKTOP,	   // The tracking mode optimised for desktop devices. Also known as 'Below'
	LEAP_MODE_SCREENTOP	   // The tracking mode optimised for screen top-mounted devices. Also know as 'Above Facing User'
};

UENUM(BlueprintType)
enum class ELeapImageType : uint8
{
	LEAP_IMAGE_LEFT,
	LEAP_IMAGE_RIGHT
};

UENUM(BlueprintType)
enum ELeapTrackingFidelity
{
	LEAP_CUSTOM,
	LEAP_LOW_LATENCY,
	LEAP_NORMAL,
	LEAP_SMOOTH,
	LEAP_WIRELESS
};

UENUM(BlueprintType)
enum ELeapPolicyFlag
{
	LEAP_POLICY_BACKGROUND_FRAMES,	   // The policy allowing an application to receive frames in the background.
	LEAP_POLICY_IMAGES,				   // The policy specifying whether to automatically stream images from the device.
	LEAP_POLICY_OPTIMIZE_HMD,		   // The policy specifying whether to optimize tracking for head-mounted device.
	LEAP_POLICY_ALLOW_PAUSE_RESUME,	   // The policy allowing an application to pause or resume service tracking
	LEAP_POLICY_MAP_POINTS			   // The policy allowing an application to receive per-frame map points
};

UENUM(BlueprintType)
enum ELeapServiceLogLevel
{
	LEAP_LOG_NONE = 0,
	LEAP_LOG_ERROR,
	LEAP_LOG_WARNING,
	LEAP_LOG_INFO
};

UENUM(BlueprintType)
enum ELeapMultiDeviceMode
{
	LEAP_MULTI_DEVICE_SINGULAR = 0,
	LEAP_MULTI_DEVICE_COMBINED
};

struct EKeysLeap
{
	static const FKey LeapPinchL;
	static const FKey LeapGrabL;

	static const FKey LeapPinchR;
	static const FKey LeapGrabR;
};
// see eLeapDevicePID (Blueprint equivalent)
UENUM(BlueprintType)
enum ELeapDeviceType
{
	/** An unknown device that is compatible with the tracking software. @since 3.1.3 */
	LEAP_DEVICE_TYPE_UNKNOWN,

	/** The Leap Motion Controller (the first consumer peripheral). @since 3.0.0 */
	LEAP_DEVICE_TYPE_PERIPHERAL,

	/** Internal research product codename "Dragonfly". @since 3.0.0 */
	LEAP_DEVICE_TYPE_DRAGONFLY,

	/** Internal research product codename "Nightcrawler". @since 3.0.0 */
	LEAP_DEVICE_TYPE_NIGHTCRAWLER,

	/** Research product codename "Rigel". @since 4.0.0 */
	LEAP_DEVICE_TYPE_RIGEL,

	/** The Ultraleap Stereo IR 170 (SIR170) hand tracking module. @since 5.3.0 */
	LEAP_DEVICE_TYPE_SIR170,

	/** The Ultraleap 3Di hand tracking camera. @since 5.3.0 */
	LEAP_DEVICE_TYPE_3DI,

	/** An invalid device type. Not currently in use. @since 3.1.3 */
	LEAP_DEVICE_INVALID = 0xFFFFFFFF
};
UENUM(BlueprintType)
enum ELeapDeviceCombinerClass
{
	LEAP_DEVICE_COMBINER_UNKNOWN,
	LEAP_DEVICE_COMBINER_CONFIDENCE,
	LEAP_DEVICE_COMBINER_ANGULAR
	// add your custom classes here and add them to the class factory in LeapWrapper
};
	USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapDevice
{
	GENERATED_USTRUCT_BODY()

	/** A combination of eLeapDeviceStatus flags. */
	// UPROPERTY(BlueprintReadOnly, Category = "Leap Device")
	int32 Status;

	/** A combination of eLeapDeviceCaps flags. */
	// UPROPERTY(BlueprintReadOnly, Category = "Leap Device")
	int32 Caps;

	/** One of the eLeapDevicePID members as a string. */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Device")
	FString PID;

	/**
	 * The device baseline, in micrometers.
	 *
	 * The baseline is defined as the distance between the center axis of each lens in a stereo camera
	 * system.  For other camera systems, this value is set to zero.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Device")
	int32 Baseline;

	/** Serial number string */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Device")
	FString Serial;

	/** The horizontal field of view of this device in radians. */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Stats")
	float HorizontalFOV;

	/** The vertical field of view of this device in radians. */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Stats")
	float VerticalFOV;

	/** The maximum range for this device, in micrometers. */
	UPROPERTY(BlueprintReadOnly, Category = "Leap Stats")
	int32 Range;

	void SetFromLeapDevice(struct _LEAP_DEVICE_INFO* LeapInfo);
};

/** Read only stats from the plugin such as version and prediction interval. */
USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapStats
{
	GENERATED_USTRUCT_BODY()
	FLeapStats();

	UPROPERTY(BlueprintReadOnly, Category = "Leap Stats")
	FString LeapAPIVersion;

	UPROPERTY(BlueprintReadOnly, Category = "Leap Stats")
	FLeapDevice DeviceInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Leap Stats")
	float FrameExtrapolationInMS;
};

USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapOptions
{
	GENERATED_USTRUCT_BODY()

	FLeapOptions();

	/** Optimize for desktop facing upward or VR facing outward? */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	TEnumAsByte<ELeapMode> Mode;

	/** Set your tracking fidelity from low latency to smooth. If not set to custom, some of the low level settings may be
	 * overwritten */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	TEnumAsByte<ELeapTrackingFidelity> TrackingFidelity;

	/** Verbosity of additional log updates from the leap service */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	TEnumAsByte<ELeapServiceLogLevel> LeapServiceLogLevel;

	/** Should leap use Temporal warping to align HMD rotation with leap samples */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	bool bUseTimeWarp;

	/** Whether leap should use frame interpolation for smooth tracking */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	bool bUseInterpolation;

	/** Should all leap data be transported to HMD space? */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	bool bTransformOriginToHMD;

	/** Timewarp offset sampling in microseconds. The higher, the further back in time. */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	float TimewarpOffset;

	/** Linear factor, useful for inverting timewarp effects for certain platforms (e.g. Oculus)*/
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	float TimewarpFactor;

	/** Number of frames we should predict forward (positive) or back (negative) from right now for hands */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	float HandInterpFactor;

	/** Number of frames we should predict forward (positive) or back (negative) from right now for fingers */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	float FingerInterpFactor;

	/** Fixed offset in leap space for all tracking data. Useful for setting Leap->HMD real world offset */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	FVector HMDPositionOffset;

	/** Fixed offset in leap space for all tracking data. Useful for setting Leap->HMD real world offset */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	FRotator HMDRotationOffset;

	/** Enable or disable the use of frame based gesture detection (old system)*/
	UPROPERTY(BlueprintReadWrite, Category = "Gesture Options")
	bool bUseFrameBasedGestureDetection;

	UPROPERTY(BlueprintReadWrite, Category = "Gesture Options")
	float StartGrabThreshold;

	UPROPERTY(BlueprintReadWrite, Category = "Gesture Options")
	float EndGrabThreshold;

	UPROPERTY(BlueprintReadWrite, Category = "Gesture Options")
	float StartPinchThreshold;

	UPROPERTY(BlueprintReadWrite, Category = "Gesture Options")
	float EndPinchThreshold;

	UPROPERTY(BlueprintReadWrite, Category = "Gesture Options")
	float GrabTimeout;

	UPROPERTY(BlueprintReadWrite, Category = "Gesture Options")
	float PinchTimeout;

	/** Experimental: Pull tracking data from OpenXR instead of LeapC.dll. Note that Pinch and Grasp events and strength are not yet
	 * implemented  */
	UPROPERTY(BlueprintReadWrite, Category = "Leap Options")
	bool bUseOpenXRAsSource;
};

USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapBoneData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Bone Data")
	FVector PrevJoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Bone Data")
	FVector NextJoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Bone Data")
	FRotator Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Bone Data")
	float Width;

	void SetFromLeapBone(struct _LEAP_BONE* bone, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset);
	void ScaleBone(float Scale);
	void RotateBone(const FRotator& InRotation);
	void TranslateBone(const FVector& InTranslation);
};

USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapPalmData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Palm Data")
	FVector Direction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Palm Data")
	FVector Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Palm Data")
	FRotator Orientation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Palm Data")
	FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Palm Data")
	FVector StabilizedPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Palm Data")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Palm Data")
	float Width;

	void SetFromLeapPalm(struct _LEAP_PALM* palm, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset);
	void ScalePalm(float Scale);
	void RotatePalm(const FRotator& InRotation);
	void TranslatePalm(const FVector& InTranslation);
};

USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapDigitData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Digit Data")
	TArray<FLeapBoneData> Bones;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Digit Data")
	FLeapBoneData Distal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Digit Data")
	int32 FingerId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Digit Data")
	FLeapBoneData Intermediate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Digit Data")
	bool IsExtended;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Digit Data")
	FLeapBoneData Metacarpal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Digit Data")
	FLeapBoneData Proximal;

	void SetFromLeapDigit(
		struct _LEAP_DIGIT* digit, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset);
	void ScaleDigit(float Scale);
	void RotateDigit(const FRotator& InRotation);
	void TranslateDigit(const FVector& InTranslation);
};

USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapHandData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	FLeapBoneData Arm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	float Confidence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	TArray<FLeapDigitData> Digits;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	int32 Flags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	float GrabAngle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	float GrabStrength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	TEnumAsByte<EHandType> HandType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	FLeapDigitData Index;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	FLeapDigitData Middle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	FLeapPalmData Palm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	float PinchDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	float PinchStrength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	FLeapDigitData Pinky;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	FLeapDigitData Ring;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	FLeapDigitData Thumb;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	int32 Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Leap Hand Data")
	float VisibleTime;

	/** Copy all data from leap type*/
	void SetFromLeapHand(struct _LEAP_HAND* hand, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset);

	/** Used in interpolation*/
	void SetArmPartialsFromLeapHand(
		struct _LEAP_HAND* hand, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset);

	void ScaleHand(float Scale);
	void RotateHand(const FRotator& InRotation);
	void TranslateHand(const FVector& InTranslation);

	void InitFromEmpty(const EHandType HandTypeIn, const int HandID);
	void UpdateFromDigits();
};

USTRUCT(BlueprintType)
struct ULTRALEAPTRACKING_API FLeapFrameData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultraleap Tracking Data")
	int32 NumberOfHandsVisible;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultraleap Tracking Data")
	int32 FrameRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultraleap Tracking Data")
	TArray<FLeapHandData> Hands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultraleap Tracking Data")
	int32 FrameId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultraleap Tracking Data")
	bool LeftHandVisible;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultraleap Tracking Data")
	bool RightHandVisible;

	// int64 not supported by blueprint, so this will only be accessible inside c++
	UPROPERTY()
	int64 TimeStamp;

	UPROPERTY()
	FRotator FinalRotationAdjustment;

	FLeapHandData HandForId(int32 HandId);

	void SetFromLeapFrame(
		struct _LEAP_TRACKING_EVENT* frame, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset);
	void SetInterpolationPartialFromLeapFrame(
		struct _LEAP_TRACKING_EVENT* frame, const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset);
	void ScaleFrame(float Scale);
	void RotateFrame(const FRotator& InRotation);
	void TranslateFrame(const FVector& InTranslation);
};
UENUM()
enum class ELeapQuatSwizzleAxisB : uint8
{
	X UMETA(DisplayName = "X"),
	Y UMETA(DisplayName = "Y"),
	Z UMETA(DisplayName = "Z"),
	W UMETA(DisplayName = "W"),
	MinusX UMETA(DisplayName = "-X"),
	MinusY UMETA(DisplayName = "-Y"),
	MinusZ UMETA(DisplayName = "-Z"),
	MinusW UMETA(DisplayName = "-W")
};