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

#include "BodyStateUtility.h"

DEFINE_LOG_CATEGORY(BodyStateLog);

FRotator FBodyStateUtility::CombineRotators(FRotator A, FRotator B)
{
	FQuat AQuat = FQuat(A);
	FQuat BQuat = FQuat(B);

	return FRotator(BQuat * AQuat);
}

float FBodyStateUtility::AngleBetweenVectors(FVector A, FVector B)
{
	float dotAB = FVector::DotProduct(A, B);
	float bottom = (A.Size() * B.Size());
	if (bottom != 0)
	{
		return FMath::Acos(dotAB / bottom);
	}
	else
	{
		return 0.f;
	}
}
