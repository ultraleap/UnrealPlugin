Ultraleap Unreal Plugin
====================

[![GitHub release](https://img.shields.io/github/release/leapmotion/leapunreal.svg)](https://github.com/leapmotion/leapunreal/releases)

## Introduction

Ultraleap's Unreal Plugin enables the data produced by integrating Ultraleap's hand tracking data to be used by developers inside their Unreal projects. It has been built to make it as easy as possible to design and use hand tracking in XR projects. Examples are included to quickly get you up and running with Ultraleap's hand tracking technology.

## Getting Started

This repository contains code for Ultraleap's Unreal Plugin which has been designed to be an easy-to-use tool for integrating Ultraleap cameras into Unreal projects. However, there are a couple of things you will need to be able to test the content you have created, and there are also several ways you can go about installing Ultraleap’s Unreal Plugin.

### Prerequisites

*N.B. This plugin only supports 64-bit Windows builds*

To use this Plugin you will need the following:

1. The latest Ultraleap Tracking Service installed
2. An Ultraleap compatible device
3. Unreal 4.27

### Installation

The Unreal Plugin repository is designed and tested to work against 4.27.

There are several ways you can consume this plugin.

1. Download the [latest release](https://github.com/leapmotion/LeapUnreal/releases) of the UnrealPlugin and SDK (make sure to use the .zip link)
2. Open or create a new project.
3. Create a Plugins folder in your project root folder (if one doesn't already exist).
4. Drag the unzipped UltraleapTracking plugin into the project's Plugins folder
5. The plugin should be enabled and ready to use. If not, enable it.
6. Use our Unreal Examples for object interaction.

#### Quick Setup Video

Watch this quick setup video to get up and running fast.

[![Install and Go](https://img.youtube.com/vi/AvnfoqIZq6k/0.jpg)](https://youtu.be/AvnfoqIZq6k)

**Please note:**

- If you are sourcing the Unreal Plugin directly from this repository, you may find that it does not function well with earlier versions of Unreal

### Dependencies

None

## Documentation

You can find out more about how to use the Unreal Plugin [here](Documentation.md)

## Usage

#### Interaction Engine

The Interaction Engine provides physics representations of hands and VR controllers fine-tuned with interaction heuristics to provide a fully-featured interaction API: grasping, throwing, stable 'soft' collision feedback, and proximity. It also comes with with a suite of examples and prefabs to power reliable, stable 3D user interfaces as well as any physics-critical experiences.

- We include a scene that shows hand tracking working with complex shapes, allowing the user to pick up and interact with objects in the scene
- We have an example to show how to interact with Unity UI
- We include an example showing UI attached to the hand (as opposed to fixed in the scene)

#### Hands

Enables developers to use hand tracking data to drive their own 3D Hand assets without writing any code, includes sample hand assets. Can be used to include any custom hand visuals or bind hand tracking data to things in your scene.

- We provide different styles of 3D hands that you can use
- We have in-depth documentation online with an explanation of each feature
- We have included step by step guides within the Editor which teaches you how to set up hands without the need to open online documentation
- No programming knowledge is needed
- We provide shaders to support HDRP/URP & the Standard render pipeline.

#### UI Input:

Enables developers to retrofit their existing 2D UIs so that they can be interacted with using hand tracking. Helps developers to get started with hand tracking without needing to build something from scratch

**Discover more about our recommended examples and the applicable use cases in our** [**XR Design Guidelines**](https://docs.ultraleap.com/xr-guidelines/).

### Contributing

Our vision is to make it as easy as possible to design the best user experience for hand tracking use cases in VR. We learn and are inspired by the creations from our open source community - any contributions you make are greatly appreciated.

1. Fork the Project
2. Create your Feature Branch:
   `git checkout -b feature/AmazingFeature`
3. Commit your Changes:
   `git commit -m "Add some AmazingFeature"`
4. Push to the Branch:
   `git push origin feature/AmazingFeature`
5. Open a Pull Request

### License

Use of Ultraleap's Unreal Plugin is subject to the [Apache V2 License Agreement](http://www.apache.org/licenses/LICENSE-2.0).

## Contact

User Support: [support@ultraleap.com](mailto:support@ultraleap.com)

## Community Support

Our [Developer Forum](https://forums.leapmotion.com/) is a place where you are actively encouraged to share your questions, insights, ideas, feature requests and projects.

## Links

[Ultraleap Unreal Plugin](https://github.com/ultraleap/UnrealPlugin.git)
