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

#include "BodyStateBPLibrary.h"

#include "Animation/AnimInstance.h"
#include "BodyStateUtility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FBodyState.h"
#include "Runtime/Launch/Resources/Version.h"

TArray<IBodyStateDeviceChangeListener*> UBodyStateBPLibrary::DeviceChangeListeners;

UBodyStateBPLibrary::UBodyStateBPLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

int32 UBodyStateBPLibrary::AttachDevice(
	const FBodyStateDeviceConfig& Configuration, TScriptInterface<IBodyStateInputInterface> InputCallbackDelegate)
{
	UE_LOG(BodyStateLog, Warning, TEXT("Not implemented yet, use raw for now."));
	return -1;
}

int32 UBodyStateBPLibrary::AttachDeviceNative(
	const FBodyStateDeviceConfig& Configuration, IBodyStateInputRawInterface* InputCallbackDelegate)
{
	
	int32 DeviceID = IBodyState::Get().AttachDevice(Configuration, InputCallbackDelegate);
	for (auto DeviceListener : DeviceChangeListeners)
	{
		DeviceListener->OnDeviceAdded(Configuration.DeviceSerial, DeviceID);
	}
	return DeviceID;
}

bool UBodyStateBPLibrary::DetachDevice(int32 DeviceID)
{
	if (IBodyState::IsAvailable())
	{
		bool Ret = IBodyState::Get().DetachDevice(DeviceID);

		for (auto DeviceListener : DeviceChangeListeners)
		{
			DeviceListener->OnDeviceRemoved(DeviceID);
		}
		return Ret;
	}
	else
	{
		return false;
	}
}
void UBodyStateBPLibrary::SetupGlobalDeviceManager(IBodyStateDeviceManagerRawInterface* CallbackInterface)
{
	if (IBodyState::IsAvailable())
	{
		IBodyState::Get().SetupGlobalDeviceManager(CallbackInterface);
	}
}
void UBodyStateBPLibrary::OnDefaultDeviceChanged()
{
	for (auto DeviceListener : DeviceChangeListeners)
	{
		DeviceListener->OnDefaultDeviceChanged();
	}
}
UBodyStateSkeleton* UBodyStateBPLibrary::SkeletonForDevice(UObject* WorldContextObject, int32 DeviceID /*= 0*/)
{
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 16
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
#else
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
#endif
	if (World == nullptr)
	{
		UE_LOG(BodyStateLog, Warning, TEXT("SkeletonForDevice:: Wrong world context"))
		return nullptr;
	}

	if (IBodyState::IsAvailable() && (World->IsGameWorld() || World->IsPreviewWorld()))
	{
		return IBodyState::Get().SkeletonForDevice(DeviceID);
	}
	else
	{
		return nullptr;
	}
}
UBodyStateSkeleton* UBodyStateBPLibrary::RequestCombinedDevice(
	UObject* WorldContextObject, const TArray<FString>& DeviceSerials,const EBSDeviceCombinerClass CombinerClass)
{
#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 16
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);
#else
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
#endif
	if (World == nullptr)
	{
		UE_LOG(BodyStateLog, Warning, TEXT("RequestCombinedDevice:: Wrong world context"))
		return nullptr;
	}

	if (IBodyState::IsAvailable() && (World->IsGameWorld() || World->IsPreviewWorld()))
	{
		const int BodyStateDeviceID = IBodyState::Get().RequestCombinedDevice(DeviceSerials,CombinerClass);
		if (BodyStateDeviceID >= 0)
		{
			return SkeletonForDevice(WorldContextObject, BodyStateDeviceID);
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}
int32 UBodyStateBPLibrary::GetDefaultDeviceID()
{
	return IBodyState::Get().GetDefaultDeviceID();
}
FTransform UBodyStateBPLibrary::TransformForBoneNamedInAnimInstance(const FName& Bone, UAnimInstance* Instance)
{
	USkeletalMeshComponent* SkeletalMesh = Instance->GetSkelMeshComponent();
	int32 BoneIndex = SkeletalMesh->GetBoneIndex(Bone);

	if (BoneIndex != INDEX_NONE)
	{
		return SkeletalMesh->GetBoneTransform(BoneIndex);
	}
	else
	{
		return FTransform();
	}
}

bool UBodyStateBPLibrary::AttachMergeAlgorithm(TFunction<void(UBodyStateSkeleton*, float)> InFunction)
{
	IBodyState::Get().AttachMergingFunctionForSkeleton(InFunction);
	return true;
}
bool UBodyStateBPLibrary::GetAvailableDevices(TArray<FString>& DeviceSerials, TArray<int32>& DeviceIDs)
{
	return IBodyState::Get().GetAvailableDevices(DeviceSerials, DeviceIDs);
}
void UBodyStateBPLibrary::AddDeviceChangeListener(IBodyStateDeviceChangeListener* Listener)
{
	DeviceChangeListeners.AddUnique(Listener);
}
void UBodyStateBPLibrary::RemoveDeviceChangeListener(IBodyStateDeviceChangeListener* Listener)
{
	DeviceChangeListeners.Remove(Listener);
}

