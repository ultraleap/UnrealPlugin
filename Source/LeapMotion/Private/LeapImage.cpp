// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LeapImage.h"
#include "LeapAsync.h"

FLeapImage::FLeapImage()
{
	LeftImageTexture = nullptr;
	RightImageTexture = nullptr;
	Reset();
}

bool FLeapImage::HasSameTextureFormat(UTexture2D* TexturePointer, const LEAP_IMAGE& Image)
{
	if (TexturePointer == nullptr)
	{
		return false;
	}
	return  (TexturePointer->IsValidLowLevelFast() &&
		TexturePointer->PlatformData &&
		TexturePointer->PlatformData->SizeX == Image.properties.width &&
		TexturePointer->PlatformData->SizeY == Image.properties.height);
}

UTexture2D* FLeapImage::CreateTextureIfNeeded(UTexture2D* TexturePointer, const LEAP_IMAGE& Image)
{
	if (bIsQuitting)
	{
		return nullptr;
	}

	if (!HasSameTextureFormat(TexturePointer, Image))
	{
		EPixelFormat PixelFormat = PF_G8;
		if (TexturePointer->IsValidLowLevelFast())
		{
			TexturePointer->RemoveFromRoot();
		}
		TexturePointer = UTexture2D::CreateTransient(Image.properties.width, Image.properties.height, PixelFormat);
		TexturePointer->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
		TexturePointer->UpdateResource();
		TexturePointer->AddToRoot();
		UpdateTextureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, Image.properties.width, Image.properties.height);
		return TexturePointer;
	}
	return TexturePointer;
}

void FLeapImage::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{
	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
			bool bFreeData;
			FLeapImage* LeapImagePtr;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;
		RegionData->bFreeData = bFreeData;
		RegionData->LeapImagePtr = this;

		ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
			[RegionData](FRHICommandListImmediate& RHICmdList)
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
					}
				}
				if (RegionData->bFreeData)
				{
					FMemory::Free(RegionData->SrcData);
				}
				RegionData->LeapImagePtr->bRenderDidUpdate= true;
				delete RegionData;
			});//End Enqueue
	}
}

void FLeapImage::UpdateTextureRegions(UTexture2D* Texture, const LEAP_IMAGE& Image, uint8* SrcData)
{
	UpdateTextureRegions(Texture, 0, 1, &UpdateTextureRegion, Image.properties.width, Image.properties.bpp, SrcData, true);
}

void FLeapImage::UpdateTextureOnGameThread(UTexture2D* Texture, uint8* SrcData, const int32 BufferLength)
{
	uint8* MipData = static_cast<uint8*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	memcpy(MipData, SrcData, BufferLength);

	Texture->PlatformData->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();
}

void FLeapImage::OnImage(const LEAP_IMAGE_EVENT *ImageEvent)
{
	//Don't schedule more events if we've received quitting signal or we haven't updated the last render
	if (bIsQuitting || !bRenderDidUpdate)
	{
		return;
	}

	const LEAP_IMAGE& LeftLeapImage = ImageEvent->image[0];
	const LEAP_IMAGE& RightLeapImage = ImageEvent->image[1];
	const int32 BufferSize = LeftLeapImage.properties.height * LeftLeapImage.properties.width * LeftLeapImage.properties.bpp;	//same size for both
	if (LeftImageBuffer.Num() != BufferSize)
	{
		LeftImageBuffer.SetNumUninitialized(BufferSize);
	}
	if (RightImageBuffer.Num() != BufferSize)
	{
		RightImageBuffer.SetNumUninitialized(BufferSize);
	}

	//Texture allocation
	LeftImageTexture = CreateTextureIfNeeded(LeftImageTexture, LeftLeapImage);
	if (LeftImageTexture)
	{
		//memcopy to a safe resource area
		uint8* SrcPtr = (uint8*)LeftLeapImage.data + LeftLeapImage.offset;
		FMemory::Memcpy(LeftImageBuffer.GetData(), SrcPtr, BufferSize);
	}

	RightImageTexture = CreateTextureIfNeeded(RightImageTexture, RightLeapImage);
	if (RightImageTexture)
	{
		//memcopy to a safe resource area
		uint8* SrcPtr = (uint8*)RightLeapImage.data + RightLeapImage.offset;
		FMemory::Memcpy(RightImageBuffer.GetData(), SrcPtr, BufferSize);
	}

	bRenderDidUpdate = false;

	if (OnImageCallback.IsBound())
	{
		if (LeftImageTexture && RightImageTexture)
		{
			FLeapAsync::RunShortLambdaOnGameThread([&, BufferSize]
			{
				if (bIsQuitting)
				{
					return;
				}
				//This is sufficient for now since leap images are small
				UpdateTextureOnGameThread(LeftImageTexture, LeftImageBuffer.GetData(), BufferSize);
				UpdateTextureOnGameThread(RightImageTexture, RightImageBuffer.GetData(), BufferSize);
				bRenderDidUpdate = true;

				//Todo: swap to optimized when ready
				//UpdateTextureRegions(LeftImageTexture, LeftLeapImage, LeftImageBuffer.GetData());
				//UpdateTextureRegions(RightImageTexture, RightLeapImage, RightImageBuffer.GetData());
				
				OnImageCallback.Broadcast(LeftImageTexture, RightImageTexture);
			});
		}
	}
}

void FLeapImage::CleanupImageData()
{
	if (LeftImageTexture != nullptr && LeftImageTexture->IsValidLowLevelFast())
	{
		LeftImageTexture->RemoveFromRoot();
		RightImageTexture = nullptr;
	}
	if (RightImageTexture != nullptr && RightImageTexture->IsValidLowLevelFast())
	{
		RightImageTexture->RemoveFromRoot();
		RightImageTexture = nullptr;
	}
	LeftImageBuffer.Empty();
	RightImageBuffer.Empty();
	bIsQuitting = true;
}

void FLeapImage::Reset()
{
	CleanupImageData();
	bIsQuitting = false;
	bRenderDidUpdate = true;
}
