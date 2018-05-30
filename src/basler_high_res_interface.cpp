///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

#include <pylon/gige/BaslerGigEInstantCamera.h>

// Use sstream to create image names including integer
#include <sstream>
#include <thread>

#include <unistd.h>

#include "../include/basler_high_res_interface.hpp"



#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#undef BOOST_LOG_TRIVIAL
#define BOOST_LOG_TRIVIAL(info) cout

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using GenApi objects
using namespace GenApi;

// Namespace for using opencv objects.
using namespace cv;

// Namespace for using cout.
using namespace std;



// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 1000;

// The exit code of the sample application.
int exitCode = 0;

BaslerHighResInterface::BaslerHighResInterface(){

}

void
BaslerHighResInterface::initialiseCamera() {
  // Create an instant camera object with the camera device found first.
  cout << "Creating Camera..." << endl;
  camera = new CInstantCamera (CTlFactory::GetInstance ().CreateFirstDevice ());

  // or use a device info object to use a specific camera
  //CDeviceInfo info;
  //info.SetSerialNumber("21694497");
  //CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice(info));
  cout << "Camera Created." << endl;
  // Print the model name of the camera.
  cout << "Using device : " << camera->GetDeviceInfo().GetModelName() << endl;
  cout << "Friendly Name: " << camera->GetDeviceInfo().GetFriendlyName() << endl;
  cout << "Full Name    : " << camera->GetDeviceInfo().GetFullName() << endl;
  cout << "SerialNumber : " << camera->GetDeviceInfo().GetSerialNumber() << endl;

  cout << endl;



}

void BaslerHighResInterface::openForParameterManipulation()
{
  // Only look for cameras supported by Camera_t.
  CDeviceInfo info;
  //info.SetDeviceClass( Camera_t::DeviceClass());
  // Create an instant camera object with the first found camera device matching the specified device class.
  cameraParameters = new CBaslerGigEInstantCamera( CTlFactory::GetInstance().CreateFirstDevice( info));
  cameraParameters->Open();
}



void BaslerHighResInterface::closeForParameterManipulation()
{
  cameraParameters->Close();
}



int64_t
BaslerHighResInterface::getGain ()
{
  //BOOST_LOG_TRIVIAL (info) << "Fetching parameter: GainValue";
  //BOOST_LOG_TRIVIAL (info) <<   camera.Gain.GetValue();
 //
  cout <<  "Gain Raw: " << cameraParameters->GainRaw.GetValue() << endl;
  return  cameraParameters->GainRaw.GetValue();
}

void
BaslerHighResInterface::setGain (int64_t value)
{
  //BOOST_LOG_TRIVIAL (info) << "Fetching parameter: GainValue";
  //BOOST_LOG_TRIVIAL (info) <<   camera.Gain.GetValue();
 //
  cameraParameters->GainRaw.SetValue(value);
}



int64_t
BaslerHighResInterface::getShutterTime()
{
  BOOST_LOG_TRIVIAL (info) << "getShutterTime()";
#if 0
  return cameraParameters->ShutterTime.GetValue();
#endif
}


void
BaslerHighResInterface::setShutterTime(int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "SetShutterTime(" << value <<")";
#if 0
  cameraParameters->ShutterTime.SetValue(value);
#endif
}


#if 0
void
BaslerHighResInterface::setRegion(PlanarRegion region)
{
  BOOST_LOG_TRIVIAL (info) << "setRegion()";

  camera.Width.GetValue(region.width);
  camera.Heigth.GetValue(region.height);

  camera.OffsetX.GetValue(region.offset_x);
  camera.OffsetY.GetValue(region.offset_y);

  BOOST_LOG_TRIVIAL (info) << "width: "<< region.width << " " << "heigth: " << region.heigth;
  BOOST_LOG_TRIVIAL (info) << "offsetX: "<< region.offset_X << " " << "offsetY: " << region.offset_Y;

}


