////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.               /
// Leap Motion proprietary and confidential. Not for distribution.              /
// Use subject to the terms of the Leap Motion SDK Agreement available at       /
// https://developer.leapmotion.com/sdk_agreement, or another agreement         /
// between Leap Motion and you, your company or other organization.             /
////////////////////////////////////////////////////////////////////////////////

// usage:
//   swig -c++ -python -o LeapPython.cpp -interface LeapPython LeapC++.i
//   swig -c++ -java   -o LeapJava.cpp -package com.leapmotion.leap -outdir com/leapmotion/leap LeapC++.i

%module(directors="1", threads="1") Leap
#pragma SWIG nowarn=325

%include "std_string.i"
%include "std_vector.i"
%include "stdint.i"
%include "attribute.i"

////////////////////////////////////////////////////////////////////////////////
// Ignore constructors for internal use only                                    /
////////////////////////////////////////////////////////////////////////////////

%ignore Leap::Finger::Finger(FingerImplementation*);
%ignore Leap::Bone::Bone(BoneImplementation*);
%ignore Leap::Hand::Hand(HandImplementation*);
%ignore Leap::Arm::Arm(HandImplementation*);
%ignore Leap::Image::Image(ImageImplementation*);
%ignore Leap::HeadPose::HeadPose(HeadPoseImplementation*);
%ignore Leap::Frame::Frame(FrameImplementation*);
%ignore Leap::Config::Config(ControllerImplementation*);
%ignore Leap::Controller::Controller(ControllerImplementation*);
%ignore Leap::Controller::Controller(const char*, Leap::Controller::Listener*);
%ignore Leap::Device::Device(DeviceImplementation*);
%ignore Leap::FailedDevice::FailedDevice(FailedDeviceImplementation*);

////////////////////////////////////////////////////////////////////////////////
// Ignore duplicate Config functions, as far as SWIG is concerned
////////////////////////////////////////////////////////////////////////////////
%ignore Leap::Config::value(const char*) const;
%ignore Leap::Config::value(const char*, uint32_t) const;
%ignore Leap::Config::setValue(const char*, const Value& value);
%ignore Leap::Config::setValue(const std::string&, const char*);

/////////////////////////////////////////////////////////////////////////////////////
// Set Attributes (done after functions are uppercased, but before vars are lowered) /
/////////////////////////////////////////////////////////////////////////////////////
//TODO: If possible, figure out how to auomatically make any C++ function
// that is const and takes no arguments be defined as a property in C/

%define %constattrib( Class, Type, Name )
  %attribute( Class, Type, Name, Name )
%enddef

%define %staticattrib(Class, AttributeType, AttributeName)
  %ignore Class::AttributeName();
  %ignore Class::AttributeName() const;
  %immutable Class::AttributeName;
  %extend Class {
    AttributeType AttributeName;
  }
  %{
    #define %mangle(Class) ##_## AttributeName ## _get() Class::AttributeName()
  %}
%enddef

%define %leapattrib( Class, Type, Name )
  %attributeval(Class, Leap::Type, Name, Name)
%enddef

#if SWIGPYTHON

%rename(FingerJoint) Leap::Finger::Joint;
%rename(FingerType) Leap::Finger::Type;
%rename(BoneType) Leap::Bone::Type;
%rename(DeviceType) Leap::Device::Type;
%rename(FailureType) Leap::FailedDevice::Failure;

#endif

#if SWIGPYTHON

%typemap(varout, noblock=1) SWIGTYPE & {
  %set_varoutput(SWIG_NewPointerObj(%as_voidptr(&$1()), $descriptor, %newpointer_flags));
}

%rename("%(undercase)s", notregexmatch$name="^[A-Z0-9_]+$") "";

#endif

#if SWIGPYTHON

%constattrib( Leap::Finger, Leap::Finger::Type, type )

