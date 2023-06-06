UltraleapTracking for Unreal
====================

[![GitHub release](https://img.shields.io/github/release/leapmotion/leapunreal.svg)](https://github.com/leapmotion/leapunreal/releases)

The official [Ultraleap Tracking]([World-leading Hand Tracking: Small. Fast. Accurate. | Ultraleap](https://www.ultraleap.com/tracking/)) plugin for the Unreal Engine 4. 

Use convenience blueprints to add to the scene and play in editor or use a custom approach via blueprints or C++.

### Problems & Questions
Reach out at https://forums.leapmotion.com/

# Setup


1. Ensure you have the latest [Leap Motion driver installed](https://developer.leapmotion.com/get-started). This plugin requires v4 SDK tracking or newer.
2. Create new or open a project. 
3. Download [latest release](https://github.com/ultraleap/UnrealPlugin/releases) (make sure to use the .zip link)
4. Create a Plugins folder in your project root folder if one doesn't already exist
5. Drag the unzipped **UltraleapTracking** plugin into the project's Plugins folder. 
6. The plugin should be enabled and ready to use, if not enable it.

#### Quick setup video, get up and running fast!
[![Install and Go](https://img.youtube.com/vi/AvnfoqIZq6k/0.jpg)](https://youtu.be/AvnfoqIZq6k)

# How to use it - Convenience Rigged Pawn

Use one of the following methods

### Option 1. VR Mode- Example Pawn
After the plugin is enabled, change your default pawn to *LeapHandsPawn* or place it in the level with auto-posses set to player 0. If using the tracking device with the HTC Vive or Oculus Rift, it expects the pawn camera to be at the floor which is it's tracking origin. See the setup video above for steps from startup to seeing your hands in VR.

### Option 2. Desktop Mode - Example Actor
After the plugin is enabled, find *Leap Desktop Actor* in your content browser (plugin content enabled) and place it anywhere in your scene. When you hit play your hands will be tracked from the actor center with the sensor expected to be facing upwards.

![Desktop quick start](https://imgur.com/vz1xzdD.gif)

## Gestures

Version 3.6.0 introduces a new gesture detection system, which is no longer based on frames. This reduces the likelihood that gestures will be dropped, particularly when hands are rotated. Users have the option to use the new or the old system.

### Grab and Pinch

These should be globally available via Input Mapping. Look for keys *Leap (L)/(R) Pinch* and *Leap (L)/(R) Grab*

![direct input](http://i.imgur.com/2oDQllv.png)

If you add input mapping to a non-input chain actor, ensure you override the default to receive the input 
![ensure input is received](http://i.imgur.com/zWMrHxn.png)

Leap Options (see below) exposes new settings that allow the sensitivity of the gesture detection to be adjusted:

![image info](./Resources/LeapOptions.PNG)

* Use Frame Based Gesture Detection - enables the older method of detecting pinch/grab gestures.
* Start Grab Threshold - specifies the minimum grab value that needs to be detected before a grab gesture is started.
* End Grab Threshold - specifies the minimum grab value that needs to be detected before a grab gesture is stopped.
* Start Pinch Threshold - specifies the minimum grab value that needs to be detected before a pinch gesture is started.
* End Pinch Threshold - specifies the minimum grab value that needs to be detected before a pinch gesture is stopped.
* Grab Timeout - the number of microseconds required to pass before an end grab is triggered, in which no values were detected above the end grab threshold during that time. 
* Pinch Timeout - the number of microseconds required to pass before an end pinch is triggered, in which no values were detected above the end pinch threshold during that time.

# Custom Blueprint & C++, the Leap Component

Add a *Leap Component* to any actor of choice to access events relating to the leap motion. 

![add component](http://i.imgur.com/UOAexrc.png")

The main API is the same in both languages, consult the following sections if you're not familiar with *Actor Components*.

For more information about adding an actor component in blueprint please see [Components](https://docs.unrealengine.com/latest/INT/Engine/Blueprints/UserGuide/Components/index.html).

To add an actor component in C++ please see [Creating and Attaching actor components](https://docs.unrealengine.com/latest/INT/Programming/Tutorials/Components/1/).

## Blueprint - Example implementations

Please see blueprint *LeapLowPolyHand* for an example of how all these functions and events are used in practice.

## Leap Component Events

From the Leap Component the following events are available, with *On Leap Tracking Data* event being the default way of getting latest frame data.

For blueprint you add delegate events by selecting your Leap Component and hitting +

![add events](http://i.imgur.com/sBldvwR.png")

For C++ consult how to bind [multicast delegates](https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Delegates/Multicast/)

#### On Connected
Event called when the leap service connects. Will likely be called before game begin play so some components won't receive this call. Signature: *void*
```c++
FLeapEventSignature OnLeapConnected;
```

#### On Leap Tracking Data

Event called when new tracking data is available, typically every game tick. Note that tracking data has the same origin as your hmd to properly compensate for head movement. Signature: ```const FLeapFrameData&, Frame```.

[FLeapFrameData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L356)

```c++
FLeapFrameSignature OnLeapTrackingData;
```

#### On Hand Grabbed

Event called when a leap hand grab gesture is detected. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L289)
```c++
FLeapHandSignature OnHandGrabbed;
```

#### On Hand Released

Event called when a leap hand release gesture is detected. Signature: ```const FLeapHandData&, Hand```, see  [FLeapHandData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L289)
```c++
FLeapHandSignature OnHandReleased;
```

#### On Hand Pinched

Event called when a leap hand pinch gesture is detected. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L289)
```c++
FLeapHandSignature OnHandPinched;
```

#### On Hand Unpinched

Event called when a leap hand unpinch gesture is detected. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L289)
```c++
FLeapHandSignature OnHandUnpinched;
```

#### On Hand Begin Tracking

Event called when a leap hand enters the field of view and begins tracking. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L289)
```c++
FLeapHandSignature OnHandBeginTracking;
```

#### On Hand End Tracking

Event called when a leap hand exits the field of view and stops tracking. Signature: ```const FLeapHandData&, Hand```, see [FLeapHandData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L289)
```c++
FLeapHandSignature OnHandEndTracking;
```

#### On Left Hand Visibility Changed

Event called when the left hand tracking changes. Signature: ```bool bIsVisible```.
```c++
FLeapVisibilityBoolSignature OnLeftHandVisibilityChanged;
```

#### On Right Hand Visibility Changed

Event called when the right hand begins tracking. Signature: ```bool bIsVisible```.
```c++
FLeapVisibilityBoolSignature OnRightHandVisibilityChanged;
```

#### On Leap Policies Updated

Event called when leap policies have changed. Signature: an array of policy flags defined as ```TArray<TEnumAsByte<ELeapPolicyFlag>>```. See [ELeapPolicyFlag](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L39)

```c++
FLeapPolicySignature OnLeapPoliciesUpdated;
```

#### On Leap Tracking Mode Updated

Event called when the tracking mode has changed. Signature: an enum for the current tracking mode. See [ELeapMode](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L14)

```c++
FLeapTrackingModeSignature OnLeapTrackingModeUpdated;
```



### Leap Component Function List

From the component you can also access functions to retrieve the latest frame data via polling and a convenience function to check if Left/Right hands are visible.

#### Are Hands Visible

Utility function to check if a left/right hand is visible and tracked at this moment
```c++
void AreHandsVisible(bool& LeftIsVisible, bool& RightIsVisible);
```

#### Get Latest Frame Data

Polling function to get latest data. See [FLeapFrameData](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L356).
```c++
void GetLatestFrameData(FLeapFrameData& OutData);
```

# Blueprint - Leap Blueprint Function Library

Some settings and options are global, these will affect all tracking properties and are set via global functions available to any blueprint.

### Global Functions

#### Adjusting Global Offset

By default some offsets are added for Oculus/Vive which should be good enough for the majority of use cases. Vive rotation offset assumes natural sag of 10 degrees.

If however these defaults don't work for your setup, use ```Get Leap Options``` and ```Set Leap Options``` along with ```Set members``` to adjust the global tracking offsets such that they match your physical mount distance and rotation to achieve your 1:1 tracking. A good way to test the adjustments is to look at your hand and lift your headset slightly and see that your overall hand shapes line up within ~ 1cm.

![adjust offsets](https://i.imgur.com/6QaT61D.png)

Note that these blueprint nodes are global and available everywhere; a good place to call them for a static option change is in begin play from a single blueprint instance e.g. actor.

#### Set Leap Mode

Set basic global leap tracking options. Useful for switching tracking fidelity or desktop/vr tracking mode. See [ELeapMode](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L13) and [ELeapTrackingFidelity](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L30)
```c++
static void SetLeapMode(ELeapMode Mode, ELeapTrackingFidelity Fidelity = ELeapTrackingFidelity::LEAP_NORMAL);
```

#### Set Leap Options

Set global leap options. See [FLeapOptions](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L122).
```c++
static void SetLeapOptions(const FLeapOptions& Options);
```
If tracking fidelity is set to custom, passed in ```TimewarpOffset```, ```TimewarpFactor```, ```HandInterpFactor```, and ```FingerInterpFactor``` settings will apply.

#### Get Leap Options

Gets currently set global options. See [FLeapOptions](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L122)

```c++
static void GetLeapOptions(FLeapOptions& OutOptions);
```

#### Get Leap Stats

Gets Leap read only stats such as api version, frame lookup and device information. See [FLeapStats](https://github.com/ultraleap/UnrealPlugin/blob/master/Source/UltraleapTrackingCore/Public/UltraleapTrackingData.h#L105)

```c++
static void GetLeapStats(FLeapStats& OutStats);
```

#### Set Leap Policy

Change leap policy. See [Leap Policies](https://developer.leapmotion.com/documentation/cpp/api/Leap.Controller.html#policy-flags)

```c++
static void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable);
```

## Wireless Adapter

If you're using the Leap Motion with e.g. a Vive and a [Wireless Adapter](https://www.vive.com/us/wireless-adapter/) you need to adjust the timewarp settings via ```SetLeapOptions```. Change only the tracking Fidelity to ```Leap Wireless``` on e.g. begin play and then the plugin should correctly compensate for the increased latency from the wireless link.

![setting wireless fidelity](https://i.imgur.com/v0yOqaL.png)



## Custom Rigging with the Hand Modules

### A note on Unreal's FBX import settings

When importing FBX hand models, if the model imports with the skeleton separated from the mesh as below, turn on **Use T0 as Ref pose**

![](https://i.imgur.com/JdynkEl.png)





![](https://i.imgur.com/T6F72vX.png)

Using the *Body State* system, the plugin supports basic auto-mapping of tracked data to skeletal mesh bones for 1 or 2 hands in single or multiple meshes. The auto-mapping function should work on 3,4, or 5 bones per mesh and will auto-detect this setup in your rig. This should eliminate most of the tedium of rigging hand bones and should make it easy to switch to new skeletal meshes with even different skeletal naming schemes.

To get started with a newly imported model, right click on the model and choose **Create->Anim Blueprint**

![](https://i.imgur.com/Nl5cx8t.png)

To add auto-mapping to your own ```anim instance```, re-parent it a ```BodyStateAnimInstance```

![](https://i.imgur.com/TbZfr59.png)



Once done, turn on **Detect Hand Rotation During Auto Mapping** and hit the **Auto map** button. New class defaults will now be created if bones were mapped successfully.

![](https://i.imgur.com/DcQhRXX.png)

Once auto mapped, compile the blueprint to see the results. After the compile you'll see a lot of values auto-filled in your ```anim preview editor``` window

![](https://i.imgur.com/rRBsReU.png)

To enable the hand animation, add an **Ultraleap Modify Mapped Bones** node to the **AnimGraph** and connect it to the output pose. This maps incoming Leap hand data to the hand skeleton at runtime.

![](https://i.imgur.com/V3t3NWg.png)



By default auto mapping is set to your left hand, simply changing the type to right will change the targeting, hit compile to affect changes after changing type. 

The tracking is now live in the editor so you should be able to place your hand in front of your leap and press *F* to center on the tracked location and preview how the rigging behaves with real data.


### Modifying Auto-map results

#### Automatic Scaling

To have the hand model automatically scale to the user's hand, turn on ```Scale Model to Tracking Data``` and click the ```Automap``` button. The hand model when tracked will scale to any user's hand size.
The scaling can be tweaked by changing the offset sliders to fit better with the mapped model.

#### Deformation

If you don't have a mesh setup that deforms well you can turn that off by adding an entry to your ```Mapped Bone List``` array and unchecking *Should Deform Mesh*. Any changes done to this ```Mapped Bone Anim Data``` entry will be applied after your auto-map fills it. Check your *Anim Preview Editor* to see all the mapped bone entries and final settings.

![](https://i.imgur.com/4sjji8b.png)

If you turn off deformation, only rotations will be applied to the mapped bones. To ensure the hand is still placed at the correct location you may need to fill your *anim graph* with appropriate custom changes. In this example we modify our *L_wrist* bone to translate to our BodyState wrist position. With this node active before all of our other rotations, we can now toggle the deform mesh to see how it changes the mesh hand.

![](https://i.imgur.com/LhIu3JQ.gif)

Note that the hand wrist position doesn't move since the deformed mesh position for the wrist is the same as the one we made in the graph.

#### Modifying mapping results

If the auto-mapping got some things right, but some things wrong, you can override the results by adding a ```Mapped Bone Anim Data``` entry and then changing individual bone maps. These are applied after the auto-map procedure so they will take precedence. Hit compile after adding bones to see changes.

![](https://i.imgur.com/joLg6SU.gif)

#### Saving auto-map results

You can save your auto-map results to stop it from re-mapping each time an instance is spawned. To do this hit apply in your ```anim preview editor``` and untick auto-mapping. You will see that the tracking still works as the same bones now show in your ```Mapped Bone List``` but in the class defaults section.

![](https://i.imgur.com/WQSXpPN.gif)

### Flipped chirality (left to right or right to left mapped models)

Often a single mesh is used as the model for both hands. In this case the hand that is not the same chirality (left or right) needs to have its rendering flipped. This is done in code by setting the scale to -1 on the X-axis. To flip the chirality of the model, enable **Flip Model Left Right** in the Mapped Bone list.

![](https://i.imgur.com/VH9vUDh.png)

### Two handed meshes

While it is recommended to separate your hand meshes into their own skeletal meshes to make hiding un-tracked hands easy, the auto-mapping system supports two handed meshes for e.g. characters and other atypical setups.

Start by changing the auto-mapping target to ```Both Hands``` which will make two arrays, one for each hand. The reason for multiple arrays is because it is typical for animators to rig hands with different rotation basis to ensure positive rotation values close your hand. The auto-mapping compensates for this by using a different pre-base rotation for each cluster of bones related to each hand.

![](https://i.imgur.com/JGnmDSI.png)

Since you have two entries of ```Mapped Bone Anim Data``` you should add another ```Modify Mapped Bones``` node in your anim graph so both positions and rotations are set.

#### Modifying Search Parameters
When auto mapping bone names, fixed strings are used to detect which bone is which in the skeleton. These can be modified in the **Search Names** parameter which is initially populated with the most common bone names.

![](https://i.imgur.com/i2ri6q3.png)

 

#### Character meshes

The procedure for character meshes is the same as other two handed meshes, but may also need more custom nodes to compensate for lack of e.g. deformation.

![](https://i.imgur.com/17AkrEh.png)

The above anim graphs shows auto-mapping to the standard UE mannequin skeletal mesh. Similar to the earlier two hand example, we use two arrays of ```Mapped Bone Anim Data``` and we turn off deform mesh as the weight painting on this mesh doesn't support deformation. With this setup only the rotations are tracked so you'll need to either use FK, FABRIK or you own setup to move the elbows to the correct places.

![](https://i.imgur.com/G6jqeYi.png)

In the above example we use FABRIK to each elbow, and another fabrik to the head, using our HMD data which is auto-set in the body state system. Finally we rotate all of our mapped data by 90 degrees in the offset transform for Y forward which is how skeletal meshes are typically set up.

While we needed to do some custom work in this instance, there was no need to manually map each finger or to forward data to the anim graph, both of which saved hours of work.

For characters, don't forget to use the alpha value to blend out of tracked data when tracking stops and e.g. resume your idle animations.

### Adding hands to an actor

Once the anim blueprint is set up for each hand, the hands can be added to an Actor as Child Actor Components. See the example hands in the **HandModules/Hands** folder in the UltraleapTracking plugin content.

![](https://i.imgur.com/RG6zsKz.png)

This actor can then be dragged into the scene to use the mapped hands at runtime.

# Using Multiple Devices

Multiple leap devices can be used together in a scene, either to bring multiple independent sets of tracked hands into the scene per tracking device, or
to combine input from multiple tracking devices into one set of hands.

**Independent devices**

Assign the tracked device serial number to your BodyStateAnimInstance derived anim blueprint's properties to track a specific device.
Active/plugged in devices are displayed in the dropdown. Set the Multi Device Mode to **BS_MULTIDEVICE_SINGULAR**.

![](https://i.imgur.com/K4godtX.png)

The anim blueprint can then be used in the scene as normal. If multiple devices are plugged in and no device is assigned, then the first device found will be used.

**Combined Devices**

Multiple tracking devices can be combined into a single set of hands. There's several steps to configuring this, as
the scene needs setting up so that each tracking device's relative transform can be detected at runtime.

    Note: Example scenes are provided in UltraleapTracking/Multileap/ExampleScenes. Device serial numbers for your tracking hardware will need
    to be setup in the anim blueprints used (CombinedLeft and CombinedRight), to assign correctly to your hardware.

**Setting up combined devices by hand**
1. Set up the BodystateAnimInstance derived classes (left and right) with the desired combined tracking devices:

![](https://i.imgur.com/74dKId9.png)

2. Set the Multi Device Mode to **BS_MULTI_COMBINED** and add one entry to the Combined Device Serials array for each device.
The dropdown contains the device serial numbers of every device plugged in.

3. Set the anim blueprint class for both hand's skeletal meshes in **UltraleapTracking/Multileap/BodyState/BSMultiCombinedLowPolyHand** 

![](https://i.imgur.com/WEsOkpf.png)

4. Add a **LeapHandsPawn** to the scene and set it to auto possess.
5. Set the LeapHandsPawn's LeapHands child actor to the **BSMultiCombinedLowPolyHand** edited above
![](https://i.imgur.com/44otuCn.png)
6. Drag a **TrackingDeviceActor** from **UltraleapTracking/Multileap/Tracking** into the scene and set its Active Device Serial
to the tracking device on the desktop. Make sure its transform is zeroed.
![](https://i.imgur.com/SLnoK1W.png)
7. Drag another **TrackingDeviceActor** from **UltraleapTracking/Multileap/Tracking** into the scene and set its Active Device Serial
to the HMD attached device similar to above.
8. Drag a **MultiDeviceAlignmentActor** from **UltraleapTracking/Multileap/Tracking** into the scene and set its Source Device to the VR/HMD TrackingDeviceActor
and set its Target Device to the desktop TrackingDeviceActor.
![](https://i.imgur.com/bZf7OQ7.png)

Once the above is setup, the desktop device's orientation and position will be automatically set when the scene runs.
Your hands will then be tracked by both the HMD attached tracking device and the desktop device.

# UIInput Modules

The UIInput Modules enable hand interaction with Unreal's UMG 2D UI system. Both direct and distance based interaction is supported in VR and Desktop modes.

**Distance interaction:**

 ![](https://imgur.com/v0max0k.gif)

**Close interaction**

![](https://imgur.com/W8Yyue8.gif)



The UIInput module blueprints are part of the Ultraleap Tracking Plugin. Four basic example scenes are included to get up and running quickly:

![](https://i.imgur.com/ffegX3k.png)

Buttons, sliders, check boxes and drop downs are supported with the pinch event mapping to the mouse down and up equivalent in UMG. The cursor changes size based on the pinch amount with the button action triggered when fully pinched.

To add an interactable cursor to any UMG widget, add the **DistanceCursor** widget to the main canvas and implement **InteractableWidgetInterface** on your UMG Widget. The distance cursor will track the hand interaction for that widget with the variable size cursor.

See the **InteractableWidgetActor** for how to place UMG widgets in the scene and how to setup a widget for distance interaction.

NOTE: it's important to use **Pressed** events rather than *Clicked* events as the UMG button event handlers. This is because the widget interaction IDs aren't handled correctly by UE with *clicked* events if there's more than one player controller (for example in multiplayer). 

# Interaction Engine

The Interaction Engine allows users to work with your application by interacting with *physical* or *pseudo-physical* objects. Whether a baseball, a [block](https://www.youtube.com/watch?v=oZ_53T2jBGg&t=1m11s), a virtual trackball, a button on an interface panel, or a hologram with more complex affordances, if there are objects in your application you need your user to be able to **hover** near, **touch**, or **grasp** in some way, the Interaction Engine can do some or all of that work for you.

You can find the Interaction Engine in the UltraleapTracking plugin at [ultraleap/UnrealPlugin (github.com)](https://github.com/ultraleap/UnrealPlugin)

For a quick look at what the Interaction Engine can do, we recommend adding the UltraleapTracking plugin to your project and checking out the included example scenes documented further down below. For an in-depth review of features in the Interaction Engine, keep reading.

## The basic components of interaction

- "Interaction objects" are StaticMesh/Primitive Components with an attached **IEGrabComponent**. ![](https://i.imgur.com/Qfrtilt.png)
- An **IEGrabberComponent** attaches to anything that is used to interact with items in the scene (for example a Hand SkeletelMeshComponent or a MotionControllerComponent).
  ![](https://i.imgur.com/Op7lClc.png)



Interaction objects can live anywhere in your scene, all that's needed is to attach the IEGrabComponent. In addition, for re-use, IEGrabComponents can easily be attached as part of a blueprint derived from StaticMeshActor . The **GrabCube** above is an example of this.

# Just add IEGrabComponent!

When you add an IEGrabComponent to an object, a couple of things happen automatically:

- Assuming you have a hand SkeletelMeshComponent or MotionControllerComponent with an IEGrabberComponent attached to it, you'll be able to pick up, poke, and smack the object with your hands or XR controller.

The first example in the Interaction Engine package showcases the default behavior of a handful of different objects when they first become interaction objects.

# First steps with the Interaction Engine

If you haven't already, add the UltraleapTrackingPlugin to your project:

- Download the latest UltraleapTrackingPlugin from [ultraleap/UnrealPlugin (github.com)](https://github.com/ultraleap/UnrealPlugin)
- Copy the plugin to the Plugins folder beneath your Unreal Project (create a Plugins folder if it doesn't already exist)
- Open your project and make sure 'Show Plugin Content' is enabled in the view options of your Content Browser.

## Add the Interaction Engine Pawn to your scene

Either

- Add/Drag the **IEPawnHands** actor directly into the scene, and set it's **Auto Possess Player** property to 0

Or

- Set the **IEPawnHands** class as the Default Pawn Class in your Game Mode (this requires a custom Game Mode to already be selected in World Settings or Project Settings)

That's it, you will now be able to interact in the scene with your Motion Controllers in VR, the Mouse in desktop mode and with Tracked Hands if an Ultraleap Tracking Device is connected.

## A note on input mappings

In order for the **IEPawnHands** pawn to receive input from motion controllers, keyboard and mouse, default input mappings need to be set up in your project. Example mappings are in the root of the plugin in **defaultinput.ini**. If starting a project from scratch, copying/overwriting this file into your project's **Config** folder will set the mappings up.

If you're integrating into an existing project with your own input mappings, either set up the mappings in the provided **defaultinput.ini** manually  from **Project Settings**->**Input** in the Unreal Editor, or merge the text/settings into your existing **defaultinput.ini** in your project's **Config** folder from the example .ini file provided.

# Check out the examples

The examples folder (`UltraleapTracking Content/InteractionEngine/ExampleScenes`) contains a series of example scenes that demonstrate the features of the Interaction Engine.

All of the examples can be used with Ultraleap tracked hands using an Ultraleap Tracking Device *or* with any XR controller that Unreal provides built-in support for, such as Oculus Touch controllers or Vive controllers.

## Example 1: Interaction Objects 101

![](https://i.imgur.com/7sK48ec.png)

The Interaction Objects example shows the behaviour of interaction objects when IEGrabComponents are attached.

Reach out with your hands or your motion controller and play around with the objects in front of you to get a sense of how the default physics of interaction objects feels. In particular, you should see that objects don't jitter or explode, even if you attempt to crush them or pull on the constrained objects in various directions.

On the right side of this scene are floating objects that have been marked **kinematic** and that have `ignoreGrasping` and `ignoreContact` set to `true` on their InteractionBehaviours. These objects have a material set on them that causes them to glow when hands are nearby – but due to their interaction settings, they will only receive hover information, and cannot be grasped. In general, we use **Contact** to refer specifically to the contact-handling subsystem in the Interaction Engine between interaction controllers (e.g. hands) and interaction objects (e.g. cubes).

## Example 2: Basic UI in the Interaction Engine

![](https://i.imgur.com/jlEEZVv.png)

Interacting with interface elements is a very particular *kind* of interaction, but in VR or AR, we find these interactions to make the most sense to users when they are provided physical metaphors and familiar mechanisms. We've built a small set of fine-tuned Interactions (that will continue to grow!) that deal with this extremely common use-case: The IEButton, and the IESlider.

Try manipulating this interface in various ways, including ways that it doesn't expect to be used. You should find that even clumsy users will be able to push only one button at a time: Fundamentally, *user interfaces in the Interaction Engine only allow the 'primary hovered' interaction object to be manipulated or triggered at any one time*. This is a soft constraint; primary hover data is exposed through the IEGrabComponent's API for any and all interaction objects for which **hovering** is enabled, and the IEButton enforces the constraint by disabling contact when it is not 'the primary hover' of an interaction controller.

![](https://i.imgur.com/Wp81b57.png)

In this scene, a hand attached menu is included. Applications may want to attach an interface directly to a user's hand so that certain important functionalities are always within arm's reach. This part of the example demonstrates this concept by animating one such interface into view when the user looks at their left palm.

## Example 3: Interaction Callbacks for Handle-type Interfaces

![](https://i.imgur.com/zhaHKPv.png)



The Interaction Callbacks example features a set of interaction objects that collectively form a basic **TransformTool** Actor the user may use at runtime to manipulate the position and rotation of an object. These interaction objects ignore contact, reacting only to grasping controllers and controller proximity through hovering. Instead of allowing themselves to be moved directly by grasping hands, these objects cancel out and report the grasped movement from controllers to their managing TransformTool object. As the transform tool object is attached to the cube it transforms, the handles move with when a transformation takes place.

## Example 5: Building on Interaction Objects with Anchors

![](https://i.imgur.com/yPJQOYb.png)

The IEAnchorableComponent and IEAnchorComponent build on the basic interactivity afforded by interaction objects. IEAnchorableComponents integrate well with IEGrabComponents (they are designed to sit on the same StaticMeshComponent or PrimitiveComponent) and allow an interaction object to be placed in Anchor points that can be defined anywhere in your scene.

## Example 5: Dynamic UI

![](https://i.imgur.com/RmwNZJa.png)

The dynamic UI example illustrates how easy it is to hook into dropping and docking items to automatically create and destroy UI when **IEGrabComponent** attached primitives are manipulated in the scene.

## Example 6: Advanced Properties

![](https://i.imgur.com/Z7dxbLQ.png)

The advanced properties scene demonstrates different settable properties on **IEGrabComponents**. Example properties include ignoring hover of either hand, and ignoring contact between the hands and the item.

## Example 7: Virtual keyboard

![](https://i.imgur.com/FsF9wIO.png)

The virtual keyboard uses **IEButtons** for each keyboard key. Rapid typing is possible along with long press to activate advanced character input. Event dispatchers are provided to bind to each key press. 

# Custom behaviors for interaction objects

Be sure to take a look at examples 2 through 6 to see how interaction objects can have their behavior fine-tuned to meet the specific needs of your application. The standard workflow for writing custom blueprints for interaction objects goes something like this:

- Be sure your object has an IEGrabComponent (or an IEButtonGrabComponent which inherits from IEGrabComponent) attached
- Add your own custom SceneComponent or ActorComponent derived Component and reference the IEGrabComponent
- Bind to the events the IEGrabComponent of IEGrabberComponent exposes to customise and extend the behaviour

- Check out the tooltips for the IEGrabComponent and IEGrabberComponent's properties and events to see what behavior you can modify

# Interaction types in-depth

## Hovering

Hover functionality in the Interaction Engine consists of two inter-related subsystems, referred to as 'Hover' and 'Primary Hover' respectively.

### Proximity feedback ("Hover")

Any interaction object within the Hover Activity Radius (defined in your Interaction Manager) around an interaction controller's hover point will receive the OnHoverBegin, OnHoverStay, and OnHoverEnd callbacks and have its `isHovered` state set to true, as long as both the hovering controller and the interaction object have their hover settings enabled. Interaction objects provide a public getter for getting the closest hovering interaction controller as well. In general, hover information is useful when scripting visual and audio feedback related to proximity.

## Grasping

When working with XR controllers, grasping is a pretty basic feature to implement: simply define which button should be used to grab objects, and use the motion of the grasp point to move any grasped object. However, when working with Leap hands, we no longer have the simplicity of dealing in digital buttons. Instead, we've implemented a finely-tuned heuristic for detecting when a user has intended to grasp an interaction object. Whether you're working with XR controllers or hands, the grasping API in the Interaction Engine provides a common interface for constructing logic around grasping, releasing, and throwing.

### Grasped pose & object movement

When an interaction controller picks up an object, the default implementation of all interaction controllers assumes that the intended behavior is for the object to follow the grasp point. Grasp points are explicitly defined by setting up Attach and Proximity SceneComponents on the IEGrabComponent (as references).

While grasped, interaction objects are moved under one of two mutually-exclusive modes: Kinematic or Nonkinematic. By default, kinematic interaction objects will move kinematically when grasped, and nonkinematic interaction objects will move nonkinematically when grasped. When moving kinematically, an interaction object's position and rotation *are set explicitly*, effectively teleporting the object to the new position and rotation. This allows the grasped object to clip through colliders it otherwise would not be able to penetrate. Nonkinematic grasping motions, however, cause an interaction object to instead *receive a velocity and angular velocity* that will move it to its new target position and rotation on the next physics engine update, which allows the object to collide against objects in the scene before reaching its target grasped position. Kinematic/Non Kinematic is the same as turning on and off 'Simulate Physics' on an Unreal primitive.

When an object is moved because it is being grapsed by a moving controller, the OnGraspedMovement is fired right after the object is moved, which you should subscribe to if you wish to modify how the object moves while it is grasped. Alternatively, you can disable the `moveObjectWhenGrasped` setting on interaction objects to prevent their grasped motion entirely (which will no longer cause the callback to fire).

### Throwing

When a Non-Kinematic grasped object is released, its velocity and angular velocity are implied by the direction of throw by adding an impulse after release.

# Pose Detection Components

The **PoseDetector** derived components provide a convenient way to detect what a user’s hand is doing. For example, detect when the fingers of a hand are curled or extended, whether a finger or palm is pointing in a particular direction, or whether the hand or fingertip is close to one of a set of target objects. 

![](https://i.imgur.com/7OvnlNS.png)

Detectors can be combined together using a Logic Gate. The **LogicGateDetector** is itself a pose detector component that logically combines two or more other detectors to determine its own state. Need a thumb’s up gesture? Combine a thumb extended detector with a thumb pointing upward detector using a logic gate.

Detectors generate event dispatchers calls when they activate or deactivate. This makes it easy to bind to a detector straight from blueprint.

Each detector can draw debug primitives (only visible in editor) that make it easy to see its configured conditions and whether it is currently active or inactive.

### Using a PoseDetector component

To use a **PoseDetector** derived component, either attach the component to a hand's **SkeletalMeshComponent** or create the component in a new actor, in which case, upon construction, setup it's HandSkeletalMesh variable to point to the desired hand's SkeletalMeshComponent.

The **OnPoseDetected** and **OnPoseLost** event dispatchers can then be bound to, in order to react to the detection event (for example *thumbs up* and then *thumbs up no longer detected*). See the example at **UltraleapTracking/Content/PoseDetection/Pawn/PoseDetectionLowPolyHand** for how to do this along with the example scene at **UltraleapTracking/PoseDetection/ExampleScenes/PoseDetection**.

### Combining Pose Detectors

Combine multiple detectors to create more complex behavior using the **LogicGateDetector** component. A logic gate takes any number of other detectors as input, and outputs a single boolean. It's derived from the **PoseDetector** component, so it also dispatches **OnPoseDetected** and **OnPoseLost** events. You can set logic gates to be AND gates (all inputs must be true for the output to be true) or OR gates (the output is true if any input is true). You can also negate the output to configure the gate as a NAND or NOR gate.

Since a logic gate is, itself, a **PoseDetector**, you can hook up multiple logic gates to create arbitrarily complex logic.

## Pose Detector Cookbook

The following collection of ideas illustrate how to use pose detectors to implement behaviors and interaction in your application.

### Thumb’s Up

To detect a “Thumb’s Up” use an **ExtendedFingerDetector** to check that the thumb is the only extended finger and a **FingerDirectionDetector** to check detect when the thumb is pointing up. Combine these detectors with an AND-type logic gate:

- **ExtendedFingerDetector** – configure the component so that the Thumb must be extended and the other fingers must not be extended.
- **FingerDirectionDetector** – configure the component so that:
  - Finger Name = [*Thumb bone name*]
  - Pointing Direction = (0, 0, 1)
  - Pointing Type = Relative To World
  - On and Off angles: set as desired
- **LogicGateDetector** – Set the Gate Type to AND Gate and bind to the event dispatchers to the Actor or Component that should react to the thumb’s up.

### Camera-Facing Open Hand

To detect when a palm is facing the camera use an **ExtendedFingerDetector** to check that all the fingers are extended and a **PalmDirectionDetector** to check detect when the palm is facing the camera. Combine these detectors with an AND-type logic gate:

- **ExtendedFingerDetector** – configure the component so that all fingers must be extended.
- **PalmDirectionDetector** – configure the component so that:
  - Pointing Direction = (0, 1, 0)
  - Pointing Type = Relative To Horizon
  - On and Off angles: set as desired
- **LogicGateDetector** – Set the Gate Type to AND Gate and hook up the event dispatchers to the Actor or Component that should react to the thumb’s up.

## Making Your Own Pose Detector

To create your own Pose Detector classes, create a child blueprint of the **PoseDetector** component. Then override the **CheckPose** function in your class to implement detection logic. Within the logic, call **SetPoseActive** when the pose is detected, and no longer detected. The parent **PoseDetector** component will then dispatch the relevant events for the two states.
Reference the other PoseDetector derived components to see how a CheckPose implementation works.

If you want to draw primitives to help visualise the pose detection algorithm, override **DrawDebug** in the custom component.

### Ready made Pose Detectors

#### ExtendedFingerDetector

Spheres turn yellow when detected

![](https://i.imgur.com/7OvnlNS.png)

#### ProximityDetector

Middle sphere turns green when detected, detection lost when proximity further than blue sphere

![](https://i.imgur.com/bXQVTQa.png)

#### PinchDetector

Middle pentagon turns green when detected, detection lost when pinch further than blue pentagon diameter

![](https://i.imgur.com/H6Knz7g.png)

#### PalmDirectionDetector

Triggers when palm crosses direction threshold

![](https://i.imgur.com/L061VcW.png)

#### FingerDirectionDetector

Triggers when a given finger points in a given direction over a given angle tolerance (see cone extending from the finger)

![](https://i.imgur.com/tSHmFlz.png)

# Metahuman support

You can use automapping functionality along with a custom pawn to use tracked hands combined with a metahuman.
This can be run in VR as an embodied avatar or in desktop mode. 

## Importing and mapping a metahuman from scratch

* Create a new VR template project or open your existing project
* Follow the Epic guide to create, download and import your metahuman to your project: [Getting started](https://docs.metahuman.unrealengine.com/en-US/MetahumansUnrealEngine/GettingStarted/)
* Create a child class of **UltraleapTracking/Metahumans/IEPawnHands_Metahuman**
* Move the newly created pawn class into your project's content folder 
* Set the **MetahumanBPClass** member of your newly created class to your imported Metahuman blueprint (for example HadleyBP) ![](https://imgur.com/NzHLdCp.png)
* Locate and create a new animation blueprint from your metahuman’s skeleton. ![](https://imgur.com/7X9bewG.png) ![](https://imgur.com/ET3Z7YF.png)
* Move the newly created animation blueprint outside of the metahumans common folder
* Reparent the blueprint to **BodyStateAnimInstance**. ![](https://imgur.com/JR4UV03.png)
* Select **BOTH HANDS** as the **auto map target** and **Ignore Wrist Translation** and click **Auto map**. ![](https://imgur.com/P51oMTa.png)
* Right click and delete the mapping for the **BONE LOWER ARM L** and **BONE LOWER ARM R** for both left and right **Bone Map** lists
* Copy the nodes from the template **UltraleapTracking/Metahumans/MetahumanTemplate** provided (anim graph and eventgraph) into your anim blueprint. ![](https://imgur.com/Q1n3GM7.png) ![](https://imgur.com/lraQXBv.png)
* You can use select all to copy paste events
* Compile your new animation blueprint. At this point there will be missing variable errors
* Right click on the missing  Metahuman Extensions variable and choose **Create variable for Metahuman Extensions** ![](https://imgur.com/yqfYWFq.png)
* Connect up the last pin in the animation graph to the **Output Pose** node. ![](https://imgur.com/sLqFeb4.png)
* The animation blueprint should now compile and be ready for use.

## Setting up the metahuman pawn
* Set the BodyAnimBPClass in your new pawn to your newly created anim class ![](https://imgur.com/acqWA8f.png)
* Set you new pawn as the default pawn in your game mode ![](https://imgur.com/YP0obW0.png)
* The metahuman will now run rigged in VR in your scene
* Note the pawn starts with the metahuman hidden, you have to select one of the **calibrate height** options from the hand menu to show it.

## Walking animations
A place holder is left in the template anim graph to add idle to walking animation blendspace 2D.

## FAQs

#### I've added the plugin to the plugins folder of my project and it says '*[ProjectName]* cannot be compiled'. What do I do?

This is a quirk of Unreal projects that don't have any C++ code in them (blueprint only projects). To rebuild the Ultraleap Tracking plugin, the project must be converted to a C++ project. To convert the project:

- Rename the **Plugins** folder to **Plugin** to prevent it being used on loading the project

- Open your project (.uproject) file

- Go to **File->Add C++ class** and add an empty C++ class to the project. It doesn't matter what it's named

  ![](https://i.imgur.com/FFCM0ge.png)

- Now, exit your project, rename the **Plugin** folder to **Plugins**

- Right click on your project .uproject file and choose **Generate Visual Studio Project Files**

- Open the generated solution (.sln) and build it

- You'll now be able to open your .uproject file and edit it as normal.



### How do I set up the hand meshes so that the fingers collide with other objects in the scene?

Create a **Physics Asset** on the hand mesh. A guide on how to do this is at [Skeletal Mesh Actors | Unreal Engine Documentation](https://docs.unrealengine.com/4.26/en-US/Basics/Actors/SkeletalMeshActors/). See the **Collision** section for details.

### How do I modify a single joint/bone that has been mapped slightly wrong?

In the anim graph of your anim blueprint, drag a connection out of the **Ultraleap Modify Mapped Bones** node and add a **Transform (Modify) bone** node. Now edit the **Transform** node settings to change whichever bone you want to modify in the skeleton.

![](https://i.imgur.com/fYomt8w.png)

### How do I clear a bone mapping back to *none* in the details view?

Go to the **Edit defaults** tab and click the yellow reset arrow next to the bone you want to reset.

![](https://i.imgur.com/dIjhFEO.png)

### Why don't the different hand meshes line up perfectly?

This is due to differences in each imported rigged model and whether or not metacarpal joints are included/mapped.

### How do I make my ring finger and little/pinky finger collide with objects in the scene?

By default, the Interaction Engine pawn has collisions turned off for these fingers to prevent accidentally hitting buttons/objects. To enable collisions the **Physics asset** for the left and right hands must be modified.

- Open the **IELowPloy_Rigged_Hand_Left_Physics** asset from **InteractionEngine/Pawn/IELowPoly_Rigged_Hand_Left_Physics.uasset**
  ![](https://i.imgur.com/VHTrYet.png)

- Ctrl/Multi-Select all the joints for the Ring and Pinky fingers

  ![](https://i.imgur.com/7PHr0TP.png)

- Set the Collision Response to **Enabled** 

- Repeat for the **IELowPloy_Rigged_Hand_Right_Physics** asset

In addition, if you want to enable the smaller fingers so that they can push buttons,  edit the **IEButtonGrabComponent** **Disabled Bones** list and remove **Pinky** and **Ring**.

![](https://i.imgur.com/8ADdDKn.png)



# Packaging

### Windows

To package project plugins you will need a C++ project. If you have a blueprint only project, simply add a C++ class to your project and then package away.

Below is a link to an example video for packaging for windows. The user here had a blueprint only project and added the required C++ class to package successfully. The user also added a simple on beginplay command to ensure VR is enabled on beginplay as the default behavior is desktop for UE4.

[![Windows Packaging](https://img.youtube.com/vi/pRzm0M_a8uY/0.jpg)](https://youtu.be/pRzm0M_a8uY)

### Android/Pico

The Ultraleap Tracking Plugin builds and deploys to Pico headsets. There's several steps to build for this setup:

- Install the android tools as specified in the Unreal Android guide (https://docs.unrealengine.com/4.27/en-US/SharingAndReleasing/Mobile/Android/Setup/AndroidStudio/).
- Install the tracking service APK on your Pico (using adb Install)
- Setup your project as usual (see the top of this document)
- Locate the PicoXR SDK install guide (https://developer.pico-interactive.com/sdk/index?device_id=1&platform_id=2)
- Download the PicoXR plugins for Unreal.
- Follow the getting started guide for setting the project settings for the Pico (https://developer.pico-interactive.com/docs/en/12058/unreal-xr-sdk-quickstart/)
- In Project Settings, switch on arm 64 bit then switch off arm 32 bit support for Android
- In Project Settings, disable Vulkan mobile support OR turn disable the OpenXR plugin
- In Project Settings, set the Min And Max Android SDK to 29
- Uncheck the Oculus OpenXR plugin
- Package for ANDROID_ASTC and deploy using the install batch file generated by Unreal 

Note: if using Vulkan support, the OpenXR plugin must be turned off. 
There's a dependency between the default UE VR template
and OpenXR (Oculus controller models) so these dependencies need changing or the VR template folders need removing from the project in order to package without OpenXR.

### Oculus Quest/Cross platform

The Ultraleap Tracking Plugin builds and deploys to Oculus Quest headsets and can access the Quest's native handtracking data via OpenXR.

Note that functionality that requires pinch and grasp events won’t work as this is not provided via OpenXR (for example, distance interaction pinch to click buttons)

Steps to setup:
- Install the android tools as specified in the Unreal Android guide (https://docs.unrealengine.com/4.27/en-US/SharingAndReleasing/Mobile/Android/Setup/AndroidStudio/).
- Enable Hand Tracking on your Quest
- Setup your project as usual (see the top of this document)
- Set the minimum and maximum Android SDK version to 29 in the project settings
- Disable arm 32 and enable arm 64 in the project settings
- Add the permission com.oculus.permission.HAND_TRACKING to the additional android manifest permissions in the project settings
- Change the UltraleapTracking plugin C++ to initialize in OpenXR mode by changing **START_IN_OPEN_XR_MODE** to **1** in **FUltraleapTrackingInputDevice.cpp**
- Close and rebuild the project's solution
- Package for ANDROID_ASTC and deploy using the install batch file generated by Unreal 


## Contact

Please post issues and feature requests to this [github repository issues section](https://github.com/ultraleap/UnrealPlugin/issues)
