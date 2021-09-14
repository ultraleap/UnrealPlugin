// Copyright Epic Games, Inc. All Rights Reserved.

#include "LeapVersionSwitcher.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/MinWindows.h"
#endif
#define LOCTEXT_NAMESPACE "FLeapVersionSwitcherModule"

DEFINE_LOG_CATEGORY(LeapVersionSwitcherLog)

void FLeapVersionSwitcherModule::StartupModule()
{
	FlipLeapDLLBasedOnServiceVersion();
}

void FLeapVersionSwitcherModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
#if PLATFORM_WINDOWS

bool GetExecutablePathForService(const FString& ServiceName, FString& PathRet)
{
	FString RegistryPath = FString("SYSTEM\\CurrentControlSet\\Services\\") + ServiceName;
	FString ValueName("ImagePath");

	HKEY hKey;
	LONG Result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, *RegistryPath, 0, KEY_READ, &hKey);
	if (Result != 0)
	{
		return false;
	}
	TCHAR Buffer[MAX_PATH];
	DWORD BufferSize = sizeof(Buffer);
	HRESULT hResult = RegQueryValueEx(hKey, *ValueName, 0, nullptr, reinterpret_cast<LPBYTE>(Buffer), &BufferSize);
	if (hResult != 0)
	{
		return false;
	}
	PathRet = FPaths::GetPath(Buffer) + "\\" + FPaths::GetBaseFilename(Buffer) + ".exe";

	return true;
}
#endif	  // PLATFORM_WINDOWS

#define LEAP_GEN2_MAJOR_VERSION 5
#define LEAP_GEN2_MINOR_VERSION 1
bool FLeapVersionSwitcherModule::IsLeapServiceVersionGemini()
{
#if PLATFORM_WINDOWS
	FString Path;
	bool ServiceFound = false;
	if (!GetExecutablePathForService(FString("UltraleapTracking"), Path))
	{
		ServiceFound = GetExecutablePathForService(FString("LeapService"), Path);
	}
	else
	{
		ServiceFound = true;
	}
	if (ServiceFound)
	{
		uint64 FileVersion = FPlatformMisc::GetFileVersion(Path);

		// each word is one point of the version
		uint32 VersionMajor = FileVersion >> 32;
		uint32 VersionMinor = FileVersion & (uint64) 0xFFFFFFFF;

		uint16 VersionMajorMSW = VersionMajor >> 16;
		uint16 VersionMajorLSW = VersionMajor & (uint32) 0xFFFF;

		uint16 VersionMinorMSW = VersionMinor >> 16;
		uint16 VersionMinorLSW = VersionMinor & (uint32) 0xFFFF;
		UE_LOG(LeapVersionSwitcherLog, Log, TEXT("Leap Service Version is %d.%d.%d.%d"), VersionMajorMSW, VersionMajorLSW,
			VersionMinorMSW, VersionMinorLSW);

		// Greater than version 4?
		if (VersionMajorMSW >= LEAP_GEN2_MAJOR_VERSION && VersionMajorLSW >= LEAP_GEN2_MINOR_VERSION)
		{
			UE_LOG(LeapVersionSwitcherLog, Log, TEXT("Leap Service is greater than v5.0 - Switching to Gen2 LeapC.dll"));
			return true;
		}
		else
		{
			UE_LOG(LeapVersionSwitcherLog, Log, TEXT("Leap Service is less than v5.1 - Switching to Gen1 LeapC.dll"));

			return false;
		}
	}
#endif	  // PLATFORM_WINDOWS

	return true;
}
void FLeapVersionSwitcherModule::FlipLeapDLLBasedOnServiceVersion()
{
#if PLATFORM_WINDOWS
	const bool IsGemini = IsLeapServiceVersionGemini();
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("UltraleapTracking"));
	// Load LeapC DLL
	FString LeapCLibraryPath;

	if (Plugin != nullptr)
	{
		FString BaseDir = Plugin->GetBaseDir();
		FString LeapCLibraryDstPath = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64/LeapC.dll"));

		if (IsGemini)
		{
			LeapCLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64/LeapCGemini.dll"));
		}
		else
		{
			LeapCLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64/LeapCOrion.dll"));
		}
		bool Status = IFileManager::Get().Copy(*LeapCLibraryDstPath, *LeapCLibraryPath, true, true) == COPY_OK;

		if (!Status)
		{
			UE_LOG(LeapVersionSwitcherLog, Log, TEXT("Error: LeapC DLL failed to switch/copy LeapC.dll from %s to %s"),
				*LeapCLibraryDstPath, *LeapCLibraryPath);
		}
	}

#endif	  // PLATFORM_WINDOWS
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLeapVersionSwitcherModule, LeapVersionSwitcher)