#pragma once

/*
Long duration lambda wrapper, which are generally not supported by the taskgraph system. New thread per lambda and they will auto-delete upon
completion.
*/
class LEAPMOTION_API FLeapLambdaRunnable : public FRunnable
{
private:
	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;
	uint64 Number;

	//Lambda function pointer
	TFunction< void()> FunctionPointer;

	/** Use this thread-safe boolean to allow early exits for your threads */
	FThreadSafeBool Finished;

	//static TArray<FLeapLambdaRunnable*> Runnables;
	static uint64 ThreadNumber;

public:
	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FLeapLambdaRunnable(TFunction< void()> InFunction);
	virtual ~FLeapLambdaRunnable();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit() override;
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void WaitForCompletion();	//blocking call that waits until done.
	void Kill(bool bShouldWait = false);	//optional forcible kill

	/*
	Runs the passed lambda on the background thread, new thread per call, auto-deletes on completion
	*/
	static FLeapLambdaRunnable* RunLambdaOnBackGroundThread(TFunction< void()> InFunction);

	/*
	Runs a short lambda on the game thread via task graph system
	*/
	static FGraphEventRef RunShortLambdaOnGameThread(TFunction< void()> InFunction);
	

	static void ShutdownThreads();
};