// Fill out your copyright notice in the Description page of Project Settings.


#include "FLeapFrameTransformStats.h"

DEFINE_LOG_CATEGORY(LeapTransformStats);

FLeapFrameTransformStats::FLeapFrameTransformStats()
{
	//// Set up our stats runs
	//StatsRunOptions lowLatencyStatsRun;
	//lowLatencyStatsRun.RunName = "LowLatency";
	//lowLatencyStatsRun.Options.bUseTimeWarp = true;
	//lowLatencyStatsRun.Options.bUseInterpolation = true;
	//lowLatencyStatsRun.Options.TimewarpOffset = 9000;
	//lowLatencyStatsRun.Options.TimewarpFactor = 1.f;
	//lowLatencyStatsRun.Options.HandInterpFactor = 0.5f;
	//lowLatencyStatsRun.Options.FingerInterpFactor = 0.5f;
	//StatsSweepRunDetails.Add(lowLatencyStatsRun);

	StatsRunOptions normalStatsRun;
	normalStatsRun.RunName = "Normal";
	normalStatsRun.FreezeFrameBeforeHeadPose = false;
	normalStatsRun.Options.bUseTimeWarp = true;
	normalStatsRun.Options.bUseInterpolation = true;
	normalStatsRun.Options.TimewarpOffset = 2750;
	normalStatsRun.Options.TimewarpFactor = 1.f;
	normalStatsRun.Options.HandInterpFactor = 0.f;
	normalStatsRun.Options.FingerInterpFactor = 0.f;
	StatsSweepRunDetails.Add(normalStatsRun);

	StatsRunOptions normalStatsRun2;
	normalStatsRun2.RunName = "Normal Pre Head Pose Frozen Hands";
	normalStatsRun2.FreezeFrameBeforeHeadPose = true;
	normalStatsRun2.Options.bUseTimeWarp = true;
	normalStatsRun2.Options.bUseInterpolation = true;
	normalStatsRun2.Options.TimewarpOffset = 2750;
	normalStatsRun2.Options.TimewarpFactor = 1.f;
	normalStatsRun2.Options.HandInterpFactor = 0.f;
	normalStatsRun2.Options.FingerInterpFactor = 0.f;
	StatsSweepRunDetails.Add(normalStatsRun2);


	//StatsRunOptions smoothStatsRun;
	//smoothStatsRun.RunName = "Smooth";
	//smoothStatsRun.Options.bUseTimeWarp = false;
	//smoothStatsRun.Options.bUseInterpolation = true;
	//smoothStatsRun.Options.TimewarpOffset = 500;
	//smoothStatsRun.Options.TimewarpFactor = -1.f;
	//smoothStatsRun.Options.HandInterpFactor = -1.f;
	//smoothStatsRun.Options.FingerInterpFactor = -1.f;
	//StatsSweepRunDetails.Add(smoothStatsRun);

	//StatsRunOptions allOffStatsRun;
	//allOffStatsRun.RunName = "NoInterpolationOrTimewarp";
	//allOffStatsRun.Options.bUseTimeWarp = false;
	//allOffStatsRun.Options.bUseInterpolation = false;
	//allOffStatsRun.Options.TimewarpOffset = 0;
	//allOffStatsRun.Options.TimewarpFactor = 0;
	//allOffStatsRun.Options.HandInterpFactor = 0;
	//allOffStatsRun.Options.FingerInterpFactor = 0;
	//StatsSweepRunDetails.Add(allOffStatsRun);

	/*for (float handInterpFactor = -1.5; handInterpFactor <= 1.5; handInterpFactor += 0.25)
	{
		StatsRunOptions statsRun;
		statsRun.RunName = *FString::Printf(TEXT("InterpFactor %f"), handInterpFactor);
		statsRun.Options.bUseTimeWarp = false;
		statsRun.Options.bUseInterpolation = true;
		statsRun.Options.TimewarpOffset = 500;
		statsRun.Options.TimewarpFactor = -1.f;
		statsRun.Options.HandInterpFactor = handInterpFactor;
		statsRun.Options.FingerInterpFactor = handInterpFactor;
		StatsSweepRunDetails.Add(statsRun);
	}*/

	UE_LOG(LeapTransformStats, Log, TEXT("Setup %d stats runs"), StatsSweepRunDetails.Num());
}

