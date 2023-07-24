// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "InputCoreTypes.h"

/**
 * 
 */

struct FUltraleapKey
{
	static const FKey UltraleapHand_Left_Pinch;
	static const FKey UltraleapHand_Left_Grab;
	static const FKey UltraleapHand_Right_Pinch;
	static const FKey UltraleapHand_Right_Grab;

};

struct FUltraleapKeyNames
{
	typedef FName Type;

	static const FName UltraleapHand_Left_Pinch;
	static const FName UltraleapHand_Left_Grab;
	static const FName UltraleapHand_Right_Pinch;
	static const FName UltraleapHand_Right_Grab;

};

class FUltraleapInput : public IInputDevice
{
public:
	FUltraleapInput();
	~FUltraleapInput();

	static void PreInit();
};
