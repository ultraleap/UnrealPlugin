#pragma once

#include "LeapLambdaRunnable.h"

uint64 FLeapLambdaRunnable::ThreadNumber = 0;

FLeapLambdaRunnable::FLeapLambdaRunnable(TFunction< void()> InFunction)
{
	FunctionPointer = InFunction;
	Finished = false;
	Number = ThreadNumber;
	
	FString threadStatGroup = FString::Printf(TEXT("FLeapLambdaRunnable%d"), ThreadNumber);
	Thread = NULL; 
	Thread = FRunnableThread::Create(this, *threadStatGroup, 0, TPri_Normal); //windows default = 8mb for thread, could specify more
	ThreadNumber++;
}

FLeapLambdaRunnable::~FLeapLambdaRunnable()
{
	if (Thread == NULL)
	{
		delete Thread;
		Thread = NULL;
	}
}

//Init
bool FLeapLambdaRunnable::Init()
{
	return true;
}

//Run
uint32 FLeapLambdaRunnable::Run()
{
	if (FunctionPointer)
	{
		FunctionPointer();
	}
	return 0;
}

//stop - won't work due to how MT lambdas are setup...
void FLeapLambdaRunnable::Stop()
{
	Finished = true;
}

void FLeapLambdaRunnable::Exit()
{
	Finished = true;

	//delete ourselves when we're done
	delete this;
}

void FLeapLambdaRunnable::Kill(bool bShouldWait)
{
	Thread->Kill(bShouldWait);
}

void FLeapLambdaRunnable::WaitForCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

FLeapLambdaRunnable* FLeapLambdaRunnable::RunLambdaOnBackGroundThread(TFunction< void()> InFunction)
{
	FLeapLambdaRunnable* Runnable;
	if (FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FLeapLambdaRunnable(InFunction);
		return Runnable;
	}
	else 
	{
		return nullptr;
	}
}


FGraphEventRef FLeapLambdaRunnable::RunShortLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

void FLeapLambdaRunnable::ShutdownThreads()
{
}
