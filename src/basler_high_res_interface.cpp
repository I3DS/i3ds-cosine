///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
///////////////////////////////////////////////////////////////////////////////

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

#include <pylon/gige/BaslerGigEInstantCamera.h>


#include <thread>



#include "../include/basler_high_res_interface.hpp"


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>




#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


#undef BOOST_LOG_TRIVIAL
#define BOOST_LOG_TRIVIAL(info) cout


using namespace std;
using namespace Pylon;



BaslerHighResInterface::BaslerHighResInterface(const char *connectionString, const char *cameraName, Operation operation)
: cameraName(cameraName), operation_(operation)

{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface constructor";
  BOOST_LOG_TRIVIAL (info) << "Camera Name: "<< cameraName;


  /*
try{


  initialiseCamera();

}
  catch (const GenericException &e)
  {
      // Error handling.
      cerr << "An exception occurred." << endl
      << e.GetDescription() << endl;
      exitCode = 1;
  }

*/
  initialiseCamera();

}





void
BaslerHighResInterface::initialiseCamera() {

  cout << "Creating Camera..." << endl;


  CDeviceInfo info;
  info.SetUserDefinedName(cameraName);
  info.SetDeviceClass( Pylon::CBaslerGigEInstantCamera::DeviceClass());
  //  info.SetPropertyValue 	( "IpAddress", "10.0.1.115"); REMARK Alternative if one wants to use ipaddress
  //info.SetSerialNumber("21694497");

  camera = new CBaslerGigEInstantCamera ( CTlFactory::GetInstance().CreateFirstDevice(info));

  cout << "Camera Created." << endl;
  //Print the model name of the camera.
  cout << "Using device : " << camera->GetDeviceInfo().GetModelName() << endl;
  cout << "Friendly Name: " << camera->GetDeviceInfo().GetFriendlyName() << endl;
  cout << "Full Name    : " << camera->GetDeviceInfo().GetFullName() << endl;
  cout << "SerialNumber : " << camera->GetDeviceInfo().GetSerialNumber() << endl;

   /*
    List to tell what kind of parameter one can use to connect to camera.

    StringList_t stringList;
    camera->GetDeviceInfo().GetPropertyNames(stringList);
    for(int i=0; i < stringList.size(); i++)
      cout << "Stringlist" << stringList[i] << endl;

    cout << endl;
    */
  cout << "connect finished" << endl;

  camera->Open();


  getGain();
  startSamplingLoop();
}



void BaslerHighResInterface::closeForParameterManipulation()
{
  camera->Close();
}



int64_t
BaslerHighResInterface::getGain ()
{
  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: GainValue";
  BOOST_LOG_TRIVIAL (info) <<   "camera.Gain.GetValue()";

  cout <<  "Gain Raw: " << camera->GainRaw.GetValue() << endl;
  return  camera->GainRaw.GetValue();
}

void
BaslerHighResInterface::setGain (int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: GainValue";
  BOOST_LOG_TRIVIAL (info) <<   "camera->Gain.SetValue()";

  camera->GainRaw.SetValue(value);
}



int64_t
BaslerHighResInterface::getShutterTime()
{
  cout <<"getShutterTime" << endl ;
  BOOST_LOG_TRIVIAL (info) << "getShutterTime()";
  cout <<"X1" << endl ;

  int exposureTimeRaw = 1;//camera->ExposureTimeRaw.GetValue();
  cout <<"X221" << endl ;

  float exposureTimeAbs = camera->ExposureTimeAbs.GetValue();
  cout <<"X14" << endl;
  //int exposureTimeRaw2 = camera->ExposureTimeRaw.GetValue();
  cout <<"X12" << endl;
  return (int)((exposureTimeAbs * exposureTimeRaw)+0.5);
}


void
BaslerHighResInterface::setShutterTime(int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "SetShutterTime(" << value <<")";
  camera->ExposureTimeAbs.SetValue(1);
  camera->ExposureTimeRaw.SetValue(value);

  //camera->ShutterTime.SetValue((float)value);
}



void
BaslerHighResInterface::setRegion(PlanarRegion region)
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::setRegion()";

  camera->Width.SetValue(region.size_x);
  camera->Height.SetValue(region.size_y);

  camera->OffsetX.SetValue(region.offset_x);
  camera->OffsetY.SetValue(region.offset_y);

  BOOST_LOG_TRIVIAL (info) << "width: "<< region.size_x << " " << "height: " << region.size_y;
  BOOST_LOG_TRIVIAL (info) << "offsetX: "<< region.offset_x << " " << "offsetY: " << region.offset_y;

}


