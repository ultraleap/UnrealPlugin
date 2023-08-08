// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltraleapTrackingData.h"
#include "ULeapFrameTransformStats.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LeapTransformStats, Log, All);

const int32 MAX_FRAME_DATA_ENTRIES = 1000;

UENUM(BlueprintType)
enum FrameDeltaType
{
	OriginalToSelf,
	InterpolatedToSelf,
	WarpedToSelf,
	InterpolatedToOriginal,
	WarpedToInterpolated,
	WarpedToOriginal
};

UENUM(BlueprintType)
enum FrameType
{
	Original,
	Interpolated,
	Warped,
	SIXDOF
};

UENUM(BlueprintType)
enum SummaryStatsType
{
	InterpolationStage,
	HeadPoseCorrectionStage,
	TimeWarpCorrectionStage
};

USTRUCT(BlueprintType)
struct FTransformSample
{
public:
	GENERATED_USTRUCT_BODY()


	double TimeStamp;
	double maxDelta = 0;
	FVector Position_Left;
	FRotator Rotation_Left;
	FVector Position_Right;
	FRotator Rotation_Right;
};

USTRUCT(BlueprintType)
struct FLeapFrameTransformSample
{
public:
	GENERATED_USTRUCT_BODY()


	int LeftHandId = -1;
	int RightHandId = -1;

	bool InterpolationOn;
	bool TimeWarpOn;
	float TimeWarpOffset;
	float TimeWarpFactor;

	float HandTrackingFrameRate;
	int64 RenderFrameTimeInMicros;
	float AverageRenderFPS;
	float AverageRenderMS;
	int FrameID;
	FTransformSample OriginalFrame;      // 1
	FTransformSample InterpolatedFrame;  // 2 - if on
	float FrameExtrapolationInMS;
	FTransformSample SixDOF; // 3
	FTransformSample SixDOFTranslatedFame; // 3
	FTransformSample TimeWarpTranslatedFrame; // 4 - if on
	bool FreezeFrameBeforeHeadPose;
	
	double TimeWarpTimeStamp;
	double TimeWarpRequestedTimeStamp;
	float TimeWarpInterpolationSnapshotDelta;
	FVector JointPositionHeadPoseCorrected;
	FVector JointPositionTimeWarpCorrected;
	FVector TimeWarpDelta;


	// Head Pose stuff
	FVector HeadPoseTranslation;
	FRotator HeadPoseRotation;
	FVector WarpPoseTranslation;
	FRotator WarpPoseRotation;
	FVector FinalHMDTranslation;
	FRotator FinalHMDRotation;
};

USTRUCT(BlueprintType)
struct FStatsRunOptions
{
public:
	GENERATED_USTRUCT_BODY()

	FString RunName = "";
	bool FreezeFrameBeforeHeadPose;
	FLeapOptions Options;
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ULeapFrameTransformStats : public UObject
{
public:
	GENERATED_BODY()

	ULeapFrameTransformStats();
	~ULeapFrameTransformStats();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool UseTimewarp;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool UseInterpolation;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TArray<FStatsRunOptions> StatsSweepRunDetails; 

	UFUNCTION(BlueprintCallable, Category = "Stats|Capture")
	FStatsRunOptions StartStatsSweep(bool logLiveStatistics);

	FLeapFrameTransformSample& GetPreviousSample();
	FLeapFrameTransformSample& GetSample(int index);
	float GetDeltaAsPercent(FrameType frameType, int32 sampleSize);
	float GetDelta(FrameType frameType);
	float GetDelta(FrameType frameType, int Index);
	FColor ColourBasedOnDelta(float deltaAsPercentOfMaxDelta);
	FStatsRunOptions StartNextStatsRun();
	void AddLeapFrameTransformSample(int frameID, FLeapFrameTransformSample newSample);
	FLeapFrameTransformSample& GetCurrentSample();

	bool IsCurrentStatsRunActive = false;
	bool IsStatsSweepComplete = true;
	int StatsSweepIndex = 0;

	void ThisSampleCollectionComplete();
	
	const bool Enabled = true;
	const bool InterpolationCollectionEnabled = false;

private:
	FLeapFrameTransformSample Samples[MAX_FRAME_DATA_ENTRIES];
	int CurrentIndex = 0;
	void LogLiveStats();

	float GetPositionDeltaMagnitude(FrameDeltaType frameType);
	FVector GetCurrentPositionRelativeToRollingAverage(FrameType frameType, int32 sampleSize);
	float GetStandardDeviation(FrameType frameType, int32 sampleSize);
	FVector GetRollingAverage(FrameType frameType, int32 sampleSize);

	void CalculateSummaryStats(SummaryStatsType forStage);
	float GetSummaryStatsDelta(SummaryStatsType forStage, int32 index, FVector averagePosition);
	FVector GetAveragePosition(SummaryStatsType forStage);

	bool logLiveStats = true;
	bool haveData = false;
};
