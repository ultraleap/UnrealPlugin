// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/Package.h"

DECLARE_LOG_CATEGORY_EXTERN(BodyStateLog, Log, All);

class BODYSTATE_API FBodyStateUtility
{
public:
	//Math Utility
	static FRotator CombineRotators(FRotator A, FRotator B);
	static float AngleBetweenVectors(FVector A, FVector B);

	template<typename T>
	static FString EnumToString(const FString& enumName, const T value)
	{
		UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, *enumName);
		return *(pEnum ? pEnum->GetNameStringByIndex(static_cast<uint8>(value)) : "null");
	}
};
