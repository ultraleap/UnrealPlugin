/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "LeapUtility.h"

#include "Engine/Engine.h"	  // for GEngine
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "IXRTrackingSystem.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "UltraleapTrackingData.h"
#include "Runtime/Launch/Resources/Version.h"
#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3)
	#include "JsonObjectConverter.h"
#else 
	#include "JsonUtilities/Public/JsonObjectConverter.h"
#endif

DEFINE_LOG_CATEGORY(UltraleapTrackingLog);

// Static vars
#define LEAP_TO_UE_SCALE 0.1f
#define UE_TO_LEAP_SCALE 10.f
#define LEAP_TO_OPENXR_SCALE 0.001	  // (Leap) mm to m (OpenXR)

// Device transforms come in in OpenXR space. OpenXR uses a right handed, y-up coordinate system, Unreal is left handed, z-up
// Unreal units are in mm by default, OpenXR is in m
#define OPENXR_DISTANCE_UNITS_TO_UNREAL_UNITS 100.0f;	 // m -> cm

FQuat FLeapUtility::LeapRotationOffset;

// Todo: use and verify this for all values
float LeapGetWorldScaleFactor()
{
	if (IsInGameThread())
	{
		if (GEngine != nullptr && GEngine->GetWorld() != nullptr && GEngine->GetWorld()->GetWorldSettings() != nullptr)
		{
			return (GEngine->GetWorld()->GetWorldSettings()->WorldToMeters) / 100.f;
		}
	}
	return 1.f;
}

// Single point to handle OpenXR to Unreal conversion
FVector FLeapUtility::ConvertOpenXRVectorToUnrealVector(const FVector& OpenXRVector)
{
	// Unreal - +X forward, +Y right, +z up
	// OpenXR - +X right,   +Y up,    -z forward
	//
	// So OpenXR X -> Unreal Y
	//    OpenXR Y -> Unreal Z
	//    OpenXR Z -> Unreal -X

	// Expects input data in VR Orientation
	return FVector(-OpenXRVector.Z, OpenXRVector.X, OpenXRVector.Y) * OPENXR_DISTANCE_UNITS_TO_UNREAL_UNITS;
}

void FLeapUtility::LogRotation(const FString& Text, const FRotator& Rotation)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("%s %f %f %f"), *Text, Rotation.Yaw, Rotation.Pitch, Rotation.Roll));
	}
}
FRotator FLeapUtility::CombineRotators(FRotator A, FRotator B)
{
	FQuat AQuat = FQuat(A);
	FQuat BQuat = FQuat(B);

	return FRotator(BQuat * AQuat);
}

// Single point to handle leap conversion
FVector FLeapUtility::ConvertLeapVectorToFVector(const LEAP_VECTOR& LeapVector)
{
	// Expects VR Orientation
	return FVector(LeapVector.y, -LeapVector.x, -LeapVector.z);
}

FQuat FLeapUtility::ConvertLeapQuatToFQuat(const LEAP_QUATERNION& Quaternion)
{
	FQuat Quat;

	// it's -Z, X, Y tilted back by 90 degree which is -y,x,z
	Quat.X = -Quaternion.y;
	Quat.Y = Quaternion.x;
	Quat.Z = Quaternion.z;
	Quat.W = Quaternion.w;

	if (Quat.ContainsNaN())
	{
		Quat = FQuat::MakeFromEuler(FVector(0, 0, 0));
		UE_LOG(
			UltraleapTrackingLog, Log, TEXT("FLeapUtility::ConvertLeapQuatToFQuat() Warning - NAN received from tracking device"));
	}
	return Quat * LeapRotationOffset;
}