FLeapFrameTransformStats::~FLeapFrameTransformStats()
{
}

void FLeapFrameTransformStats::AddLeapFrameTransformSample(int frameID, LeapFrameTransformSample newSample)
{
	if (IsCurrentStatsRunActive)
	{
		newSample.FrameID = frameID;
		Samples[CurrentIndex] = newSample;
		haveData = true;
	}
}

/// <summary>
/// Current sample has had all data committed
/// </summary>
void FLeapFrameTransformStats::ThisSampleCollectionComplete()
{
	if (IsCurrentStatsRunActive)
	{
		if (CurrentIndex + 1 >= MAX_FRAME_DATA_ENTRIES)
		{
			CurrentIndex = 0;
			IsCurrentStatsRunActive = false;
			CalculateSummaryStats(SummaryStatsType::HeadPoseCorrectionStage);
			CalculateSummaryStats(SummaryStatsType::TimeWarpCorrectionStage);

			if (StatsSweepIndex >= StatsSweepRunDetails.Num())
			{
				IsStatsSweepComplete = true;
				UE_LOG(LeapTransformStats, Log, TEXT("Stats sweep complete"));
			}
		}
		else
		{
			LogLiveStats();
			CurrentIndex++;
		}
	}
}

LeapFrameTransformSample& FLeapFrameTransformStats::GetCurrentSample()
{
	return Samples[CurrentIndex];
}

