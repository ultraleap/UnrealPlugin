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

#include "Components/ActorComponent.h"
#include "Skeleton/BodyStateSkeleton.h"

#include "BodyStateEstimatorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBodyStateSkeletonSignature, UBodyStateSkeleton*, Skeleton);

/**
 * Offers an estimation event with a skeleton which will be applied to the merged bodystate.
 * A good place to experiment with derived bodystate bones
 */
UCLASS(ClassGroup = "BodyState", meta = (BlueprintSpawnableComponent))
class BODYSTATE_API UBodyStateEstimatorComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:
	/** Called when the skeleton should be updated before it propagates to all listeners */
	UPROPERTY(BlueprintAssignable, Category = "BodyState Estimation Events")
	FBodyStateSkeletonSignature OnUpdateSkeletonEstimation;

protected:
	/** Override this function inside your initialize component and it will get called correctly in the merging chain */
	TFunction<void(UBodyStateSkeleton*, float)> MergingFunction;
	int32 MergingFunctionId;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

private:
	TFunction<void(UBodyStateSkeleton*, float)> WrapperMergingFunction;
};