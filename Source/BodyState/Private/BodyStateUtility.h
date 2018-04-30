#pragma once
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(BodyStateLog, Log, All);

class BODYSTATE_API FBodyStateUtility
{
public:
	//Math Utility
	static FRotator CombineRotators(FRotator A, FRotator B);
	static float AngleBetweenVectors(FVector A, FVector B);

	template<typename T>
	static FString EnumToString(const FString& enumName, const T value);
};