void FLeapFrameTransformStats::LogLiveStats()
{
	if (haveData && logLiveStats)
	{
		if (CurrentIndex == 0)
		{
			//UE_LOG(LeapTransformStats, Log, TEXT("LeapTransformStats: FrameID, Original Data: Frame Timestamp (us), Delta Timestamp (us), Position (X,Y,Z) cm,  Position Delta (cm), Position Delta as Percent of rolling average,     Interpolated Data: Timestamp (us), Delta Timestamp (us), Interpolated Position (X,Y,Z) (cm),  Position Delta (cm), Position Delta as Percent of rolling average,    Timewarp Data: (X,Y,Z), Rotations Original/Interpolated/Timewarp (X,Y,Z), (X,Y,Z), (X,Y,Z), Render Frame Rate"));
			UE_LOG(LeapTransformStats, Log, TEXT("Interpolation On, Time Warp On, Time Warp offset, Time warp Factor, FrameID, Original Data: Frame Timestamp (us), Delta Timestamp (us), Average Render Frame Rate (FPS), Average Render Frame Time (),  Head Pose Translation (X, Y, Z), Head Pose Rotation (X,Y,Z), Time Warp Timestamp, Time Warp Requested Time Stamp, Time Warp Interpolation Frame Delta, Head Pose Corrected(X, Y, Z), Time Warp Corrected(X, Y, Z), Delta, Freeze Frame Before Head Pose"));
		}
		
		if (CurrentIndex > 0 && Samples[CurrentIndex].LeftHandId != -1)
		{
			/*UE_LOG(LeapTransformStats, Log, TEXT("%d,   %f,%f, %f,%f,%f,  %f, %f,     %f,%f, %f,%f,%f, %f,%f,    %f,%f,%f, %f,  %f,%f,%f,   %f,%f,%f,   %f,%f,%f,   %lld"),

				Samples[CurrentIndex].FrameID,

				(Samples[CurrentIndex].OriginalFrame.TimeStamp - Samples[0].OriginalFrame.TimeStamp) / 1000.0,
				(Samples[CurrentIndex].OriginalFrame.TimeStamp - GetPreviousSample().OriginalFrame.TimeStamp) / 1000.0,
				Samples[CurrentIndex].OriginalFrame.Position_Left.X, Samples[CurrentIndex].OriginalFrame.Position_Left.Y, Samples[CurrentIndex].OriginalFrame.Position_Left.Z,
				(Samples[CurrentIndex].OriginalFrame.Position_Left - GetPreviousSample().OriginalFrame.Position_Left).Size(),
				GetDeltaAsPercent(Original, 45),

				(Samples[CurrentIndex].InterpolatedFrame.TimeStamp - Samples[0].InterpolatedFrame.TimeStamp) / 1000.0,
				(Samples[CurrentIndex].InterpolatedFrame.TimeStamp - GetPreviousSample().InterpolatedFrame.TimeStamp) / 1000.0,
				Samples[CurrentIndex].InterpolatedFrame.Position_Left.X, Samples[CurrentIndex].InterpolatedFrame.Position_Left.Y, Samples[CurrentIndex].InterpolatedFrame.Position_Left.Z,
				(Samples[CurrentIndex].InterpolatedFrame.Position_Left - GetPreviousSample().InterpolatedFrame.Position_Left).Size(),
				GetDeltaAsPercent(Interpolated, 45),

				Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.X, Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.Y, Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.Z,
				(Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left - GetPreviousSample().TimeWarpTranslatedFrame.Position_Left).Size(),

				Samples[CurrentIndex].OriginalFrame.Rotation_Left.Euler().X, Samples[CurrentIndex].OriginalFrame.Rotation_Left.Euler().Y, Samples[CurrentIndex].OriginalFrame.Rotation_Left.Euler().Z,
				Samples[CurrentIndex].InterpolatedFrame.Rotation_Left.Euler().X, Samples[CurrentIndex].InterpolatedFrame.Rotation_Left.Euler().Y, Samples[CurrentIndex].InterpolatedFrame.Rotation_Left.Euler().Z,
				Samples[CurrentIndex].TimeWarpTranslatedFrame.Rotation_Left.Euler().X, Samples[CurrentIndex].TimeWarpTranslatedFrame.Rotation_Left.Euler().Y, Samples[CurrentIndex].TimeWarpTranslatedFrame.Rotation_Left.Euler().Z,

				Samples[CurrentIndex].RenderFrameTimeInMicros
			);*/

			
			UE_LOG(LeapTransformStats, Log, TEXT("%d, %d, %f, %f,  %d,  %f,%f,  %f, %f,  %f, %f, %f,  %f, %f, %f,    %f, %f, %f,   %f, %f, %f,  %f, %f, %f,   %f, %d"),

				Samples[CurrentIndex].InterpolationOn,
				Samples[CurrentIndex].TimeWarpOn,
				Samples[CurrentIndex].TimeWarpOffset,
				Samples[CurrentIndex].TimeWarpFactor,


				Samples[CurrentIndex].FrameID,

				(Samples[CurrentIndex].OriginalFrame.TimeStamp - Samples[0].OriginalFrame.TimeStamp) / 1000.0,
				(Samples[CurrentIndex].OriginalFrame.TimeStamp - GetPreviousSample().OriginalFrame.TimeStamp) / 1000.0,

				Samples[CurrentIndex].AverageRenderFPS,
				Samples[CurrentIndex].AverageRenderMS,

				Samples[CurrentIndex].HeadPoseTranslation.X, Samples[CurrentIndex].HeadPoseTranslation.Y, Samples[CurrentIndex].HeadPoseTranslation.Z,
				Samples[CurrentIndex].HeadPoseRotation.Euler().X, Samples[CurrentIndex].HeadPoseRotation.Euler().Y, Samples[CurrentIndex].HeadPoseRotation.Euler().Z,

				Samples[CurrentIndex].TimeWarpTimeStamp / 1000.0,
				Samples[CurrentIndex].TimeWarpRequestedTimeStamp / 1000.0,
				Samples[CurrentIndex].TimeWarpInterpolationSnapshotDelta / 1000.0,

				Samples[CurrentIndex].JointPositionHeadPoseCorrected.X, Samples[CurrentIndex].JointPositionHeadPoseCorrected.Y, Samples[CurrentIndex].JointPositionHeadPoseCorrected.Z,
				Samples[CurrentIndex].JointPositionTimeWarpCorrected.X, Samples[CurrentIndex].JointPositionTimeWarpCorrected.Y, Samples[CurrentIndex].JointPositionTimeWarpCorrected.Z,
			
				Samples[CurrentIndex].TimeWarpDelta.Size(),
				Samples[CurrentIndex].FreezeFrameBeforeHeadPose
			);
		}
		else
		{
			UE_LOG(LeapTransformStats, Log, TEXT("%d, no hands"), Samples[CurrentIndex].FrameID);
		}


		/*UE_LOG(LeapTransformStats, Log, TEXT("LH : O (%f,%f,%f), I (%f,%f,%f), TW (%f,%f,%f), DO %f DI %f DW %f"),
			Samples[CurrentIndex].OriginalFrame.Position_Left.X, Samples[CurrentIndex].OriginalFrame.Position_Left.Y, Samples[CurrentIndex].OriginalFrame.Position_Left.Z,
			Samples[CurrentIndex].InterpolatedFrame.Position_Left.X, Samples[CurrentIndex].InterpolatedFrame.Position_Left.Y, Samples[CurrentIndex].InterpolatedFrame.Position_Left.Z,
			Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.X, Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.Y, Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.Z,
			GetPositionDelta(Original).Size(),
			GetPositionDelta(Interpolated).Size(),
			GetPositionDelta(Warped).Size()); */

		//UE_LOG(LeapTransformStats, Log, TEXT("%f,%f,%f,  ,%f,%f,%f,  ,%f,%f,%f,   %f, %f"),
		//	Samples[CurrentIndex].OriginalFrame.Position_Left.X, Samples[CurrentIndex].OriginalFrame.Position_Left.Y, Samples[CurrentIndex].OriginalFrame.Position_Left.Z,
		//	Samples[CurrentIndex].InterpolatedFrame.Position_Left.X, Samples[CurrentIndex].InterpolatedFrame.Position_Left.Y, Samples[CurrentIndex].InterpolatedFrame.Position_Left.Z,
		//	Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.X, Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.Y, Samples[CurrentIndex].TimeWarpTranslatedFrame.Position_Left.Z,
		//	GetPositionDeltaMagnitude(InterpolatedToOriginal),
		//	GetPositionDeltaMagnitude(WarpedToInterpolated),
		//	GetPositionDeltaMagnitude(WarpedToOriginal));

		/* UE_LOG(LeapTransformStats, Log, TEXT("%f,%f, %f,%f, %f,%f, %f,%f"),
			GetCurrentPositionRelativeToRollingAverage(Original, 45).Size(),
			GetStandardDeviation(Original,45),
			GetCurrentPositionRelativeToRollingAverage(Interpolated, 45).Size(),
			GetStandardDeviation(Original, 45),
			GetCurrentPositionRelativeToRollingAverage(Warped, 45).Size(),
			GetStandardDeviation(Original, 45),
			GetCurrentPositionRelativeToRollingAverage(SIXDOF, 45).Size(),
			GetStandardDeviation(SIXDOF,45)); */
	}
}

