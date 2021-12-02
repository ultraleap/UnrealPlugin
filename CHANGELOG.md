Ultraleap Tracking - Change Log
====================

Versions

### Version 4.3.0

- Plugin rebranded to UltraleapTracking from LeapMotion
- Added Interaction Engine
- Added UI input modules
- Added OpenXR hand tracking support

 Known issues:
	* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
	* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.

### Version 3.9.0

* Added Gemini 5.2.0.0 LeapC.dll (Gen2 support)
* Added hand modules 
  * One click auto orientation and new auto bone mapping logic to enable fast import and rigging of hand models.
  * New premapped example hand models - skeleton arms, ghost hands, UE Mannequin and outline hands, with example scenes.
  * Streamlined auto rigging so no nodes are needed in the anim instance's event graph. 

Known issues:
	* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.

### Version 3.7.1

* Adds Support for new "ScreenTop" Tracking Mode: Compatible with 4.9+ Services. This is a feature preview only.
* Tracking mode APIs and notifications now replace Policy flag APIs and notifications
* Fixed: Failure to compile in UE 4.26.x
* Fixed: Pinch and grab events both received on grab (grab now overrides pinch)
* Fixed: Hands tracked/not tracked notifications events being fired even when no change has occurred
* Fixed: Hands not tracked when run in Stand Alone mode from the editor
* Fixed: Intermittent crashes when using body state animation instance based blueprints in editor
* Fixed: Right pinch leap component event not sent
* Removed 32bit Leap DLL and 32bit support.
* Fixed: Live Link hand bone positions are incorrect (spread out in space)
* This version will still re-compile and run with UE 4.25.x

### Version 3.6.0

* Now supports Unreal 4.25. Tracking tested for HMD based hand tracking. However, desktop mode hand tracking still has some bugs/performance issues.
* Improved performance/stability (due to LeapC.dll change / use of the FastTrack Animation System for Hands).
* Non-frame based gesture detection system introduced (this reduces the frequency of grabs/pinches dropping out by giving it a buffer time). The old frame based system is still supported through a UI option.
* Removed some unsupported platforms.
* Fixed a bug that forced desktop mode when vr headset is attached. 
* Fixed freeing of device property before it is read causing invalid serials/devices. 
* Fixed a bug where HMDPositionOffset and HMDRotationOffset settings for a Vive were being applied to all SteamVR headsets (Valve Index, Varjo, etc), if the value were set to zero - UNREAL-64
* Fixed bad event usage example.
* Updated hand colliders to be more accurate (now mesh based).
* Resolved an issue where performance would decrease after a leap was unplugged/replugged.
* Fixed a bug where the HMDPositionOffset was being applied twice, and at different scales. Should be in leap units - mm (UNREAL-63)


---

Copyright © 2012-2020 Ultraleap Ltd. All rights reserved.

Use of the Leap Motion SDK is subject to the terms of the Leap Motion SDK Agreement available at https://developer.leapmotion.com/sdk_agreement, or another agreement between Ultraleap Ltd. and you, your company or other organization.
