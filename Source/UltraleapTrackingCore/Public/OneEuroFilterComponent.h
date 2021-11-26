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
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void Init(const float InMinCutoff, const float InCutoffSlope, const float InDeltaCutoff);

	/** Smooth vector */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	FVector Filter(const FVector& InRaw, const float InDeltaTime);

	/** Set the minimum cutoff */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void SetMinCutoff(const float InMinCutoff);

	/** Set the cutoff slope */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void SetCutoffSlope(const float InCutoffSlope);

	/** Set the delta slope */
	UFUNCTION(BlueprintCallable, Category = "Ultraleap IE")
	void SetDeltaCutoff(const float InDeltaCutoff);

private:
	const FVector CalculateCutoff(const FVector& InValue);
	const FVector CalculateAlpha(const FVector& InCutoff, const float InDeltaTime) const;
	const float CalculateAlpha(const float InCutoff, const float InDeltaTime) const;

	double MinCutoff;
	double CutoffSlope;
	double DeltaCutoff;
	FLowpassFilter RawFilter;
	FLowpassFilter DeltaFilter;
};