float FLeapFrameTransformStats::GetPositionDeltaMagnitude(FrameDeltaType frameType)
{
	switch (frameType)
	{
	case OriginalToSelf:
		return (GetCurrentSample().OriginalFrame.Position_Left - GetPreviousSample().OriginalFrame.Position_Left).Size();
		break;
	case InterpolatedToSelf:
		return (GetCurrentSample().InterpolatedFrame.Position_Left - GetPreviousSample().InterpolatedFrame.Position_Left).Size();
		break;
	case WarpedToSelf:
		return (GetCurrentSample().TimeWarpTranslatedFrame.Position_Left - GetPreviousSample().TimeWarpTranslatedFrame.Position_Left).Size();
		break;

	case InterpolatedToOriginal:
		return 
			   (GetCurrentSample().InterpolatedFrame.Position_Left - GetCurrentSample().OriginalFrame.Position_Left).Size() -
			   (GetPreviousSample().InterpolatedFrame.Position_Left - GetPreviousSample().OriginalFrame.Position_Left).Size();

	case WarpedToInterpolated:
		return (GetCurrentSample().TimeWarpTranslatedFrame.Position_Left - GetCurrentSample().InterpolatedFrame.Position_Left).Size() -
			   (GetPreviousSample().TimeWarpTranslatedFrame.Position_Left - GetPreviousSample().InterpolatedFrame.Position_Left).Size();

	case WarpedToOriginal:
		return (GetCurrentSample().TimeWarpTranslatedFrame.Position_Left - GetCurrentSample().OriginalFrame.Position_Left).Size() -
			(GetPreviousSample().TimeWarpTranslatedFrame.Position_Left - GetPreviousSample().OriginalFrame.Position_Left).Size();

	default:

		return 0.0;
		break;
	}
}

