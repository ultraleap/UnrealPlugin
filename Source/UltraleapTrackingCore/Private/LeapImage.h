/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#pragma once

#include "Engine/Texture2D.h"
#include "LeapC.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5 || (ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 26)
#include "Engine/Public/Rendering/Texture2DResource.h"
#endif
#include "HAL/ThreadSafeBool.h"
#include "RHI.h"
#include "UltraleapTrackingData.h"

/** Signature with Left/Right Image pair */
DECLARE_MULTICAST_DELEGATE_TwoParams(FLeapImageRawSignature, UTexture2D*, UTexture2D*);

/** Handles checking, conversion, scheduling, and forwarding of image texture data from leap type events */
class FLeapImage
{
public:
	FLeapImage();

	// Callback when an image has been processed and is ready to consume
	FLeapImageRawSignature OnImageCallback;

	bool HasSameTextureFormat(UTexture2D* TexturePointer, const LEAP_IMAGE& Image);
	UTexture2D* CreateTextureIfNeeded(UTexture2D* TexturePointer, const LEAP_IMAGE& Image);
	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions,
		uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);
	void UpdateTextureRegions(UTexture2D* Texture, const LEAP_IMAGE& ImageProps, uint8* SrcData);

	// Backup function, images are fairly small
	void UpdateTextureOnGameThread(UTexture2D* Texture, uint8* SrcData, const int32 BufferLength);
	void OnImage(const LEAP_IMAGE_EVENT* ImageEvent);

	void CleanupImageData();
	void Reset();

private:
	UTexture2D* LeftImageTexture;
	UTexture2D* RightImageTexture;
	FUpdateTextureRegion2D UpdateTextureRegion;
	TArray<uint8> LeftImageBuffer;
	TArray<uint8> RightImageBuffer;
	bool bIsQuitting;
	FThreadSafeBool bRenderDidUpdate;

	// FThreadSafeBool bRenderDidUpdateLeft;
	// FThreadSafeBool bRenderDidUpdateRight;
};