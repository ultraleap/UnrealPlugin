// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltraleapTrackingData.h"

const int32 MAX_FRAME_DATA_ENTRIES = 1000;

DECLARE_LOG_CATEGORY_EXTERN(LeapTransformStats, Log, All);

enum FrameDeltaType
{
	OriginalToSelf,
	InterpolatedToSelf,
	WarpedToSelf,
	InterpolatedToOriginal,
	WarpedToInterpolated,
	WarpedToOriginal
};

enum FrameType
{
	Original,
	Interpolated,
	Warped,
	SIXDOF
};

enum SummaryStatsType
{
	InterpolationStage,
	HeadPoseCorrectionStage,
	TimeWarpCorrectionStage
};

struct TransformSample
{
public:
	double TimeStamp;
	double maxDelta = 0;
	FVector Position_Left;
	FRotator Rotation_Left;
	FVector Position_Right;
	FRotator Rotation_Right;
};

struct LeapFrameTransformSample
{
public:
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
	TransformSample OriginalFrame;      // 1
	TransformSample InterpolatedFrame;  // 2 - if on
	float FrameExtrapolationInMS;
	TransformSample SixDOF; // 3
	TransformSample SixDOFTranslatedFame; // 3
	TransformSample TimeWarpTranslatedFrame; // 4 - if on
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

struct StatsRunOptions
{
public:
	FString RunName = "";
	bool FreezeFrameBeforeHeadPose;
	FLeapOptions Options;
};

/**
 * 
 */
class FLeapFrameTransformStats
{
public:
	void AddLeapFrameTransformSample(int frameID, LeapFrameTransformSample newSample);
	LeapFrameTransformSample& GetCurrentSample();

public:
	FLeapFrameTransformStats();
	~FLeapFrameTransformStats();

	bool UseTimewarp;
	bool UseInterpolation;

	StatsRunOptions StartStatsSweep(bool logLiveStatistics);

	LeapFrameTransformSample& GetPreviousSample();
	LeapFrameTransformSample& GetSample(int index);
	float GetDeltaAsPercent(FrameType frameType, int32 sampleSize);
	float GetDelta(FrameType frameType);
	float GetDelta(FrameType frameType, int Index);
	FColor ColourBasedOnDelta(float deltaAsPercentOfMaxDelta);
	StatsRunOptions StartNextStatsRun();
	bool IsCurrentStatsRunActive = false;
	bool IsStatsSweepComplete = true;
	int StatsSweepIndex = 0;
	TArray<StatsRunOptions> StatsSweepRunDetails; 
	void ThisSampleCollectionComplete();
	
	const bool Enabled = true;
	const bool InterpolationCollectionEnabled = false;

private:
	LeapFrameTransformSample Samples[MAX_FRAME_DATA_ENTRIES];
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