PlanarRegion
BaslerHighResInterface::getRegion(){
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::getRegion()";

  int64_t height = camera->Height.GetValue();

  int64_t i = camera->Width.GetValue();

  int64_t width = camera->Width.GetValue();
  int64_t height2 = camera->Height.GetValue();

  int64_t offsetX = camera->OffsetX.GetValue();
  int64_t offsetY = camera->OffsetY.GetValue();

  BOOST_LOG_TRIVIAL (info) << "width: "<< width << " " << "height: " << height;
  BOOST_LOG_TRIVIAL (info) << "offsetX: "<< offsetX << " " << "offsetY: " << offsetY;

  PlanarRegion region;
  region.size_x = width;
  region.size_y = height;

  region.offset_x = offsetX;
  region.offset_y = offsetY;
  return region;
}




void
BaslerHighResInterface::setRegionEnabled(bool regionEnabled)
{
  BOOST_LOG_TRIVIAL (info) << "setRegionEnabled()";
  //camera->GainRaw.SetValue(value);
}




bool
BaslerHighResInterface::getRegionEnabled()
{
  BOOST_LOG_TRIVIAL (info) << "getRegionEnabled()";
  return camera->GainRaw.GetValue();
}


// \todo float !!!
void
BaslerHighResInterface::setTriggerInterval(int64_t value)
{
  float value_f;
  BOOST_LOG_TRIVIAL (info) << "setTriggerInterval(" << value << ")";
  camera->AcquisitionFrameRateAbs.SetValue(value_f);
}

//Todo float!!
bool
BaslerHighResInterface::checkTriggerInterval(int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval(" << value << ")";

  double min = camera->AcquisitionFrameRateAbs.GetMin();
  double max = camera->AcquisitionFrameRateAbs.GetMax();
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
  return camera->MaxShutterTime.GetValue();
#endif
}

void
BaslerHighResInterface::setMaxShutterTime(int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "setMaxShutterTime(" << value << ")";
#if 0
  camera->MaxShutterTime.SetValue(value);
#endif
}



// Todo This is actually a boolean!!!!

// TODO What about once?

bool
BaslerHighResInterface::getAutoExposureEnabled ()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::Fetching parameter: getAutoExposureEnabled";

  bool retval = false;
  Basler_GigECamera::ExposureAutoEnums e = camera->ExposureAuto.GetValue();

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
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::setAutoExposureEnabled(AutoExposure): " << enabled;
  if (enabled)
    {
      camera->ExposureAuto.SetValue(Basler_GigECamera::ExposureAutoEnums::ExposureAuto_Continuous);
    }
  else
    {
      camera->ExposureAuto.SetValue(Basler_GigECamera::ExposureAutoEnums::ExposureAuto_Off);
    }

}



void
BaslerHighResInterface::do_activate()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::do_activate()";

}


void
BaslerHighResInterface::do_deactivate()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::do_deactivate()";

}



void
BaslerHighResInterface::do_start()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::do_start()";
  stopSampling();
}

void
BaslerHighResInterface::connect()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::connect()";
  initialiseCamera();

}

void
BaslerHighResInterface::do_stop()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::do_stop()";
  stopSampling();
}

void
BaslerHighResInterface::stopSampling()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::stopSampling()";
  camera->StopGrabbing();
  if(threadSamplingLoop.joinable()){
      threadSamplingLoop.join();
  }


}



void
BaslerHighResInterface::startSamplingLoop()
{
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::startSamplingLoop()";

  // The parameter MaxNumBuffer can be used to control the count of buffers
  // allocated for grabbing. The default value of this parameter is 10.
  camera->MaxNumBuffer = 10;

  // create pylon image format converter and pylon image
  CImageFormatConverter formatConverter;
  formatConverter.OutputPixelFormat = PixelType_BGR8packed;
  CPylonImage pylonImage;

  // Create an OpenCV image
  cv::Mat openCvImage;//

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
	  cv::namedWindow ("OpenCV Display Window", CV_WINDOW_NORMAL); //AUTOSIZE //FREERATIO
	  // Display the current image with opencv
	  cv::imshow ("OpenCV Display Window", openCvImage);
	  // Define a timeout for customer's input in ms.
	  // '0' means indefinite, i.e. the next image will be displayed after closing the window
	  // '1' means live stream
	  cv::waitKey (1);

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

#ifndef HR_CAMERA
int main(int argc, char* argv[])
{

  BaslerHighResInterface *baslerHRI =  new BaslerHighResInterface("abc","abc", NULL);

  try
    {
      baslerHRI->initialiseCamera();

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
#endif