FVector FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets(const LEAP_VECTOR& LeapVector,
	const FVector& LeapMountTranslationOffset, const FQuat& LeapMountRotationOffset, const bool SuppliedTransformsAreForLeapToOpenXR)
{
	if (FLeapUtility::ContainsNaN(LeapVector))
	{
		return LeapMountRotationOffset.RotateVector(FVector::ForwardVector);
	}
	
	// Rotate our vector to adjust for any global rotation offsets
	if (SuppliedTransformsAreForLeapToOpenXR)
	{	
		FVector OpenXRVector =
			LeapMountRotationOffset.RotateVector(FVector(LeapVector.x, LeapVector.y, LeapVector.z)) * LEAP_TO_OPENXR_SCALE;

		if (OpenXRVector.ContainsNaN())
		{
			OpenXRVector = FVector::ZeroVector;
			UE_LOG(UltraleapTrackingLog, Log,
				TEXT("FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets Warning - NAN received from tracking "
						"device"));
		}

		FVector OffsetOpenXRVector = OpenXRVector + LeapMountTranslationOffset;
		FVector UnrealVector = ConvertOpenXRVectorToUnrealVector(OffsetOpenXRVector);

		return UnrealVector;
	}
	else
	{
		// Scale from mm to cm (ue default)
		FVector ConvertedVector =
			(ConvertLeapVectorToFVector(LeapVector) + LeapMountTranslationOffset) * (LEAP_TO_UE_SCALE * LeapGetWorldScaleFactor());
		if (ConvertedVector.ContainsNaN())
		{
			ConvertedVector = FVector::ZeroVector;
			UE_LOG(UltraleapTrackingLog, Log,
				TEXT("FLeapUtility::ConvertAndScaleLeapVectorToFVectorWithHMDOffsets Warning - NAN received from tracking "
					 "device"));
		}
		return LeapMountRotationOffset.RotateVector(ConvertedVector);
	}
}

FQuat FLeapUtility::ConvertToFQuatWithHMDOffsets(
	LEAP_QUATERNION Quaternion, const FQuat& LeapMountRotationOffset, const bool SuppliedTransformsAreForLeapToOpenXR)
{
	if (SuppliedTransformsAreForLeapToOpenXR)
	{
		FQuat Quat;

		// The first part of this code duplicates the code in ConvertLeapQuatToFQuat - *it's unclear why this swizzling is done in that code*
		
		// It's -Z, X, Y tilted back by 90 degree which is -y,x,z
		Quat.X = -Quaternion.y;
		Quat.Y = Quaternion.x;
		Quat.Z = Quaternion.z;
		Quat.W = Quaternion.w;

		// We then apply our adjusted LeapMountRotationOffset in place of the usual LeapRotationOffset
		// Why this order? Well ...
		// [A] ... the rotation values for a headset - e.g. a Lynx - map onto the default values set in LeapRotationOffset based on this mapping
		/// (Y, X, -Z)
		FQuat AdjustedLeapMountRotationOffset = FQuat(
			FRotator(LeapMountRotationOffset.Euler().Y, LeapMountRotationOffset.Euler().X, -LeapMountRotationOffset.Euler().Z));

		auto OpenXRQuat = Quat * AdjustedLeapMountRotationOffset;

		// Remove this debugging code
		UE_LOG(UltraleapTrackingLog, Log, TEXT("LeapRotationOffset : (%f,%f,%f)"), LeapRotationOffset.Euler().X,
			LeapRotationOffset.Euler().Y, LeapRotationOffset.Euler().Z);

		UE_LOG(UltraleapTrackingLog, Log, TEXT("AdjustedLeapMountRotationOffset : (%f,%f,%f)"),
			AdjustedLeapMountRotationOffset.Euler().X, AdjustedLeapMountRotationOffset.Euler().Y,
			AdjustedLeapMountRotationOffset.Euler().Z);

		// Apply an additional correction - Rotate 90 around red (tilt), then rotate 90 around blue (yaw) to bring the red axis
		// to point down the left index finger, blue axis to point up and green axis to point to the right
		// (where the axes are based on the visualization of the bone orientations in the blueprint)  
		// Rotation order is: green / blue / red
		// *** It's not clear why we needed to do this, but it works (maybe it's to do with the mapping in [A]) (((
		return OpenXRQuat * FQuat(FRotator3d(0, 0, 90)) * FQuat(FRotator3d(0, 90, 0));
	}
	else
	{
		FQuat UEQuat = ConvertLeapQuatToFQuat(Quaternion);
		auto afterMountRotation = LeapMountRotationOffset * UEQuat;
		return afterMountRotation;
	}
}

FMatrix FLeapUtility::SwapLeftHandRuleForRight(const FMatrix& UEMatrix)
{
	FMatrix Matrix = UEMatrix;
	// Convenience method to swap the axis correctly, already in UE format to swap Y instead of leap Z
	FVector InverseVector = -Matrix.GetUnitAxis(EAxis::Y);
	Matrix.SetAxes(NULL, &InverseVector, NULL, NULL);
	return Matrix;
}

