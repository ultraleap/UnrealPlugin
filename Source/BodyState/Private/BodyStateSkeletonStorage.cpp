

#include "BodyStateSkeletonStorage.h"

#include "BodyStateUtility.h"
#include "CoreMinimal.h"
#include "Misc/App.h"
#include "Skeleton/BodyStateSkeleton.h"

FBodyStateSkeletonStorage::FBodyStateSkeletonStorage()
{
	PrivateMergedSkeleton = nullptr;
	DeviceIndex = 0;
	MergingFunctionIndexCount = 0;
}

FBodyStateSkeletonStorage::~FBodyStateSkeletonStorage()
{
	// Allow merge skeleton to be removed
	if (PrivateMergedSkeleton && PrivateMergedSkeleton->IsValidLowLevel())
	{
		PrivateMergedSkeleton->RemoveFromRoot();
		PrivateMergedSkeleton = nullptr;
	}
}

int32 FBodyStateSkeletonStorage::AddDevice(const FBodyStateDevice& InDevice)
{
	// Ensure we have a merged skeleton first
	MergedSkeleton();

	FBodyStateDevice Device = InDevice;
	DeviceIndex++;
	Device.DeviceId = DeviceIndex;

	Device.Skeleton = NewObject<UBodyStateSkeleton>();
	Device.Skeleton->Name = Device.Config.DeviceName;
	Device.Skeleton->SkeletonId = Device.DeviceId;
	Device.Skeleton->TrackingTags = Device.Config.TrackingTags;
	Device.Skeleton->AddToRoot();

	Devices.Add(Device.InputCallbackDelegate, Device);
	DeviceKeyMap.Add(Device.DeviceId, Device.InputCallbackDelegate);

	UE_LOG(BodyStateLog, Log, TEXT("BodyState::DeviceAttached: %s (%d)"), *Device.Config.DeviceName, Devices.Num());
	return Device.DeviceId;
}

bool FBodyStateSkeletonStorage::RemoveDevice(int32 DeviceId)
{
	UE_LOG(BodyStateLog, Log, TEXT("Id: %d"), DeviceId);
	if (DeviceId < 0)
	{
		UE_LOG(BodyStateLog, Log, TEXT("BodyState::RemoveDevice attempted to remove invalid index: (%d)"), DeviceId);
		return false;	 // couldn't find
	}

	bool HasKey = DeviceKeyMap.Contains(DeviceId);

	if (!HasKey)
	{
		UE_LOG(BodyStateLog, Log, TEXT("BodyState::RemoveDevice already removed (%d)"), DeviceId);
		return false;	 // already removed
	}

	IBodyStateInputRawInterface* DelegatePtr = DeviceKeyMap[DeviceId];
	const FBodyStateDevice& Device = Devices[DelegatePtr];
	const FString DeviceName = Device.Config.DeviceName;

	if (Device.Skeleton->IsValidLowLevel())
	{
		Device.Skeleton->RemoveFromRoot();
	}
	DeviceKeyMap.Remove(DeviceId);
	Devices.Remove(DelegatePtr);

	UE_LOG(BodyStateLog, Log, TEXT("BodyState::Device Detached: %s (%d)"), *DeviceName, Devices.Num());

	return true;
}

void FBodyStateSkeletonStorage::RemoveAllDevices()
{
	// We need to make a copy of our devices as remove will change the size of devices
	TArray<FBodyStateDevice> AllDevices;

	Devices.GenerateValueArray(AllDevices);

	for (auto& Device : AllDevices)
	{
		if (Device.InputCallbackDelegate != nullptr)
		{
			RemoveDevice(Device.DeviceId);
		}
	}
}

UBodyStateSkeleton* FBodyStateSkeletonStorage::SkeletonForDevice(int32 DeviceId)
{
	// Return merged/only skeleton
	if (DeviceId == 0)
	{
		return PrivateMergedSkeleton;
	}
	// Return specific skeleton
	else if (!DeviceKeyMap.Contains(DeviceId))
	{
		UE_LOG(BodyStateLog, Warning, TEXT("DeviceID: %d is invalid, returning nullptr skeleton."), DeviceId);
		return nullptr;
	}
	else
	{
		return Devices[DeviceKeyMap[DeviceId]].Skeleton;
	}
}

UBodyStateSkeleton* FBodyStateSkeletonStorage::MergedSkeleton()
{
	// create as needed
	if (!PrivateMergedSkeleton || !PrivateMergedSkeleton->IsValidLowLevel())
	{
		PrivateMergedSkeleton = NewObject<UBodyStateSkeleton>();
		PrivateMergedSkeleton->Name = TEXT("Merged");
		PrivateMergedSkeleton->SkeletonId = 0;
		PrivateMergedSkeleton->AddToRoot();
	}
	return PrivateMergedSkeleton;
}

void FBodyStateSkeletonStorage::UpdateMergeSkeletonData()
{
	double Now = FApp::GetCurrentTime();
	DeltaTime = (Now - LastFrameTime);

	// Basic merge of skeleton data
	MergedSkeleton();

	if (!PrivateMergedSkeleton->bTrackingActive)
	{
		return;
	}

	// Reset our confidence
	PrivateMergedSkeleton->ClearConfidence();
	PrivateMergedSkeleton->TrackingTags.Empty();

	// Merges all skeleton data
	{
		FScopeLock ScopeLock(&PrivateMergedSkeleton->BoneDataLock);
		for (auto& Elem : Devices)
		{
			UBodyStateSkeleton* Skeleton = Elem.Value.Skeleton;
			PrivateMergedSkeleton->MergeFromOtherSkeleton(Skeleton);
		}
	}

	// Dispatch estimator function lambdas which give merge skeleton and expect further updated values
	CallMergingFunctions();

	LastFrameTime = Now;
}

void FBodyStateSkeletonStorage::CallMergingFunctions()
{
	// Call all merging functions on our private merged skeleton
	for (auto& Pair : MergingFunctions)
	{
		FScopeLock ScopeLock(&PrivateMergedSkeleton->BoneDataLock);
		Pair.Value(PrivateMergedSkeleton, DeltaTime);
	}
}

int32 FBodyStateSkeletonStorage::AddMergingFunction(TFunction<void(UBodyStateSkeleton*, float)> InFunction)
{
	MergingFunctions.Add(MergingFunctionIndexCount, InFunction);
	MergingFunctionIndexCount++;
	return MergingFunctionIndexCount - 1;
}

bool FBodyStateSkeletonStorage::RemoveMergingFunction(int32 MergingFunctionId)
{
	int32 ValueCount = MergingFunctions.Remove(MergingFunctionId);
	return ValueCount > 0;
}

void FBodyStateSkeletonStorage::ClearMergingFunctions()
{
	MergingFunctions.Empty();
	MergingFunctionIndexCount = 0;
}

void FBodyStateSkeletonStorage::CallFunctionOnDevices(TFunction<void(const FBodyStateDevice&)> InFunction)
{
	for (auto& Elem : Devices)
	{
		auto& Device = Elem.Value;
		if (Device.InputCallbackDelegate != nullptr)
		{
			InFunction(Device);
		}
	}
}