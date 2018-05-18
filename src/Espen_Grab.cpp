/* openCVGrab: A sample program showing to convert Pylon images to opencv MAT.
	Copyright 2017 Matthew Breit <matt.breit@gmail.com>
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
	THIS SOFTWARE REQUIRES ADDITIONAL SOFTWARE (IE: LIBRARIES) IN ORDER TO COMPILE
	INTO BINARY FORM AND TO FUNCTION IN BINARY FORM. ANY SUCH ADDITIONAL SOFTWARE
	IS OUTSIDE THE SCOPE OF THIS LICENSE.
*/
// Include files to use OpenCV API.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>

#include <pylon/gige/BaslerGigEInstantCamera.h>

// Use sstream to create image names including integer
#include <sstream>
#include <thread>

#include <unistd.h>

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using GenApi objects
using namespace GenApi;

// Namespace for using opencv objects.
using namespace cv;

// Namespace for using cout.
using namespace std;

class BaslerHighResInterface
{
public:
  BaslerHighResInterface();
  void initialiseCamera();
  void stopSampling();
  void startSamplingLoop();
  void startSampling();
  void openForParameterManipulation();

  void setGain (int64_t value);
  int64_t getGain ();


private:

  CInstantCamera *camera;

  // Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
  // is initialized during the lifetime of this object.
  Pylon::PylonAutoInitTerm autoInitTerm;

  std::thread threadSamplingLoop;

  CBaslerGigEInstantCamera *cameraParameters;


};


// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 1000;

// The exit code of the sample application.
int exitCode = 0;

BaslerHighResInterface::BaslerHighResInterface(){

}

void BaslerHighResInterface::initialiseCamera() {
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











void BaslerHighResInterface::stopSampling()
{
  camera->StopGrabbing();
  if(threadSamplingLoop.joinable()){
      threadSamplingLoop.join();
  }


}


void BaslerHighResInterface::startSamplingLoop()
{

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

void BaslerHighResInterface::startSampling()
{
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