%leapattrib( Leap::Bone, Vector, prevJoint );
%leapattrib( Leap::Bone, Vector, nextJoint );
%leapattrib( Leap::Bone, Vector, center );
%leapattrib( Leap::Bone, Vector, direction );
%constattrib( Leap::Bone, float, length );
%constattrib( Leap::Bone, float, width );
%constattrib( Leap::Bone, Leap::Bone::Type, type )
%leapattrib( Leap::Bone, Matrix, basis )
%constattrib( Leap::Bone, bool, isValid );

%constattrib( Leap::Hand, int, id );
%leapattrib( Leap::Hand, FingerList, fingers );
%leapattrib( Leap::Hand, Vector, palmPosition );
%leapattrib( Leap::Hand, Vector, palmVelocity );
%leapattrib( Leap::Hand, Vector, palmNormal );
%leapattrib( Leap::Hand, Vector, direction );
%leapattrib( Leap::Hand, Matrix, basis )
%constattrib( Leap::Hand, bool, isValid );
%constattrib( Leap::Hand, float, grabAngle );
%constattrib( Leap::Hand, float, pinchDistance );
%constattrib( Leap::Hand, float, grabStrength );
%constattrib( Leap::Hand, float, pinchStrength );
%constattrib( Leap::Hand, float, palmWidth );
%leapattrib( Leap::Hand, Vector, stabilizedPalmPosition )
%leapattrib( Leap::Hand, Vector, wristPosition )
%constattrib( Leap::Hand, float, timeVisible );
%constattrib( Leap::Hand, bool, isLeft );
%constattrib( Leap::Hand, bool, isRight );
%leapattrib( Leap::Hand, Frame, frame );
%leapattrib( Leap::Hand, Arm, arm );

%constattrib( Leap::Arm, float, width );
%leapattrib( Leap::Arm, Vector, center );
%leapattrib( Leap::Arm, Vector, direction );
%leapattrib( Leap::Arm, Matrix, basis )
%leapattrib( Leap::Arm, Vector, elbowPosition );
%leapattrib( Leap::Arm, Vector, wristPosition );
%constattrib( Leap::Arm, bool, isValid );

%constattrib( Leap::Image, int64_t, sequenceId );
%constattrib( Leap::Image, int32_t, id );
%constattrib( Leap::Image, int, width );
%constattrib( Leap::Image, int, height );
%constattrib( Leap::Image, int, bytesPerPixel );
%constattrib( Leap::Image, Leap::Image::FormatType, format );
%constattrib( Leap::Image, int, distortionWidth );
%constattrib( Leap::Image, int, distortionHeight );
%constattrib( Leap::Image, float, rayOffsetX );
%constattrib( Leap::Image, float, rayOffsetY );
%constattrib( Leap::Image, float, rayScaleX );
%constattrib( Leap::Image, float, rayScaleY );
%constattrib( Leap::Image, int64_t, timestamp );
%constattrib( Leap::Image, bool, isValid );

%constattrib( Leap::HeadPose, int64_t, timestamp );
%leapattrib( Leap::HeadPose, Vector, position );
%leapattrib( Leap::HeadPose, Quaternion, orientation );

%constattrib( Leap::FingerList, bool, isEmpty );
%constattrib( Leap::HandList, bool, isEmpty );
%constattrib( Leap::ImageList, bool, isEmpty );
%constattrib( Leap::DeviceList, bool, isEmpty );
%constattrib( Leap::FailedDeviceList, bool, isEmpty );

%leapattrib( Leap::FingerList, Finger, leftmost );
%leapattrib( Leap::FingerList, Finger, rightmost );
%leapattrib( Leap::FingerList, Finger, frontmost );
%leapattrib( Leap::HandList, Hand, leftmost );
%leapattrib( Leap::HandList, Hand, rightmost );
%leapattrib( Leap::HandList, Hand, frontmost );

%constattrib( Leap::Frame, int64_t, id );
%constattrib( Leap::Frame, int64_t, timestamp );
%constattrib( Leap::Frame, float, currentFramesPerSecond );
%leapattrib( Leap::Frame, FingerList, fingers );
%leapattrib( Leap::Frame, HandList, hands );
%leapattrib( Leap::Frame, ImageList, images );
%leapattrib( Leap::Frame, ImageList, rawImages );
%constattrib( Leap::Frame, bool, isValid );