FVector FLeapFrameTransformStats::GetCurrentPositionRelativeToRollingAverage(FrameType frameType, int32 sampleSize)
{
	FVector rollingAverage = GetRollingAverage(frameType, sampleSize);

	switch (frameType)
	{
	case Original:
		return GetCurrentSample().OriginalFrame.Position_Left - rollingAverage;
		break;
	case Interpolated:
		return GetCurrentSample().InterpolatedFrame.Position_Left - rollingAverage;
		break;
	case Warped:
		return GetCurrentSample().JointPositionTimeWarpCorrected; // TimeWarpTranslatedFrame.Position_Left - rollingAverage;
		break;
	case SIXDOF:
		return GetCurrentSample().JointPositionHeadPoseCorrected; // SixDOF.Position_Left - rollingAverage;
		break;
	default:
		break;
	}

	return FVector();
}

float FLeapFrameTransformStats::GetDelta(FrameType frameType)
{
	return GetDelta(frameType, CurrentIndex);
}


float FLeapFrameTransformStats::GetDelta(FrameType frameType, int index)
{
	FVector sampleNow, samplePrevious;
	
	switch (frameType)
	{
	case Original:
		sampleNow = GetSample(index).OriginalFrame.Position_Left;
		samplePrevious = GetSample(index - 1).OriginalFrame.Position_Left;
		break;

	case Interpolated:
		sampleNow = GetSample(index).InterpolatedFrame.Position_Left;
		samplePrevious = GetSample(index - 1).InterpolatedFrame.Position_Left;
		break;

	case Warped:
		sampleNow = GetSample(index).JointPositionTimeWarpCorrected; // TimeWarpTranslatedFrame.Position_Left;
		samplePrevious = GetSample(index - 1).JointPositionTimeWarpCorrected; // TimeWarpTranslatedFrame.Position_Left;
		break;

	case SIXDOF:
		sampleNow = GetSample(index).JointPositionHeadPoseCorrected; // SixDOF.Position_Left;
		samplePrevious = GetSample(index - 1).JointPositionHeadPoseCorrected; // SixDOF.Position_Left;
		break;

	default:
		break;
	}
	 
	return (sampleNow - samplePrevious).Size();
}

FColor FLeapFrameTransformStats::ColourBasedOnDelta(float deltaAsPercentOfMaxDelta)
{
	if (deltaAsPercentOfMaxDelta < 50)
	{
		return FColor::Green;
	}
	else if (deltaAsPercentOfMaxDelta < 75)
	{
		return FColor::Yellow;
	}
	else
	{
		return FColor::Red;
	}

	return FColor::White;
}

StatsRunOptions FLeapFrameTransformStats::StartNextStatsRun()
{
	if (StatsSweepIndex+1 < StatsSweepRunDetails.Num())
	{
		StatsSweepIndex++;
		IsCurrentStatsRunActive = true;
		IsStatsSweepComplete = false;
		CurrentIndex = 0;

		UE_LOG(LeapTransformStats, Log, TEXT("Starting stats run for %s"), *StatsSweepRunDetails[StatsSweepIndex].RunName);
		return StatsSweepRunDetails[StatsSweepIndex];
	}
	else
	{
		UE_LOG(LeapTransformStats, Log, TEXT("Stats sweep complete"));
		StatsSweepIndex = 0;
		IsCurrentStatsRunActive = false;
		IsStatsSweepComplete = true;
		StatsRunOptions default;
		default.RunName = "Default";
		return default;
	}
}

