/*
 Access to ToF cameras is provided by a GenICam-compliant GenTL Producer. A GenTL Producer is a dynamic library
 implementing a standardized software interface for accessing the camera.

 The software interacting with the GentL Producer is called a GenTL Consumer. Using this terminology,
 this sample is a GenTL Consumer, too.

 As part of the suite of sample programs, Basler provides the ConsumerImplHelper C++ template library that serves as
 an implementation helper for GenTL consumers.

 The GenICam GenApi library is used to access the camera parameters.

 For more details about GenICam, GenTL, and GenApi, refer to the http://www.emva.org/standards-technology/genicam/ website.
 The GenICam GenApi standard document can be downloaded from http://www.emva.org/wp-content/uploads/GenICam_Standard_v2_0.pdf
 The GenTL standard document can be downloaded from http://www.emva.org/wp-content/uploads/GenICam_GenTL_1_5.pdf

 This sample illustrates how to grab images from a ToF camera and how to access the image and depth data.

 The GenApi sample, which is part of the Basler ToF samples, illustrates in more detail how to configure a camera.
 */

#include <ConsumerImplHelper/ToFCamera.h>
#include <iostream>
#include <iomanip>
#include <exception>
#include <thread>
#include <stdlib.h>

#include  "../include/basler_tof_interface.hpp"

// REMARK: Must be set different to point to different library than the other Basler cameras
// or it will not run.
// Should it be configured in cmake?
#define GENICAM_GENTL64_PATH "/opt/BaslerToF/lib64/gentlproducer/gtl"

using namespace GenTLConsumerImplHelper;
using namespace std;

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// TODO Move identifying parameter and which to use?

// TODO Check Throwing in sampling loop. May be the errorhandling should return and set a global flag as in example.?

Basler_ToF_Interface::Basler_ToF_Interface (const char *connectionString, const char * camera_name,
					    Operation operation) :
    mConnectionID (connectionString),
    camera_name(camera_name),
    operation_ (operation)
{
  cout << "Basler_ToF_Interface constructor\n";
  cout << "Connection String: " << mConnectionID << "\n";
  Basler_ToF_Interface2 ();

}

void Basler_ToF_Interface::Basler_ToF_Interface2 ()
{
  // Must be set different than the other basler cameras
  // GENICAM_GENTL64_PATH=/opt/BaslerToF/lib64/gentlproducer/gtl
  cout << "Setting environment variable: GENICAM_GENTL64_PATH to :"
      << GENICAM_GENTL64_PATH << std::endl;
  setenv ("GENICAM_GENTL64_PATH", GENICAM_GENTL64_PATH, 1);
  cout << "Initialising camera environment for ToF camera"
      << GENICAM_GENTL64_PATH << std::endl;
  CToFCamera::InitProducer ();
}

Basler_ToF_Interface::~Basler_ToF_Interface ()
{
  // Release the GenTL producer and all of its resources.
  // Note: Don't call TerminateProducer() until the destructor of the CToFCamera
  // class has been called. The destructor may require resources which may not
  // be available anymore after TerminateProducer() has been called.
  cout << "Terminating camera environment for ToF camera" << endl;
  if (CToFCamera::IsProducerInitialized ())
    {
      CToFCamera::TerminateProducer ();  // Won't throw any exceptions
    }
}


//TODO Not found?

int64_t
Basler_ToF_Interface::getShutterTime ()
{
  return -1;
}


//TODO Not found?
void
Basler_ToF_Interface::setShutterTime (int64_t value)
{

  
  
  
  
}