%constattrib( Leap::Device, float, horizontalViewAngle );
%constattrib( Leap::Device, float, verticalViewAngle );
%constattrib( Leap::Device, float, range );
%constattrib( Leap::Device, float, baseline );
%constattrib( Leap::Device, bool, isValid );
%constattrib( Leap::Device, bool, isStreaming );
%constattrib( Leap::Device, bool, isSmudged );
%constattrib( Leap::Device, bool, isLightingBad );

%attributestring( Leap::Device, std::string, serialNumber, serialNumber );

%attributestring( Leap::FailedDevice, std::string, pnpId, pnpId);
%constattrib( Leap::FailedDevice, Leap::FailedDevice::FailureType, failure);

%staticattrib( Leap::Finger, static const Finger&, invalid);
%staticattrib( Leap::Bone, static const Bone&, invalid);
%staticattrib( Leap::Hand, static const Hand&, invalid);
%staticattrib( Leap::Arm, static const Arm&, invalid);
%staticattrib( Leap::Image, static const Image&, invalid);
%staticattrib( Leap::Device, static const Device&, invalid );
%staticattrib( Leap::Frame, static const Frame&, invalid);

%constattrib( Leap::Vector, float, magnitude );
%constattrib( Leap::Vector, float, magnitudeSquared );
%constattrib( Leap::Vector, float, pitch );
%constattrib( Leap::Vector, float, roll );
%constattrib( Leap::Vector, float, yaw );
%leapattrib( Leap::Vector, Vector, normalized );

%constattrib( Leap::Controller, bool, isConnected );
%constattrib( Leap::Controller, Controller::PolicyFlag, policyFlags );
%leapattrib( Leap::Controller, ImageList, images );
%leapattrib( Leap::Controller, ImageList, rawImages );
%leapattrib( Leap::Controller, DeviceList, devices );

%staticattrib( Leap::Vector, static const Vector&, zero );
%staticattrib( Leap::Vector, static const Vector&, xAxis );
%staticattrib( Leap::Vector, static const Vector&, yAxis );
%staticattrib( Leap::Vector, static const Vector&, zAxis );
%staticattrib( Leap::Vector, static const Vector&, forward );
%staticattrib( Leap::Vector, static const Vector&, backward );
%staticattrib( Leap::Vector, static const Vector&, left );
%staticattrib( Leap::Vector, static const Vector&, right );
%staticattrib( Leap::Vector, static const Vector&, up );
%staticattrib( Leap::Vector, static const Vector&, down );

%staticattrib( Leap::Matrix, static const Matrix&, identity );

#endif

%ignore Leap::Image::data() const;
%ignore Leap::Image::distortion() const;

#if SWIGPYTHON

%include "carrays.i"
%array_class(unsigned char, byteArray)
%array_class(float, floatArray)

%extend Leap::Image {
%pythoncode {
  def data(self):
      ptr = byte_array(self.width * self.height * self.bytes_per_pixel)
      LeapPython.Image_data(self, ptr)
      return ptr
  def distortion(self):
      ptr = float_array(self.distortion_width * self.distortion_height)
      LeapPython.Image_distortion(self, ptr)
      return ptr
  __swig_getmethods__["data"] = data
  if _newclass:data = _swig_property(data)
  __swig_getmethods__["distortion"] = distortion
  if _newclass:distortion = _swig_property(distortion)
}}

%rename("%(camelcase)s", %$isclass) "";
%rename("%(camelcase)s", %$isconstructor) "";

#elif SWIGJAVA

%include "arrays_java.i"
%apply signed char[] { unsigned char* };
%apply float[] { float* };

