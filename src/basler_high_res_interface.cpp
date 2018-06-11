///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
///////////////////////////////////////////////////////////////////////////////

#undef SHOW_IMAGE
//#define SHOW_IMAGE

#ifdef SHOW_IMAGE
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

#include <pylon/gige/BaslerGigEInstantCamera.h>


#include <thread>
#include <chrono>



#include "../include/basler_high_res_interface.hpp"




#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


using namespace std;
using namespace Pylon;
namespace logging = boost::log;

BaslerHighResInterface::BaslerHighResInterface(const char *connectionString, const char *cameraName,
					       bool free_running, Operation operation)
: cameraName(cameraName), free_running_(free_running), operation_(operation)

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
 // initialiseCamera();

}





void
BaslerHighResInterface::initialiseCamera() {

  BOOST_LOG_TRIVIAL (info) << "Creating Camera...";

try{
  CDeviceInfo info;
  info.SetUserDefinedName(cameraName);
  info.SetDeviceClass( Pylon::CBaslerGigEInstantCamera::DeviceClass());
  //  info.SetPropertyValue 	( "IpAddress", "10.0.1.115"); REMARK Alternative if one wants to use ipaddress
  //info.SetSerialNumber("21694497");

  camera = new CBaslerGigEInstantCamera ( CTlFactory::GetInstance().CreateFirstDevice(info));

  BOOST_LOG_TRIVIAL (info) << "Camera Created.";
  //Print the model name of the camera.
  BOOST_LOG_TRIVIAL (info) << "Using device : " << camera->GetDeviceInfo().GetModelName();
  BOOST_LOG_TRIVIAL(info)<< "Friendly Name: " << camera->GetDeviceInfo().GetFriendlyName();
  BOOST_LOG_TRIVIAL (info)<< "Full Name    : " << camera->GetDeviceInfo().GetFullName();
  BOOST_LOG_TRIVIAL (info)<< "SerialNumber : " << camera->GetDeviceInfo().GetSerialNumber();

   /*
    List to tell what kind of parameter one can use to connect to camera.

    StringList_t stringList;
    camera->GetDeviceInfo().GetPropertyNames(stringList);
    for(int i=0; i < stringList.size(); i++)
      BOOST_LOG_TRIVIAL << "Stringlist" << stringList[i];

    */
  BOOST_LOG_TRIVIAL(info) << "connect finished";

  camera->Open();


 // getGain();
 // startSamplingLoop();

  //Switching format
  camera->PixelFormat.SetValue(Basler_GigECamera::PixelFormat_Mono12);
  sample_rate_in_Hz_ = camera->AcquisitionFrameRateAbs.GetValue();

}catch (GenICam::GenericException &e)
  {
      // Error handling.
    std::ostringstream errorDescription;
    errorDescription << "An exception occurred." << e.GetDescription();
    throw i3ds::CommandError(error_value, errorDescription.str());;
  }
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

  BOOST_LOG_TRIVIAL (info) <<  "Gain Raw: " << camera->GainRaw.GetValue();
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
  BOOST_LOG_TRIVIAL (info) <<"getShutterTime";
  BOOST_LOG_TRIVIAL (info) << "getShutterTime()";
  BOOST_LOG_TRIVIAL (info) <<"X1";

  int exposureTimeRaw = 1;//camera->ExposureTimeRaw.GetValue();
  BOOST_LOG_TRIVIAL (info) <<"X221";

  float exposureTimeAbs = camera->ExposureTimeAbs.GetValue();
  BOOST_LOG_TRIVIAL (info) <<"X14";
  //int exposureTimeRaw2 = camera->ExposureTimeRaw.GetValue();
  BOOST_LOG_TRIVIAL (info) <<"X12";
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

/*
void
BaslerHighResInterface::setTriggerInterval(int6)
{
  BOOST_LOG_TRIVIAL (info) << "setTriggerInterval(" << value << ")";
  camera->AcquisitionFrameRateEnable.SetValue(true);
  camera->AcquisitionFrameRateAbs.SetValue(int64_t);
}
*/

bool
BaslerHighResInterface::checkTriggerInterval(int64_t period)
{
  BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval(" << period << ")";

  /* My understanding:
   * if one sets AcquisitionFrameRateAbs Then ResultingFrameRateAbs will give you the what rate you will get
   *
   * Algorithm:
   * 1. Remember old AcquisitionFrameRateAbs
   * 2. Set Wanted value for  AcquisitionFrameRateAbs
   * 3. Read out ResultingFrameRateAbs
   * 4. Set Back old AcquisitionFrameRateAbs
   * 5. Check if ResultingFrameRateAbs is close to AcquisitionFrameRateAbs, then ok
   * (Remember Camera is operating i Hertz second (float) , but we get inn period in i us int64 )
   */

  float wished_rate_in_Hz = 1.e6/period;// Convert to Hz
  // Must be enabled to do calculating
  //1. Stored in  sampleRate_in_Hz_

  camera->AcquisitionFrameRateEnable.SetValue(true);
  // 2.
  BOOST_LOG_TRIVIAL (info) << "Testing frame rate: " << wished_rate_in_Hz << "Hz";
  try{
      camera->AcquisitionFrameRateAbs.SetValue(wished_rate_in_Hz);
  }catch (GenICam::GenericException &e)
  {
      // Error handling.
      BOOST_LOG_TRIVIAL (info)  << "An exception occurred." << e.GetDescription();
      BOOST_LOG_TRIVIAL (info) << "frame rate is out of range: " << wished_rate_in_Hz;
      return false;
  }
  //3.
  float resulting_rate_in_Hz = camera->ResultingFrameRateAbs.GetValue();
  BOOST_LOG_TRIVIAL (info) << "Reading Resulting frame rate  (ResultingFrameRateAbs)"
      << resulting_rate_in_Hz << "Hz";
  //4.
  BOOST_LOG_TRIVIAL (info) << "Setting back old sample rate";
  camera->AcquisitionFrameRateAbs.SetValue(sample_rate_in_Hz_);
  //.5
  if (abs(wished_rate_in_Hz - resulting_rate_in_Hz) < 1){
      sample_rate_in_Hz_= resulting_rate_in_Hz;
    BOOST_LOG_TRIVIAL (info) << "Test of sample rate yields ok. Storing it";
    return true;
  }
  else
    {
      BOOST_LOG_TRIVIAL (info) << "Test of sample rate NOT yields ok. not keeping it";
      return false;
    }

  //double min = camera->AcquisitionFrameRateAbs.GetMin();
  //double max = camera->AcquisitionFrameRateAbs.GetMax();






/*
  BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval: min: " << min << " max:" << max;
  if(value < min || value > max)
    {
      BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval out of range";
      return false;
    }
  BOOST_LOG_TRIVIAL (info) << "Triggerinterval OK";
  samplingsRate_= value;
  return true;
*/
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
  BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::do_start():" << free_running_;
  if(free_running_)
    {

      camera->TriggerMode.SetValue(Basler_GigECamera::TriggerModeEnums::TriggerMode_Off);
      camera->AcquisitionFrameRateEnable.SetValue(true);
      BOOST_LOG_TRIVIAL (info) << "BaslerHighResInterface::do_start() sample_rate_in_Hz_:" << sample_rate_in_Hz_;
      camera->AcquisitionFrameRateAbs.SetValue(sample_rate_in_Hz_);

    }
  else
    {
      camera->AcquisitionFrameRateEnable.SetValue(false);
      camera->TriggerMode.SetValue(Basler_GigECamera::TriggerModeEnums::TriggerMode_On);
      camera->TriggerSource.SetValue(Basler_GigECamera::TriggerSourceEnums::TriggerSource_Line1);
      camera->TriggerSelector.SetValue(Basler_GigECamera::TriggerSelectorEnums::TriggerSelector_FrameStart);

     }
  startSampling();
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

#ifdef SHOW_IMAGE
  // Create an OpenCV image
  cv::Mat openCvImage;//
#endif

  // Start the grabbing of c_countOfImagesToGrab images.
  // The camera device is parameterized with a default configuration which
  // sets up free-running continuous acquisition.
  //camera->StartGrabbing ();
  //  camera->StartGrabbing (c_countOfImagesToGrab);


  float AcquisitionFrameRateAbsFloat = camera->AcquisitionFrameRateAbs.GetValue();
  BOOST_LOG_TRIVIAL(info) << "AcquisitionFrameRateAbsFloat: " << AcquisitionFrameRateAbsFloat << "Hz="
      << (1./AcquisitionFrameRateAbsFloat) << "sec";


  float timeoutGrabingFloat =  2.0*1000./camera->ResultingFrameRateAbs.GetValue() ;
  BOOST_LOG_TRIVIAL (info) << "timeoutGrabingFloat: " << timeoutGrabingFloat;
  int  timeoutGrabingInt = ceil(timeoutGrabingFloat);
  BOOST_LOG_TRIVIAL (info) << "timeoutGrabingInt: " << timeoutGrabingInt << "ms";
  camera->StartGrabbing ();


  // This smart pointer will receive the grab result data.
  CGrabResultPtr ptrGrabResult;

  // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
  // when c_countOfImagesToGrab images have been retrieved.
  while (camera->IsGrabbing ())
    {
      // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
      camera->RetrieveResult (timeoutGrabingInt,
			      ptrGrabResult,
			      TimeoutHandling_ThrowException);

      // Image grabbed successfully?
      if (ptrGrabResult->GrabSucceeded ())
	{
	  // Access the image data.
	  BOOST_LOG_TRIVIAL (info) << "SizeX: " << ptrGrabResult->GetWidth ();
	  BOOST_LOG_TRIVIAL (info) << "SizeY: " << ptrGrabResult->GetHeight ();
	  const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer ();
	  BOOST_LOG_TRIVIAL (info) << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0];

#ifdef SHOW_IMAGE
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
#endif
	  uint8_t *pImageBuffer2 = (uint8_t *) ptrGrabResult->GetBuffer ();
	  clock::time_point next = clock::now();
	 // operation_(lImage->GetDataPointer(), std::chrono::duration_cast<std::chrono::microseconds>(next.time_since_epoch()).count());
	  operation_(pImageBuffer2, std::chrono::duration_cast<std::chrono::microseconds>(next.time_since_epoch()).count());

	}
      else
	{
	  BOOST_LOG_TRIVIAL (info) << "Error: " << ptrGrabResult->GetErrorCode () << " "
	      << ptrGrabResult->GetErrorDescription ();
	}
    }

}

void
BaslerHighResInterface::startSampling()
{

  BOOST_LOG_TRIVIAL (info) << "startSampling()";

     try
     {
        threadSamplingLoop = std::thread(&BaslerHighResInterface::startSamplingLoop, this);
     }
     catch (GenICam::GenericException &e)
     {
         // Error handling.
         cerr << "An exception occurred." << endl
         << e.GetDescription() << endl;
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