void
Basler_ToF_Interface::setRegion (const PlanarRegion region)
{
  BOOST_LOG_TRIVIAL (info) << "setRegion";

    GenApi::CIntegerPtr ptrWidth (m_Camera.GetParameter ("Width"));
    GenApi::CIntegerPtr ptrHeight (m_Camera.GetParameter ("Height"));
    GenApi::CIntegerPtr ptrOffsetX (m_Camera.GetParameter ("OffsetX"));
    GenApi::CIntegerPtr ptrOffsetY (m_Camera.GetParameter ("OffsetY"));
    
    ptrWidth->SetValue (region.size_x);
    ptrHeight->SetValue (region.size_y); 
    ptrOffsetX->SetValue (region.offset_x);
    ptrOffsetY->SetValue (region.offset_y);
}

PlanarRegion
Basler_ToF_Interface::getRegion ()
{
  BOOST_LOG_TRIVIAL (info) << "getRegion";

  GenApi::CIntegerPtr ptrWidth (m_Camera.GetParameter ("Width"));
  GenApi::CIntegerPtr ptrHeight (m_Camera.GetParameter ("Height"));
  GenApi::CIntegerPtr ptrOffsetX (m_Camera.GetParameter ("OffsetX"));
  GenApi::CIntegerPtr ptrOffsetY (m_Camera.GetParameter ("OffsetY"));
  int64_t size_x = ptrWidth->GetValue (); //    getParameter ("Width");
  int64_t size_y = ptrHeight->GetValue (); // getParameter ("Height");
  int64_t offset_x = ptrOffsetX->GetValue (); //getParameter ("OffsetY");
  int64_t offset_y = ptrOffsetY->GetValue (); //getParameter ("RegionHeight");
  
  PlanarRegion region;

  region.offset_x = offset_x;
  region.offset_y = offset_y;
  region.size_x = size_x;
  region.size_y = size_y;
  
  
  return region;

 }


//// TODO Float conversion
/// And scale
void
Basler_ToF_Interface::setTriggerInterval (int64_t sampleRate)
{

  GenApi::CFloatPtr ptrTriggerInterval (m_Camera.GetParameter ("AcquisitionFrameRate"));
  ptrTriggerInterval->SetValue (sampleRate);
}


//// TODO Float conversion
/// And scale
bool
Basler_ToF_Interface::checkTriggerInterval (int64_t rate)
{

  GenApi::CFloatPtr ptrTriggerInterval (m_Camera.GetParameter ("AcquisitionFrameRate"));
  float maxRate = ptrTriggerInterval->GetMax();
  float minRate = ptrTriggerInterval->GetMin();
  if((rate >= minRate) && rate >= maxRate){
      return true;
  }
  else
    {
      return false;
    }
}



/// REMARK HDR mode
bool
Basler_ToF_Interface::getAutoExposureEnabled ()
{
  GenApi::CEnumerationPtr ptrAutoExposureEnabled (m_Camera.GetParameter ("ExposureAuto"));
  const char *as = ptrAutoExposureEnabled->ToString();
  if(strncmp(as,"Continuous",4) == 0)
      {
	return true;
      }
    else
      {
	return false;
      }
}

void
Basler_ToF_Interface::setAutoExposureEnabled (bool value)
{
  GenApi::CEnumerationPtr ptrAutoExposureEnabled (m_Camera.GetParameter ("ExposureAuto"));
  if(value== true)
    {
      ptrAutoExposureEnabled->FromString("Continuous");
    }
  else
    {
      ptrAutoExposureEnabled->FromString("Off");
    }
}

/// \TODO Not found?
int64_t
Basler_ToF_Interface::getGain ()
{
  return -1;
}


/// \TODO Not found?
void
Basler_ToF_Interface::setGain (int64_t value)
{

}


void
Basler_ToF_Interface::setRegionEnabled (bool regionEnabled)
{

}

bool
Basler_ToF_Interface::getRegionEnabled ()
{
  return false;
}


/// \TODO Is this exposure time here?
// Actually a float in us
int64_t
Basler_ToF_Interface::getMaxShutterTime ()
{
  GenApi::CFloatPtr ptrMaxShutterTime (m_Camera.GetParameter ("ExposureTime"));
  return ptrMaxShutterTime->GetValue ();
 
}