%typemap(javacode) Leap::Image %{
  /**
  * The image data.
  *
  * The image data is a set of 8-bit intensity values. The buffer is
  * ``image.width() * image.height() * image.bytesPerPixel()`` bytes long.
  *
  * \include Image_data_1.txt
  *
  * @since 2.1.0
  */
  public byte[] data() {
    byte[] ptr = new byte[width() * height() * bytesPerPixel()];
    LeapJNI.Image_data(swigCPtr, this, ptr);
    return ptr;
  }
  /**
  * The distortion calibration map for this image.
  *
  * The calibration map is a 64x64 grid of points. Each point is defined by
  * a pair of 32-bit floating point values. Each point in the map
  * represents a ray projected into the camera. The value of
  * a grid point defines the pixel in the image data containing the brightness
  * value produced by the light entering along the corresponding ray. By
  * interpolating between grid data points, you can find the brightness value
  * for any projected ray. Grid values that fall outside the range [0..1] do
  * not correspond to a value in the image data and those points should be ignored.
  *
  * \include Image_distortion_1.txt
  *
  * The calibration map can be used to render an undistorted image as well as to
  * find the true angle from the camera to a feature in the raw image. The
  * distortion map itself is designed to be used with GLSL shader programs.
  * In other contexts, it may be more convenient to use the Image rectify()
  * and warp() functions.
  *
  * Distortion is caused by the lens geometry as well as imperfections in the
  * lens and sensor window. The calibration map is created by the calibration
  * process run for each device at the factory (and which can be rerun by the
  * user).
  *
  * @returns The float array containing the camera lens calibration map.
  * @since 2.1.0
  */
  public float[] distortion() {
    float[] ptr = new float[distortionWidth() * distortionHeight()];
    LeapJNI.Image_distortion(swigCPtr, this, ptr);
    return ptr;
  }
%}

%ignore Leap::DEG_TO_RAD;
%ignore Leap::RAD_TO_DEG;
%ignore Leap::PI;

// Use proper Java enums
%include "enums.swg"
%javaconst(1);

SWIG_JAVABODY_PROXY(public, public, SWIGTYPE)

#endif

// Ignore C++ streaming operator
%ignore operator<<;
// Ignore C++ equal operator
%ignore operator=;

#if SWIGPYTHON
%begin %{
#if defined(_WIN32) && defined(_DEBUG)
// Workaround for obscure STL template error
#include <vector>
// Workaround for non-existent Python debug library
#define _TMP_DEBUG _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG _TMP_DEBUG
#undef _TMP_DEBUG
#endif
#if defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wself-assign"
#endif
%}
#endif

#if SWIGJAVA
%begin %{
#if defined(_WIN32)
#pragma warning(disable : 4267)
#include <windows.h>
// When dynamically loading LeapJava.dll, set the DLL search path to look in the
// same the directory. This will allow loading LeapC++.dll. Create LeapJava.dll
// with the /DELAYLOAD:LeapC++.dll link option.
extern "C" BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD fdwReason,
    _In_ LPVOID lpvReserved)
{
  if (lpvReserved == 0) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
      TCHAR lpPathName[1024];
      int len;

      len = static_cast<int>(GetModuleFileName(static_cast<HMODULE>(hinstDLL),
                         lpPathName, static_cast<DWORD>(sizeof(lpPathName))));
      if (len > 0 && len < sizeof(lpPathName)) {
        for (int i = len; i >= 0; i--) {
          if (lpPathName[i] == '\\' || lpPathName[i] == '/') {
            lpPathName[i] = '\0';
            static TCHAR lpPathVar[8192]; // static to avoid stacksize concerns
            DWORD sz = GetEnvironmentVariable("PATH", lpPathVar, sizeof(lpPathVar));
            if (sz == 0) {
              // %PATH% not defined
              lpPathVar[0] = '\0';
            }
            if (sz + strlen(lpPathName) + 1 < sizeof(lpPathVar)) {
              // If someone's %PATH% is 8 kB, they have bigger problems
              static TCHAR lpTmp[8192];
              strcpy(lpTmp, lpPathVar);
              strcpy(lpPathVar, lpPathName);
              lpPathVar[strlen(lpPathName)] = ';';
              strcat(lpPathVar, lpTmp);
              SetEnvironmentVariable("PATH", lpPathVar);
            }
            break;
          }
        }
      }
    }
  }
  return TRUE;
}
#endif
%}
#endif