PlanarRegion
BaslerHighResInterface::getRegion(){
  BOOST_LOG_TRIVIAL (info) << "getRegion()";

  int64_t width = camera.Width.GetValue();
  int64_t height = camera.Heigth.GetValue();

  int64_t offsetX = camera.OffsetX.GetValue();
  int64_t offsetY = camera.OffsetY.GetValue();

  BOOST_LOG_TRIVIAL (info) << "width: "<< width << " " << "heigth: " << heigth;
  BOOST_LOG_TRIVIAL (info) << "offsetX: "<< offsetX << " " << "offsetY: " << offsetY;

 //XXXXPlanarRegion region =XXXX
 //return region;

}


#endif

void
BaslerHighResInterface::setRegionEnabled(bool regionEnabled)
{
  BOOST_LOG_TRIVIAL (info) << "setRegionEnabled()";
  //cameraParameters->GainRaw.SetValue(value);
}




bool
BaslerHighResInterface::getRegionEnabled()
{
  BOOST_LOG_TRIVIAL (info) << "getRegionEnabled()";
  return cameraParameters->GainRaw.GetValue();
}


// \todo float !!!
void
BaslerHighResInterface::setTriggerInterval(int64_t value)
{
  float value_f;
  BOOST_LOG_TRIVIAL (info) << "setTriggerInterval(" << value << ")";
  cameraParameters->AcquisitionFrameRateAbs.SetValue(value_f);
}

//Todo float!!
bool
BaslerHighResInterface::checkTriggerInterval(int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval(" << value << ")";

  double min = cameraParameters->AcquisitionFrameRateAbs.GetMin();
  double max = cameraParameters->AcquisitionFrameRateAbs.GetMax();
  BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval: min: " << min << " max:" << max;
  if(value < min || value > max)
    {
      BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval out of range";
      return false;
    }
  BOOST_LOG_TRIVIAL (info) << "Triggerinterval OK";
  return true;
}



int64_t
BaslerHighResInterface::getMaxShutterTime()
{
  BOOST_LOG_TRIVIAL (info) << "getMaxShutterTime()";
#if 0
  return cameraParameters->MaxShutterTime.GetValue();
#endif
}

void
BaslerHighResInterface::setMaxShutterTime(int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "setMaxShutterTime(" << value << ")";
#if 0
  cameraParameters->MaxShutterTime.SetValue(value);
#endif
}



// Todo This is actually a boolean!!!!

// TODO What about once?

bool
BaslerHighResInterface::getAutoExposureEnabled ()
{
  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: getAutoExposureEnabled";

  bool retval = false;
  Basler_GigECamera::ExposureAutoEnums e = cameraParameters->ExposureAuto.GetValue();

  /*
  ExposureAuto_Off --> Disables the Exposure Auto function.
  ExposureAuto_Once -->	Sets operation mode to 'once'.
  ExposureAuto_Continuous --> Sets operation mode to 'continuous'.
  */

  switch(e){
    case Basler_GigECamera::ExposureAuto_Continuous:
      BOOST_LOG_TRIVIAL (info) << "Fetched parameter: ExposureAuto_Continuous";
      retval = true;
      break;

    case Basler_GigECamera::ExposureAutoEnums::ExposureAuto_Once:
      BOOST_LOG_TRIVIAL (info) << "Fetched parameter: :ExposureAuto_Once ";
      retval = false;
      break;

    case Basler_GigECamera::ExposureAutoEnums:: ExposureAuto_Off:
      BOOST_LOG_TRIVIAL (info) << "Fetched parameter:ExposureAuto_Off";
      retval = false;
      break;
  }
  return retval;
}




// TODO What about once?
void
BaslerHighResInterface::setAutoExposureEnabled (bool enabled)
{
  BOOST_LOG_TRIVIAL (info) << "Set setAutoExposureEnabled(AutoExposure): " << enabled;
  if (enabled)
    {
      cameraParameters->ExposureAuto.SetValue(Basler_GigECamera::ExposureAutoEnums::ExposureAuto_Continuous);
    }
  else
    {
      cameraParameters->ExposureAuto.SetValue(Basler_GigECamera::ExposureAutoEnums::ExposureAuto_Off);
    }

}






void
BaslerHighResInterface::stopSampling()
{
  BOOST_LOG_TRIVIAL (info) << "stopSampling()";
  camera->StopGrabbing();
  if(threadSamplingLoop.joinable()){
      threadSamplingLoop.join();
  }


}



