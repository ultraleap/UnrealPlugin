/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/


#include "UltraleapEditorNotifyComponent2.h"


void UUltraleapEditorNotifyComponent2::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport/* = ETeleportType::None*/)
{
	HandleComponentMove(true);
}