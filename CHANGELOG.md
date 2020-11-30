LeapUnreal - Change Log
====================

Versions

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