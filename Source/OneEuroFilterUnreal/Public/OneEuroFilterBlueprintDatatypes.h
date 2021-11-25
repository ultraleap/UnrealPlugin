// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OneEuroFilter.h"

#include "OneEuroFilterBlueprintDatatypes.generated.h"

// Base struct datatype for Blueprints OneEuroFilter variables.
// Unluckily, we cannot use templates with USTRUCTS/UFUNCTIONS/etc
USTRUCT(BlueprintType)
struct FOneEuroFilterSettings
{
	GENERATED_USTRUCT_BODY()
	virtual ~FOneEuroFilterSettings()
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "One Euro Filter Settings")
	float Frequency = 120;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "One Euro Filter Settings")
	float MinCutoff = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "One Euro Filter Settings")
	float Beta = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "One Euro Filter Settings")
	float DCutoff = 1;

	bool bInitialized = false;

	FOneEuroFilterSettings(float InFrequency = 120.0, float InMinCutoff = 1.0, float InBeta = 0.0, float InDCutoff = 1.0)
	{
		Frequency = InFrequency;
		MinCutoff = InMinCutoff;
		Beta = InBeta;
		DCutoff = InDCutoff;
	};

	virtual void SetParameters(float InFrequency = 120.0, float InMinCutoff = 1.0, float InBeta = 0.0, float InDCutoff = 1.0)
	{
		FOneEuroFilterSettings(InFrequency, InMinCutoff, InBeta, InDCutoff);
		RefreshParameters();
	}

	virtual void RefreshParameters()
	{
	}
};

// Float OneEuroFilter to be used in Blueprints
USTRUCT(BlueprintType)
struct FOneEuroFilterFloat : public FOneEuroFilterSettings
{
	GENERATED_USTRUCT_BODY()

	FOneEuroFilterFloat(float InFrequency = 120.0, float InMinCutoff = 1.0, float InBeta = 0.0, float InDCutoff = 1.0);

	void SetValue(float NewValue);

	void GetValue(float& Raw, float& Filtered);

	virtual void RefreshParameters() override;

private:
	float Value;
	float FilteredValue;
	TSharedPtr<OneEuroFilter<float>> TheFilter = nullptr;
};

// FVector OneEuroFilter to be used in Blueprints
USTRUCT(BlueprintType)
struct FOneEuroFilterVector : public FOneEuroFilterSettings
{
	GENERATED_USTRUCT_BODY()

	FOneEuroFilterVector(float InFrequency = 120.0, float InMinCutoff = 1.0, float InBeta = 0.0, float InDCutoff = 1.0);

	void SetValue(FVector NewValue);

	void GetValue(FVector& Raw, FVector& Filtered);

	virtual void RefreshParameters() override;

private:
	FVector Value;
	FVector FilteredValue;
	TSharedPtr<OneEuroFilter<FVector>> TheFilter = nullptr;
};

// FRotator OneEuroFilter to be used in Blueprints
USTRUCT(BlueprintType)
struct FOneEuroFilterRotator : public FOneEuroFilterSettings
{
	GENERATED_USTRUCT_BODY()

	FOneEuroFilterRotator(float InFrequency = 120.0, float InMinCutoff = 1.0, float InBeta = 0.0, float InDCutoff = 1.0);

	void SetValue(FRotator NewValue);

	void GetValue(FRotator& Raw, FRotator& Filtered);

	virtual void RefreshParameters() override;

private:
	FRotator Value;
	FRotator FilteredValue;
	TSharedPtr<OneEuroFilter<FRotator>> TheFilter = nullptr;
};

// FTransform OneEuroFilter to be used in Blueprints
USTRUCT(BlueprintType)
struct FOneEuroFilterTransform : public FOneEuroFilterSettings
{
	GENERATED_USTRUCT_BODY()

	FOneEuroFilterTransform(float InFrequency = 120.0, float InMinCutoff = 1.0, float InBeta = 0.0, float InDCutoff = 1.0);

	void SetValue(FTransform NewValue);

	void GetValue(bool bBypassScale, FTransform& Raw, FTransform& Filtered);

	virtual void RefreshParameters() override;

private:
	FTransform Value;
	FTransform FilteredValue;
	TSharedPtr<OneEuroFilter<FVector>> TheLocationFilter = nullptr;
	TSharedPtr<OneEuroFilter<FQuat>> TheRotationFilter = nullptr;
	TSharedPtr<OneEuroFilter<FVector>> TheScaleFilter = nullptr;
};
