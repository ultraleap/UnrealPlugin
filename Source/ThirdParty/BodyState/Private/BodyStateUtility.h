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

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/Package.h"

DECLARE_LOG_CATEGORY_EXTERN(BodyStateLog, Log, All);

class BODYSTATE_API FBodyStateUtility
{
public:
	// Math Utility
	static FRotator CombineRotators(FRotator A, FRotator B);
	static float AngleBetweenVectors(FVector A, FVector B);

	template <typename T>
	static FString EnumToString(const FString& enumName, const T value)
	{
		UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, *enumName);
		return *(pEnum ? pEnum->GetNameStringByIndex(static_cast<uint8>(value)) : "null");
	}
};
