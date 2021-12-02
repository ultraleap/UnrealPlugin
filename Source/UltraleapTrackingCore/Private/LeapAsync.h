

#pragma once

#include "Async/Async.h"

/*
 *	Convenience wrappers for common thread/task work flow. Run background task on thread, callback via task graph on game thread
 */

class ULTRALEAPTRACKING_API FLeapAsync
{
public:
	/*
	Runs the passed lambda on the background thread, new thread per call
	*/
	static TFuture<void> RunLambdaOnBackGroundThread(TFunction<void()> InFunction);

	/*
	Runs the passed lambda on the background thread pool
	*/
	static TFuture<void> RunLambdaOnBackGroundThreadPool(TFunction<void()> InFunction);

	/*
	Runs a short lambda on the game thread via task graph system
	*/
	static FGraphEventRef RunShortLambdaOnGameThread(TFunction<void()> InFunction);
};