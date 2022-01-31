// /****************************************************************************** * Copyright (C) Ultraleap, Ltd. 2011-2021. * *
// * * Use subject to the terms of the Apache License 2.0 available at            * * http://www.apache.org/licenses/LICENSE-2.0, or
// another agreement           * * between Ultraleap and you, your company or other organization.             *
// ******************************************************************************/

#include "OneEuroFilterComponent.h"

/************************************************************************/
// 1 Euro filter smoothing algorithm									
// http://cristal.univ-lille.fr/~casiez/1euro/
//
// This is a port of the Epic implementation used for editor laser pointers
/************************************************************************/

// Sets default values for this component's properties
UOneEuroFilterComponent::UOneEuroFilterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UOneEuroFilterComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

UOneEuroFilterComponent::FLowpassFilter::FLowpassFilter() : Previous(FVector::ZeroVector), bFirstTime(true)
{
}

FVector UOneEuroFilterComponent::FLowpassFilter::Filter(const FVector& InValue, const FVector& InAlpha)
{
	FVector Result = InValue;
	if (!bFirstTime)
	{
		for (int i = 0; i < 3; i++)
		{
			Result[i] = InAlpha[i] * InValue[i] + (1 - InAlpha[i]) * Previous[i];
		}
	}

	bFirstTime = false;
	Previous = Result;
	return Result;
}

bool UOneEuroFilterComponent::FLowpassFilter::IsFirstTime() const
{
	return bFirstTime;
}

FVector UOneEuroFilterComponent::FLowpassFilter::GetPrevious() const
{
	return Previous;
}

void UOneEuroFilterComponent::Init(const float InMinCutoff, const float InCutoffSlope, const float InDeltaCutoff)
{
	MinCutoff = InMinCutoff;
	CutoffSlope = InCutoffSlope;
	DeltaCutoff = InDeltaCutoff;
}

FVector UOneEuroFilterComponent::Filter(const FVector& InRaw, const float InDeltaTime)
{
	// Calculate the delta, if this is the first time then there is no delta
	const FVector Delta = RawFilter.IsFirstTime() == true ? FVector::ZeroVector : (InRaw - RawFilter.GetPrevious()) * InDeltaTime;

	// Filter the delta to get the estimated
	const FVector Estimated = DeltaFilter.Filter(Delta, FVector(CalculateAlpha(DeltaCutoff, InDeltaTime)));

	// Use the estimated to calculate the cutoff
	const FVector Cutoff = CalculateCutoff(Estimated);

	// Filter passed value
	return RawFilter.Filter(InRaw, CalculateAlpha(Cutoff, InDeltaTime));
}

void UOneEuroFilterComponent::SetMinCutoff(const float InMinCutoff)
{
	MinCutoff = InMinCutoff;
}

void UOneEuroFilterComponent::SetCutoffSlope(const float InCutoffSlope)
{
	CutoffSlope = InCutoffSlope;
}

void UOneEuroFilterComponent::SetDeltaCutoff(const float InDeltaCutoff)
{
	DeltaCutoff = InDeltaCutoff;
}

const FVector UOneEuroFilterComponent::CalculateCutoff(const FVector& InValue)
{
	FVector Result;
	for (int i = 0; i < 3; i++)
	{
		Result[i] = MinCutoff + CutoffSlope * FMath::Abs(InValue[i]);
	}
	return Result;
}

const FVector UOneEuroFilterComponent::CalculateAlpha(const FVector& InCutoff, const float InDeltaTime) const
{
	FVector Result;
	for (int i = 0; i < 3; i++)
	{
		Result[i] = CalculateAlpha(InCutoff[i], InDeltaTime);
	}
	return Result;
}

const float UOneEuroFilterComponent::CalculateAlpha(const float InCutoff, const float InDeltaTime) const
{
	const float tau = 1.0 / (2 * PI * InCutoff);
	return 1.0 / (1.0 + tau / InDeltaTime);
}