/// TODO Is this exposure time here?
// Actually a float in us
void
Basler_ToF_Interface::setMaxShutterTime (int64_t value)
{
  GenApi::CFloatPtr ptrMaxShutterTime (m_Camera.GetParameter ("ExposureTime"));
  ptrMaxShutterTime->SetValue (value);
}

bool
Basler_ToF_Interface::connect ()
{
  do_activate();
  return true;
}

// TODO Which parameter to use to find?
void
Basler_ToF_Interface::do_activate ()
{
  cout << "do_activate ()\n";
  try
    {
      // Open the first camera found, i.e., establish a connection to the camera device.
      // m_Camera.OpenFirstCamera ();

      //  CToFCamera::OpenFirstCamera() is a shortcut for the following sequence:
      CameraList cameras = CToFCamera::EnumerateCameras ();
      CameraInfo camInfo = *cameras.begin ();
      // m_Camera.Open (camInfo);

      //  If there are multiple cameras connected and you want to open a specific one, use
      //  the CToFCamera::Open( CameraInfoKey, string ) method.

      // Example: Open a camera using its IP address
      // CToFCamera::Open( IpAddress, "10.0.1.110" );
      //m_Camera.Open (IpAddress, "10.0.1.110");
      
      m_Camera.Open (UserDefinedName, camera_name);
      
      
      
      
      /*
       Instead of the IP address, any other property of the CameraInfo struct can be used,
       e.g., the serial number or the user-defined name:

       CToFCamera::Open( SerialNumber, "23167572" );
       CToFCamera::Open( UserDefinedName, "Left" );
       */

      cout << "Connected to camera " << m_Camera.GetCameraInfo ().strDisplayName
	  << endl;

      // Enable 3D (point cloud) data, intensity data, and confidence data.
      GenApi::CEnumerationPtr ptrComponentSelector = m_Camera.GetParameter (
	  "ComponentSelector");
      GenApi::CBooleanPtr ptrComponentEnable = m_Camera.GetParameter (
	  "ComponentEnable");
      GenApi::CEnumerationPtr ptrPixelFormat = m_Camera.GetParameter (
	  "PixelFormat");

      // Enable range data.
      ptrComponentSelector->FromString ("Range");
      ptrComponentEnable->SetValue (true);
      // Range information can be sent either as a 16-bit grey value image or as 3D coordinates (point cloud). For this sample, we want to acquire 3D coordinates.
      // Note: To change the format of an image component, the Component Selector must first be set to the component
      // you want to configure (see above).
      // To use 16-bit integer depth information, choose "Mono16" instead of "Coord3D_ABC32f".
      ptrPixelFormat->FromString ("Coord3D_ABC32f");

      ptrComponentSelector->FromString ("Intensity");
      ptrComponentEnable->SetValue (true);

      ptrComponentSelector->FromString ("Confidence");
      ptrComponentEnable->SetValue (true);
    }
  catch (const GenICam::GenericException& e)
    {
      cerr << "Exception occurred: " << e.GetDescription () << endl;
      // After successfully opening the camera, the IsConnected method can be used
      // to check if the device is still connected.
      if (m_Camera.IsOpen () && !m_Camera.IsConnected ())
	{
	  cerr << "Camera has been removed." << endl;
	  throw std::runtime_error ("Camera has been removed.");
	}
      else
	{
	  throw e;
	}
    }

}

void
Basler_ToF_Interface::do_start ()
{
  cout << "do_start()\n";
  threadSamplingLoop = std::thread (&Basler_ToF_Interface::StartSamplingLoop,
				    this);
}

void
Basler_ToF_Interface::do_stop ()
{
  cout << "do_stop()\n";
  stopSamplingLoop = true;
  if (threadSamplingLoop.joinable ())
    {
      cout << "waiting for join()\n";
      threadSamplingLoop.join ();
      cout << "did join()\n";
    }
 // m_Camera.Close();
}

