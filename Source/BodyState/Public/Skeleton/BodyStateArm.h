// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Skeleton/BodyStateBone.h"
#include "BodyStateArm.generated.h"

/** Convenience BodyState wrapper around finger bones*/
UCLASS(BlueprintType)
class BODYSTATE_API UBodyStateFinger : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Finger")
	UBodyStateBone* Metacarpal;

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Finger")
	UBodyStateBone* Proximal;

	//Note thumbs don't have this bone
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Finger")
	UBodyStateBone* Intermediate;

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Finger")
	UBodyStateBone* Distal;

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Finger")
	bool bIsExtended;
};

/** Convenience BodyState wrapper around bones relating to the hand*/
UCLASS(BlueprintType)
class BODYSTATE_API UBodyStateHand : public UObject
{
	GENERATED_UCLASS_BODY()

	//Order should be: Thumb->Pinky
	UPROPERTY(BlueprintReadOnly, Category = "BodyState Hand")
	TArray<UBodyStateFinger*> Fingers;

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Hand")
	UBodyStateBone* Wrist;

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Hand")
	UBodyStateBone* Palm;

	UFUNCTION(BlueprintPure, Category = "BodyState Hand")
	UBodyStateFinger* ThumbFinger();

	UFUNCTION(BlueprintPure, Category = "BodyState Hand")
	UBodyStateFinger* IndexFinger();

	UFUNCTION(BlueprintPure, Category = "BodyState Hand")
	UBodyStateFinger* MiddleFinger();

	UFUNCTION(BlueprintPure, Category = "BodyState Hand")
	UBodyStateFinger* RingFinger();

	UFUNCTION(BlueprintPure, Category = "BodyState Hand")
	UBodyStateFinger* PinkyFinger();
};

UCLASS(BlueprintType)
class BODYSTATE_API UBodyStateArm : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Arm")
	UBodyStateHand* Hand;

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Arm")
	UBodyStateBone* LowerArm;

	UPROPERTY(BlueprintReadOnly, Category = "BodyState Arm")
	UBodyStateBone* UpperArm;
};