LEAP_VECTOR FLeapUtility::ConvertUEToLeap(FVector UEVector)
{
	LEAP_VECTOR vector;
	vector.x = UEVector.Y;
	vector.y = UEVector.Z;
	vector.z = -UEVector.X;
	return vector;
}

LEAP_VECTOR FLeapUtility::ConvertAndScaleUEToLeap(FVector UEVector)
{
	LEAP_VECTOR vector;
	vector.x = UEVector.Y * UE_TO_LEAP_SCALE;
	vector.y = UEVector.Z * UE_TO_LEAP_SCALE;
	vector.z = -UEVector.X * UE_TO_LEAP_SCALE;
	return vector;
}

float FLeapUtility::ScaleLeapFloatToUE(float LeapFloat)
{
	return LeapFloat * LEAP_TO_UE_SCALE;	// mm->cm
}

float FLeapUtility::ScaleUEToLeap(float UEFloat)
{
	return UEFloat * UE_TO_LEAP_SCALE;	  // mm->cm
}
// static, this has to be done during runtime as the static initialiser
// does not work in shipping builds.
void FLeapUtility::InitLeapStatics()
{
	LeapRotationOffset = FQuat(FRotator(90.f, 0.f, 180.f));
}

void FLeapUtility::CleanupConstCharArray(const char** ConstCharArray, int32 Size)
{
	// Assume array is filled dynamically
	if (ConstCharArray!=nullptr)
	{
		for (int i = 0; (i <= Size)&&(ConstCharArray[i] != nullptr); ++i)
		{
			free(const_cast<char*>(ConstCharArray[i]));
		}
		// Free memory for the const char* array
		delete[] ConstCharArray;
		ConstCharArray = nullptr;
	}
}


void FLeapUtility::ConvertFStringArrayToCharArray(const TArray<FString>& FStringArray, const char*** ConstCharArrayPtr)
{
	// Allocate memory for array +1 for the NULL end
	*ConstCharArrayPtr = new const char*[FStringArray.Num() + 1];
	for (int32 i = 0; i < FStringArray.Num(); ++i)
	{
		// Convert FString to ANSI const char array
		auto ConvertedStr = StringCast<ANSICHAR>(*FStringArray[i]);
		const char* ConstCharArray = ConvertedStr.Get();

		// String Duplication: Check that string duplications are done correctly
#if PLATFORM_WINDOWS
		(*ConstCharArrayPtr)[i] = _strdup(ConstCharArray);
#else
		(*ConstCharArrayPtr)[i] = strdup(ConstCharArray);
#endif
	}
}

void FLeapUtility::SetLastArrayElemNull(const char*** ConstCharArrayPtr, int32 LastIdx)
{
	(*ConstCharArrayPtr)[LastIdx] = NULL;
}

FString FLeapUtility::GetAnalyticsData(size_t& Size)
{
	FAnalytics Analytics;
	Analytics.telemetry.app_title = FApp::GetProjectName();

#if WITH_EDITOR
	Analytics.telemetry.app_type = "editor";
#else
	Analytics.telemetry.app_type = "build";
#endif

	Analytics.telemetry.engine_name = "Unreal";
	FString UnrealVersion = FString::FromInt(ENGINE_MAJOR_VERSION);
	UnrealVersion += ".";
	UnrealVersion += FString::FromInt(ENGINE_MINOR_VERSION);
	Analytics.telemetry.engine_version = UnrealVersion;
	Analytics.telemetry.installation_source = "github";
	Analytics.telemetry.plugin_version = FString();


	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("UltraleapTracking"));
	if (Plugin.IsValid())
	{
		const FPluginDescriptor& PluginDescriptor = Plugin->GetDescriptor();
		Analytics.telemetry.plugin_version = PluginDescriptor.VersionName;

		if (Plugin->GetLoadedFrom() == EPluginLoadedFrom::Engine)
		{
			Analytics.telemetry.installation_source = "EpicGamesLauncher";
		}
	}

	FString SerializedJson;
	bool bConverted = FJsonObjectConverter::UStructToFormattedJsonObjectString<TCHAR, TPrettyJsonPrintPolicy>(FAnalytics::StaticStruct(), &Analytics, SerializedJson);
	if (!bConverted)
	{
		UE_LOG(UltraleapTrackingLog, Error, TEXT("FLeapUtility::GetAnalyticsData Failed Json conversion"));
	}
	Size = SerializedJson.Len() + 1;
	return SerializedJson;
}