%typemap(csin, pre="    lock($csinput) {", post="      $csinput.Dispose();\n    }") const Leap::Controller& "Controller.getCPtr($csinput)"

%header %{
#define SWIG
#include "LeapC++.h"
%}

%feature("director") Leap::Listener;
#if SWIGPYTHON
%feature("director:except") {
  if ($error != NULL) {
    PyErr_Print();
  }
}
#endif

%pragma(java) jniclasscode=%{
  static {
    try {
      System.loadLibrary("LeapC");
      System.loadLibrary("LeapC++");
      System.loadLibrary("LeapJava");
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Native code library failed to load. \n" + e);
      System.exit(1);
    }
  }
%}

////////////////////////////////////////////////////////////////////////////////
// Operator overloading                                                         #
////////////////////////////////////////////////////////////////////////////////

#if SWIGJAVA

%ignore *::operator+=;
%ignore *::operator-=;
%ignore *::operator*=;
%ignore *::operator/=;
%ignore *::operator!=;
%ignore Leap::Vector::toFloatPointer;
%ignore Leap::Matrix::toArray3x3;
%ignore Leap::Matrix::toArray4x4;
%ignore Leap::FloatArray;

%rename(plus) *::operator+;
%rename(minus) *::operator-;
%rename(opposite) *::operator-() const;
%rename(times) *::operator*;
%rename(divide) *::operator/;
%rename(get) *::operator [];
%rename(equals) *::operator==;

%typemap(javacode) Leap::Vector
%{
  public float[] toFloatArray() {
    return new float[] {getX(), getY(), getZ()};
  }
%}
%typemap(javacode) Leap::Matrix
%{
  public float[] toArray3x3(float[] output) {
    output[0] = getXBasis().getX(); output[1] = getXBasis().getY(); output[2] = getXBasis().getZ();
    output[3] = getYBasis().getX(); output[4] = getYBasis().getY(); output[5] = getYBasis().getZ();
    output[6] = getZBasis().getX(); output[7] = getZBasis().getY(); output[8] = getZBasis().getZ();
    return output;
  }
  public double[] toArray3x3(double[] output) {
    output[0] = getXBasis().getX(); output[1] = getXBasis().getY(); output[2] = getXBasis().getZ();
    output[3] = getYBasis().getX(); output[4] = getYBasis().getY(); output[5] = getYBasis().getZ();
    output[6] = getZBasis().getX(); output[7] = getZBasis().getY(); output[8] = getZBasis().getZ();
    return output;
  }
  public float[] toArray3x3() {
    return toArray3x3(new float[9]);
  }
  public float[] toArray4x4(float[] output) {
    output[0]  = getXBasis().getX(); output[1]  = getXBasis().getY(); output[2]  = getXBasis().getZ(); output[3]  = 0.0f;
    output[4]  = getYBasis().getX(); output[5]  = getYBasis().getY(); output[6]  = getYBasis().getZ(); output[7]  = 0.0f;
    output[8]  = getZBasis().getX(); output[9]  = getZBasis().getY(); output[10] = getZBasis().getZ(); output[11] = 0.0f;
    output[12] = getOrigin().getX(); output[13] = getOrigin().getY(); output[14] = getOrigin().getZ(); output[15] = 1.0f;
    return output;
  }
  public double[] toArray4x4(double[] output) {
    output[0]  = getXBasis().getX(); output[1]  = getXBasis().getY(); output[2]  = getXBasis().getZ(); output[3]  = 0.0f;
    output[4]  = getYBasis().getX(); output[5]  = getYBasis().getY(); output[6]  = getYBasis().getZ(); output[7]  = 0.0f;
    output[8]  = getZBasis().getX(); output[9]  = getZBasis().getY(); output[10] = getZBasis().getZ(); output[11] = 0.0f;
    output[12] = getOrigin().getX(); output[13] = getOrigin().getY(); output[14] = getOrigin().getZ(); output[15] = 1.0f;
    return output;
  }
  public float[] toArray4x4() {
    return toArray4x4(new float[16]);
  }
%}

