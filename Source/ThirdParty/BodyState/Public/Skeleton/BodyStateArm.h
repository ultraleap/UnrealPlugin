/*************************************************************************************************************************************
 *The MIT License(MIT)
 *
 *Copyright(c) 2016 Jan Kaniewski(Getnamo)
 *Modified work Copyright(C) 2019 - 2021 Ultraleap, Inc.
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 *files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
 *merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions :
 *
 *The above copyright notice and this permission notice shall be included in all copies or
 *substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 *FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *************************************************************************************************************************************/

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

	// Note thumbs don't have this bone
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

	// Order should be: Thumb->Pinky
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
