// /****************************************************************************** * Copyright (C) Ultraleap, Ltd. 2011-2021. * *
// * * Use subject to the terms of the Apache License 2.0 available at            * * http://www.apache.org/licenses/LICENSE-2.0, or
// another agreement           * * between Ultraleap and you, your company or other organization.             *
// ******************************************************************************/

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "OneEuroFilterComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ULTRALEAPTRACKING_API UOneEuroFilterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UOneEuroFilterComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
private:
	class FLowpassFilter
	{
	public:
		/** Default constructor */
		FLowpassFilter();

		/** Calculate */
		FVector Filter(const FVector& InValue, const FVector& InAlpha);

		/** If the filter was not executed yet */
		bool IsFirstTime() const;

		/** Get the previous filtered value */
		FVector GetPrevious() const;

	private:
		/** The previous filtered value */
		FVector Previous;

		/** If this is the first time doing a filter */
		bool bFirstTime;
	};

public:
	void Init(const double InMinCutoff, const double InCutoffSlope, const double InDeltaCutoff);

	/** Smooth vector */
	FVector Filter(const FVector& InRaw, const double InDeltaTime);

	/** Set the minimum cutoff */
	void SetMinCutoff(const double InMinCutoff);

	/** Set the cutoff slope */
	void SetCutoffSlope(const double InCutoffSlope);

	/** Set the delta slope */
	void SetDeltaCutoff(const double InDeltaCutoff);

private:
	const FVector CalculateCutoff(const FVector& InValue);
	const FVector CalculateAlpha(const FVector& InCutoff, const double InDeltaTime) const;
	const float CalculateAlpha(const float InCutoff, const double InDeltaTime) const;

	double MinCutoff;
	double CutoffSlope;
	double DeltaCutoff;
	FLowpassFilter RawFilter;
	FLowpassFilter DeltaFilter;
};
