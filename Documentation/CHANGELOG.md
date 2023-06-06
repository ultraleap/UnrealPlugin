Ultraleap Tracking - Change Log
====================

Versions

### Version 4.6.0
12th September 2022

* Fixed SteamVR motion controller not visible when using automatic controller vs hands switching
* Fixed auto rotation combined with hand scaling and distort mesh enabled creating corrupted/distorted hands
* Multiple leap device support as either individual/per device and combined/merged into one set of hands.  


Known issues:

* Text colour in UI interaction drop downs wrong (UE5 bug)
* Running VR with OpenXR via SteamVR doesn't see VR (UE5 bug). To work around, enable the SteamVR plugin in the project.
* In UE5, grabbing with the right hand motion controller is intermittent
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.
* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
* With multiple leap device setups where one is Screentop, the screentop device position can be offset vertically
* With multiple devices, when rapidly plugging between devices, a ghost duplicate/inactive device can be detected 


### Version 4.5.1
11th July 2022

* Fixed plugin not packaging due to missing Metahuman reference 
* Fixed plugin not packaging when OpenXR plugin disabled
* Changed debug mirror to use Scene Final Color (Scene capture works on Android)
* Updated documentation on getting started with Android/XR2 platforms

Known issues:

* Text colour in UI interaction drop downs wrong (UE5 bug)
* Running VR with OpenXR via SteamVR doesn't see VR (UE5 bug). To work around, enable the SteamVR plugin in the project.
* In UE5, grabbing with the right hand motion controller is intermittent
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.
* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
* In SteamVR, motion controller models aren't visible when using automatic controller vs hands switching


### Version 4.5.0
22nd June 2022

* Added Metahuman auto mapping functionality and templates (Note: a separate example project will be deployed to the marketplace)

Known issues:

* Text colour in UI interaction drop downs wrong (UE5 bug)
* Running VR with OpenXR via SteamVR doesn't see VR (UE5 bug). To work around, enable the SteamVR plugin in the project.
* In UE5, grabbing with the right hand motion controller is intermittent
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.
* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
* In SteamVR, motion controller models aren't visible when using automatic controller vs hands switching

### Version 4.4.0
7th June 2022

* Added Linux support/Leap libraries

Known issues:

* Text colour in UI interaction drop downs wrong (UE5 bug)
* Running VR with OpenXR via SteamVR doesn't see VR (UE5 bug). To work around, enable the SteamVR plugin in the project.
* In UE5, grabbing with the right hand motion controller is intermittent
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.
* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.


### Version 4.3.0
18th May 2022

* Added auto hand scaling support to BodyStateAnimInstance/AutoMapping workflow
* Fixed - Blocks attracted by left hand in Anchors and Dynamic UI scene when menu not shown
* Fixed - Blocks can be docked whilst open in the Dynamic UI scene
* Fixed - Auto switching between hands and controllers show left hand when in and out of tracking
* Fixed - Palm orientation pose detector now defaults to triggering on palm facing camera/user's face
* Fixed - Not a number could be received if hands tracking during scene startup
* Fixed - When picking up blocks underneath other blocks, blocks behaved as if very heavy/pinned down
* Fixed - Motion controller interaction with UMG interfaces

Known issues:

* Text colour in UI interaction drop downs wrong (UE5 bug)
* Running VR with OpenXR via SteamVR doesn't see VR (UE5 bug). To work around, enable the SteamVR plugin in the project.
* In UE5, grabbing with the right hand motion controller is intermittent
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.
* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
* In UE5 Using the SteamVR plugin only, motion controller models are not visible when auto switching is enabled.

### Version 4.2.0
28th March 2022

* Fixed initialise to VR mode for legacy LeapHandsPawn
* Added XR2/Android LeapC libraries and support
* Added support for Pico stand alone mode
* Added support for UE 5 Preview 2 - NOTE that as UE5 is still in preview, this should not be considered final support

Known issues:

* Text colour in UI interaction drop downs wrong (UE5 bug)
* Running VR with OpenXR via SteamVR doesn't see VR (UE5 bug). To work around, enable the SteamVR plugin in the project.
* In UE5, grabbing with the right hand motion controller is intermittent


* Blocks attracted by left hand in Anchors and Dynamic UI scene when menu not shown
* Blocks and be docked whilst open in the Dynamic UI scene
* Auto switching between hands and controllers show left hand when in and out of tracking
* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.


### Version 4.1.1
14th February 2022

* Fixed clicking on AutoMap button from default tab crashes project
* Hand menu show and hide algorithm now the same as the Unity tools (doesn't hide when hand high up above camera)
* Updated to latest LeapC.dll v5.3.0.0
* Added early initialisation of the Leap Plugin:
	- Removes/fixes any input mapping warnings on build (LeapGrab etc.)
	- Fixes the initialisation when packaged such that the leap device is present at begin play (this used to be only present after a delay of one frame)
* Added Pico motion controller input event mappings
* Added teleporter niagara beam from the standard UE VR template

Known issues:

* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.


### Version 4.1.0
27th January 2022

* Added Virtual Keyboard Actor with example Scene
* Added Pose detection with example scene (port of Detection utilities from Unity)
* Updated far field interaction algorithm
* Auto show/hide controllers when hands in use
* Fixed hand modules examples scenes and actors not showing hands if hands already in view on startup
* Monolithic build fixed (header dependencies)
* Epic Marketplace fixes/changes
	- CoreRedirects works in both Engine plugins folder and in the project plugins folder (references to old LeapMotion assets)
	- Licensing statements added to all headers
	- BodyState module moved to ThirdParty folder

Known issues:

* Auto calculating orientations with imported meshes that have negative scales require manual correction to the PreBaseRotation.
* HMD offset is not set automatically for SteamVR devices and should be set manually for your headset in the IEPawnHands blueprint.
* As with most Unreal 4 VR projects, when run in VR preview in the editor, full framerate is often not achievable. This is fine when packaged.

### Version 4.0.3

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

Copyright © 2012-2022 Ultraleap Ltd. All rights reserved.

Use of the Leap Motion SDK is subject to the terms of the Leap Motion SDK Agreement available at https://developer.leapmotion.com/sdk_agreement, or another agreement between Ultraleap Ltd. and you, your company or other organization.