#elif SWIGPYTHON

%ignore Leap::Interface::operator=;
%ignore Leap::ConstListIterator::operator++;
%ignore Leap::Vector::toFloatPointer;
%ignore Leap::Matrix::toArray3x3;
%ignore Leap::Matrix::toArray4x4;
%ignore Leap::FloatArray;

%rename(__getitem__) *::operator [];

%extend Leap::Vector {
%pythoncode {
  def to_float_array(self): return [self.x, self.y, self.z]
  def to_tuple(self): return (self.x, self.y, self.z)
}}
%extend Leap::Matrix {
%pythoncode {
  def to_array_3x3(self, output = None):
      if output is None:
          output = [0]*9
      output[0], output[1], output[2] = self.x_basis.x, self.x_basis.y, self.x_basis.z
      output[3], output[4], output[5] = self.y_basis.x, self.y_basis.y, self.y_basis.z
      output[6], output[7], output[8] = self.z_basis.x, self.z_basis.y, self.z_basis.z
      return output
  def to_array_4x4(self, output = None):
      if output is None:
          output = [0]*16
      output[0],  output[1],  output[2],  output[3]  = self.x_basis.x, self.x_basis.y, self.x_basis.z, 0.0
      output[4],  output[5],  output[6],  output[7]  = self.y_basis.x, self.y_basis.y, self.y_basis.z, 0.0
      output[8],  output[9],  output[10], output[11] = self.z_basis.x, self.z_basis.y, self.z_basis.z, 0.0
      output[12], output[13], output[14], output[15] = self.origin.x,  self.origin.y,  self.origin.z,  1.0
      return output
}}

#endif

////////////////////////////////////////////////////////////////////////////////
// List Helpers
////////////////////////////////////////////////////////////////////////////////

#if SWIGJAVA

%define %leap_iterator_helper(BaseType)
%typemap(javainterfaces) Leap::BaseType##List "Iterable<BaseType>"
%typemap(javacode) Leap::BaseType##List
%{
  public class BaseType##ListIterator implements java.util.Iterator<BaseType> {
    int index = 0;
    @Override public boolean hasNext() {
      return index < count();
    }
    @Override public BaseType next() {
      return get(index++);
    }
    @Override public void remove() {
    }
  }
  @Override public java.util.Iterator<BaseType> iterator() {
    return new BaseType##ListIterator();
  }
%}
%enddef

#elif SWIGPYTHON

%define %leap_iterator_helper(BaseType)
%rename(__len__) Leap::BaseType##List::count;
%extend Leap::BaseType##List {
%pythoncode {
  def __iter__(self):
    _pos = 0
    while _pos < len(self):
      yield self[_pos]
      _pos += 1
}}
%enddef

#else

%define %leap_iterator_helper(BaseType)
%enddef

#endif

%define %leap_list_helper(BaseType)
%ignore Leap::BaseType##List::BaseType##List(const std::shared_ptr< ListBaseImplementation<BaseType> >&);
%ignore Leap::BaseType##List::const_iterator;
%ignore Leap::BaseType##List::begin() const;
%ignore Leap::BaseType##List::end() const;
%leap_iterator_helper(BaseType)
%enddef

%leap_list_helper(Finger);
%leap_list_helper(Image);
%leap_list_helper(Hand);
%leap_list_helper(MapPoint);
%leap_list_helper(Device);
%leap_list_helper(FailedDevice);

////////////////////////////////////////////////////////////////////////////////
// ToString methods
////////////////////////////////////////////////////////////////////////////////

#if SWIGJAVA

%javamethodmodifiers *::toString "@Override public";

#elif SWIGPYTHON

%rename(__str__) *::toString;

#endif

%include "LeapMath.h"
%include "LeapC++.h"