float FLeapFrameTransformStats::GetDeltaAsPercent(FrameType frameType, int32 sampleSize)
{
	FVector sampleNow, samplePrevious;
	FMath hlpr;
	double maxDelta = 0;

	for (int i = CurrentIndex; i > CurrentIndex - sampleSize; i--)
	{
		switch (frameType)
		{
		case Original:
			sampleNow = GetSample(i).OriginalFrame.Position_Left;
			samplePrevious = GetSample(i-1).OriginalFrame.Position_Left;
			break;

		case Interpolated:
			sampleNow = GetSample(i).InterpolatedFrame.Position_Left;
			samplePrevious = GetSample(i-1).InterpolatedFrame.Position_Left;
			break;

		case Warped:
			sampleNow = GetSample(i).JointPositionTimeWarpCorrected;// TimeWarpTranslatedFrame.Position_Left;
			samplePrevious = GetSample(i - 1).JointPositionTimeWarpCorrected; // TimeWarpTranslatedFrame.Position_Left;
			break;

		case SIXDOF:
			sampleNow = GetSample(i).JointPositionHeadPoseCorrected; // SixDOF.Position_Left;
			samplePrevious = GetSample(i - 1).JointPositionHeadPoseCorrected; // SixDOF.Position_Left;
			break;

		default:
			break;
		}

		if (hlpr.Abs((sampleNow - samplePrevious).Size()) > maxDelta)
		{
			maxDelta = hlpr.Abs((sampleNow - samplePrevious).Size());
		}
	}

	return (GetDelta(frameType) / maxDelta) * 100;
}


float FLeapFrameTransformStats::GetStandardDeviation(FrameType frameType, int32 sampleSize)
{
	FVector rollingAverage = GetRollingAverage(frameType, sampleSize);
	FVector samplePosition;
	FMath hlpr;

	double sum = 0;

	for (int i = CurrentIndex; i > CurrentIndex - sampleSize; i--)
	{
		switch (frameType)
		{
		case Original:
			samplePosition = GetSample(i).OriginalFrame.Position_Left;
			break;

		case Interpolated:
			samplePosition = GetSample(i).InterpolatedFrame.Position_Left;
			break;

		case Warped:
			samplePosition = GetSample(i).JointPositionTimeWarpCorrected;// TimeWarpTranslatedFrame.Position_Left;
			break;

		case SIXDOF:
			samplePosition = GetSample(i).JointPositionHeadPoseCorrected; // SixDOF.Position_Left;
			break;

		default:
			break;
		}

		sum += hlpr.Square((samplePosition - rollingAverage).Size());
	}

	return hlpr.Sqrt(sum / sampleSize);
}

FVector FLeapFrameTransformStats::GetRollingAverage(FrameType frameType, int32 sampleSize)
{
	double x = 0, y = 0, z = 0;
	FVector samplePosition;

	for (int i = CurrentIndex; i > CurrentIndex - sampleSize; i--)
	{
		switch (frameType)
		{
		case Original:
			samplePosition = GetSample(i).OriginalFrame.Position_Left;
			break;

		case Interpolated:
			samplePosition = GetSample(i).InterpolatedFrame.Position_Left;
			break;

		case Warped:
			samplePosition = GetSample(i).JointPositionTimeWarpCorrected; //.TimeWarpTranslatedFrame.Position_Left;
			break;

		case SIXDOF:
			samplePosition = GetSample(i).JointPositionHeadPoseCorrected; // SixDOF.Position_Left;
			break;

		default:
			break;
		}

		x += samplePosition.X;
		y += samplePosition.Y;
		z += samplePosition.Z;
	}

	return FVector((float)x / (float) sampleSize, (float)y / (float)sampleSize, (float)z / (float)sampleSize);
}

