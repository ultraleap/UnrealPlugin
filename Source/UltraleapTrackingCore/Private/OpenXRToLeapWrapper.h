// /****************************************************************************** * Copyright (C) Ultraleap, Ltd. 2011-2021. * *
// * * Use subject to the terms of the Apache License 2.0 available at            * * http://www.apache.org/licenses/LICENSE-2.0, or
// another agreement           * * between Ultraleap and you, your company or other organization.             *
// ******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "LeapWrapper.h"
/**
 *
 */
class FOpenXRToLeapWrapper : FLeapWrapperBase
{
public:
	FOpenXRToLeapWrapper();
	virtual ~FOpenXRToLeapWrapper();

	void UpdateHandState();

private:
	class IHandTracker* HandTracker;
	void InitOpenXRHandTrackingModule();
};