void
BaslerHighResInterface::startSamplingLoop()
{
  BOOST_LOG_TRIVIAL (info) << "startSamplingLoop()";

  // The parameter MaxNumBuffer can be used to control the count of buffers
  // allocated for grabbing. The default value of this parameter is 10.
  camera->MaxNumBuffer = 10;

  // create pylon image format converter and pylon image
  CImageFormatConverter formatConverter;
  formatConverter.OutputPixelFormat = PixelType_BGR8packed;
  CPylonImage pylonImage;

  // Create an OpenCV image
  Mat openCvImage;//

  // Start the grabbing of c_countOfImagesToGrab images.
  // The camera device is parameterized with a default configuration which
  // sets up free-running continuous acquisition.
  //camera->StartGrabbing ();
  //  camera->StartGrabbing (c_countOfImagesToGrab);
  camera->StartGrabbing ();


  // This smart pointer will receive the grab result data.
  CGrabResultPtr ptrGrabResult;

  // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
  // when c_countOfImagesToGrab images have been retrieved.
  while (camera->IsGrabbing ())
    {
      // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
      camera->RetrieveResult (5000, ptrGrabResult,
			      TimeoutHandling_ThrowException);

      // Image grabbed successfully?
      if (ptrGrabResult->GrabSucceeded ())
	{
	  // Access the image data.
	  cout << "SizeX: " << ptrGrabResult->GetWidth () << endl;
	  cout << "SizeY: " << ptrGrabResult->GetHeight () << endl;
	  const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer ();
	  cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0]
	      << endl << endl;

	  // Convert the grabbed buffer to pylon imag
	  formatConverter.Convert (pylonImage, ptrGrabResult);
	  // Create an OpenCV image out of pylon image
	  openCvImage = cv::Mat (ptrGrabResult->GetHeight (),
				 ptrGrabResult->GetWidth (), CV_8UC3,
				 (uint8_t *) pylonImage.GetBuffer ());

	  // Create a display window
	  namedWindow ("OpenCV Display Window", CV_WINDOW_NORMAL); //AUTOSIZE //FREERATIO
	  // Display the current image with opencv
	  imshow ("OpenCV Display Window", openCvImage);
	  // Define a timeout for customer's input in ms.
	  // '0' means indefinite, i.e. the next image will be displayed after closing the window
	  // '1' means live stream
	  waitKey (1);

	}
      else
	{
	  cout << "Error: " << ptrGrabResult->GetErrorCode () << " "
	      << ptrGrabResult->GetErrorDescription () << endl;
	}
    }

}

void
BaslerHighResInterface::startSampling()
{

  BOOST_LOG_TRIVIAL (info) << "startSampling()";
  // The exit code of the sample application.
     int exitCode = 0;

     // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
     // is initialized during the lifetime of this object.
     Pylon::PylonAutoInitTerm autoInitTerm;
;
     try
     {
        //startSamplingLoop();
        threadSamplingLoop = std::thread(&BaslerHighResInterface::startSamplingLoop, this);
     }
     catch (GenICam::GenericException &e)
     {
         // Error handling.
         cerr << "An exception occurred." << endl
         << e.GetDescription() << endl;
         exitCode = 1;
     }

}


int main(int argc, char* argv[])
{

  BaslerHighResInterface *baslerHRI =  new BaslerHighResInterface();

  try
    {
      baslerHRI->initialiseCamera();

      baslerHRI->openForParameterManipulation();
      baslerHRI->getGain();
      baslerHRI->setGain(150);
      baslerHRI->getGain();
      baslerHRI->closeForParameterManipulation();





     // baslerHRI->startSamplingLoop ();
     // sleep(1);
     // baslerHRI->stopSampling();
    }
  catch (GenICam::GenericException &e)
    {
      // Error handling.
      cerr << "An exception occurred." << endl << e.GetDescription () << endl;
      exitCode = 1;
    }

  // Comment the following two lines to disable waiting on exit.
  cerr << endl << "Press Enter to exit." << endl;
  while (cin.get () != '\n')
    ;

  return exitCode;
}