void FLeapFrameTransformStats::CalculateSummaryStats(SummaryStatsType forStage)
{
	FMath hlpr;
	float maxDeviation = 0.0;

	UE_LOG(LeapTransformStats, Log, TEXT("Summary Stats for %s"), *StatsSweepRunDetails[StatsSweepIndex].RunName);

	float delta = 0;

	FVector averagePosition = GetAveragePosition(forStage);
	UE_LOG(LeapTransformStats, Log, TEXT("Average position was %s"), *averagePosition.ToString());


	for (int i = 1; i < MAX_FRAME_DATA_ENTRIES; i++)
	{	
		delta = GetSummaryStatsDelta(forStage, i, averagePosition);

		if (delta > maxDeviation)
		{
			maxDeviation = delta;
		}
	}

	const float binSize = 0.002;
	int bin = 0;

	if (maxDeviation > binSize)
	{
		UE_LOG(LeapTransformStats, Log, TEXT("Calculating histogram"));

		int nBins = hlpr.CeilToInt(maxDeviation / binSize) + 1;

		if (nBins > 10000)
		{
			nBins = 10000;
		}

		TArray<int32> histogram;
		histogram.Init(0, nBins);

		for (int i = 0; i < MAX_FRAME_DATA_ENTRIES; i++)
		{
			delta = GetSummaryStatsDelta(forStage, i, averagePosition);

			bin = hlpr.FloorToInt((delta / binSize));

			if (bin >=0 && bin < histogram.Num()-1)
			{
				histogram[bin]++;
			}
			else
			{
				if (histogram.Num() > 0)
				{
					histogram[histogram.Num() - 1]++;
				}
			}
		}

		float binStart = 0;
		for (int binIndex = 0; binIndex < nBins; binIndex++)
		{
			UE_LOG(LeapTransformStats, Log, TEXT("%f, %f, :, %d"), binStart, binStart + binSize, histogram[binIndex]);
			binStart += binSize;
		}
	}
	else
	{
		UE_LOG(LeapTransformStats, Log, TEXT("No data for %s"), *StatsSweepRunDetails[StatsSweepIndex].RunName);
	}

}

float FLeapFrameTransformStats::GetSummaryStatsDelta(SummaryStatsType forStage, int32 index, FVector averagePosition)
{
	switch (forStage)
	{
	case InterpolationStage:

		if (UseInterpolation == false)
		{
			return GetDelta(Original, index);
		}
		else
		{
			return GetDelta(Interpolated, index);
		}

		break;

	case HeadPoseCorrectionStage:
		return (GetSample(index).JointPositionHeadPoseCorrected - averagePosition).Size();
		break;

	case TimeWarpCorrectionStage:
		return (GetSample(index).JointPositionTimeWarpCorrected - averagePosition).Size();
		break;

	default:
		break;
	}

	return 0;
}

FVector  FLeapFrameTransformStats::GetAveragePosition(SummaryStatsType forStage)
{
	switch (forStage)
	{
	case InterpolationStage:

		if (UseInterpolation == false)
		{
			return GetRollingAverage(Original, MAX_FRAME_DATA_ENTRIES);
		}
		else
		{
			return GetRollingAverage(Interpolated, MAX_FRAME_DATA_ENTRIES);
		}
		break;

	case HeadPoseCorrectionStage:
		return GetRollingAverage(FrameType::SIXDOF, MAX_FRAME_DATA_ENTRIES);
		break;

	case TimeWarpCorrectionStage:
		return GetRollingAverage(FrameType::Warped, MAX_FRAME_DATA_ENTRIES);
		break;

	default:
		return FVector(0, 0, 0);
		break;
	}
}

StatsRunOptions FLeapFrameTransformStats::StartStatsSweep(bool logLiveStatistics)
{
	if (IsCurrentStatsRunActive == false && IsStatsSweepComplete == true)
	{
		IsCurrentStatsRunActive = true;
		IsStatsSweepComplete = false;
		logLiveStats = logLiveStatistics;
		CurrentIndex = 0;
		StatsSweepIndex = 0;
		UE_LOG(LeapTransformStats, Log, TEXT("======================================= Collecting Stats ================================================="));
	}

	return StatsSweepRunDetails[StatsSweepIndex];
}

LeapFrameTransformSample& FLeapFrameTransformStats::GetPreviousSample()
{
	if (CurrentIndex < 1 && haveData)
	{
		return Samples[MAX_FRAME_DATA_ENTRIES - 1];
	}
	else
	{
		return Samples[CurrentIndex - 1];
	}
}

LeapFrameTransformSample& FLeapFrameTransformStats::GetSample(int index)
{
	FMath hlpr;
	int index2 = (hlpr.Abs(index) % (MAX_FRAME_DATA_ENTRIES - 1));
	return Samples[index2];
}

