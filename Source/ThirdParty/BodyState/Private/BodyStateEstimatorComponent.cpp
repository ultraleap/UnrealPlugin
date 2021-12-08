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

#include "BodyStateEstimatorComponent.h"

#include "Engine/World.h"
#include "IBodyState.h"

UBodyStateEstimatorComponent::UBodyStateEstimatorComponent(const FObjectInitializer& init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;

	MergingFunctionId = -1;
	MergingFunction = nullptr;
}

void UBodyStateEstimatorComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Only allow game world estimators
	if (!GetWorld()->IsGameWorld())
	{
		return;
	}

	// Wrapper function which calls the broadcast function
	WrapperMergingFunction = [&](UBodyStateSkeleton* SkeletonToUpdate, float DeltaTime) {
		if (MergingFunction != nullptr)
		{
			MergingFunction(SkeletonToUpdate, DeltaTime);
		}
		OnUpdateSkeletonEstimation.Broadcast(SkeletonToUpdate);
	};

	// Attach our selves as a bone scene listener. This will auto update our transforms
	MergingFunctionId = IBodyState::Get().AttachMergingFunctionForSkeleton(WrapperMergingFunction);
}

void UBodyStateEstimatorComponent::UninitializeComponent()
{
	// remove ourselves from auto updating transform delegates
	IBodyState::Get().RemoveMergingFunction(MergingFunctionId);

	Super::UninitializeComponent();
}