void Basler_ToF_Interface::do_deactivate (){
  m_Camera.Close();
}


void
Basler_ToF_Interface::StartSamplingLoop ()
{
  cout << "startSamplingLoopx()\n";
  try
    {
      m_nBuffersGrabbed = 0;
      // Acquire images until the call-back onImageGrabbed indicates to stop acquisition.
      // 5 buffers are used (round-robin).
      m_Camera.GrabContinuous (15, 1000, this,
			       &Basler_ToF_Interface::onImageGrabbed);

      // Clean-up
      m_Camera.Close ();
    }
  catch (const GenICam::GenericException& e)
    {
      cerr << "Exception occurred: " << e.GetDescription () << endl;
      // After successfully opening the camera, the IsConnected method can be used
      // to check if the device is still connected.
      if (m_Camera.IsOpen () && !m_Camera.IsConnected ())
	{
	  cerr << "Camera has been removed." << endl;
	  throw std::runtime_error ("Camera has been removed.");
	}
      else
	{
	  throw e;
	}
    }
}

/// TODO Show we avoid throwing exception from inside here? and just signal via variable as in example?
bool
Basler_ToF_Interface::onImageGrabbed (GrabResult grabResult, BufferParts parts)
{
  if (grabResult.status == GrabResult::Timeout)
    {
      cerr << "Timeout occurred. Acquisition stopped." << endl;
      // The timeout might be caused by a removal of the camera. Check if the camera
      // is still connected.m_Camera
      if (!m_Camera.IsConnected ())
	{
	  cerr << "Camera has been removed." << endl;
	  throw std::runtime_error ("Camera has been removed.");
	}

      return false; // Indicate to stop acquisition
    }
  m_nBuffersGrabbed++;
  if (grabResult.status != GrabResult::Ok)
    {
      cerr << "Image " << m_nBuffersGrabbed << "was not grabbed." << endl;
    }
  else
    {
      // Retrieve the values for the center pixel
      const int width = (int) parts[0].width;
      const int height = (int) parts[0].height;
      const int x = (int) (0.5 * width);
      const int y = (int) (0.5 * height);
      CToFCamera::Coord3D *p3DCoordinate = (CToFCamera::Coord3D*) parts[0].pData
	  + y * width + x;
      uint16_t *pIntensity = (uint16_t*) parts[1].pData + y * width + x;
      uint16_t *pConfidence = (uint16_t*) parts[2].pData + y * width + x;

      cout << "BASLERTOF::before operationXXXYYYYYY" << endl;
      cout << "Center pixelc of image " << setw (2) << m_nBuffersGrabbed << ": ";
      cout.setf (ios_base::fixed);
      cout.precision (1);
      if (p3DCoordinate->IsValid ())
	cout << "x=" << setw (6) << p3DCoordinate->x << " y=" << setw (6)
	    << p3DCoordinate->y << " z=" << setw (6) << p3DCoordinate->z;
      else
	cout << "x=   n/a y=   n/a z=   n/a";
      
      
      cout << " intensity=" << setw (5) << *pIntensity << " confidence="
	  << setw (5) << *pConfidence << endl;
      
      cout << "BASLERTOF::before operationXXX" << endl;
      clock::time_point next = clock::now();
      cout << "BASLERTOF::before operation" << endl;
     // operation_((unsigned char *)&parts, std::chrono::duration_cast<std::chrono::microseconds>(next.time_since_epoch()).count());

      
      
    }
  return !stopSamplingLoop;
  // return m_nBuffersGrabbed < 10; // Indicate to stop acquisition when 10 buffers are grabbed
}




#ifndef TOF_CAMERA
int
main (int argc, char* argv[])
{
  Basler_ToF_Interface *basler_ToF_Interface = new Basler_ToF_Interface ();
  basler_ToF_Interface->do_activate ();
  basler_ToF_Interface->do_start ();
  sleep (30);
  basler_ToF_Interface->do_stop ();
}
#endif
