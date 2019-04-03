// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LeapAsync.h"
#include "Async/Async.h"

TFuture<void> FLeapAsync::RunLambdaOnBackGroundThread(TFunction< void()> InFunction)
{
	return Async(EAsyncExecution::Thread, InFunction);
}

TFuture<void> FLeapAsync::RunLambdaOnBackGroundThreadPool(TFunction< void()> InFunction)
{
	return Async(EAsyncExecution::ThreadPool, InFunction);
}

FGraphEventRef FLeapAsync::RunShortLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}