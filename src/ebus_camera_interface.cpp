///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////



#include <iostream>

#include "../include/ebus_camera_interface.hpp"


#include <boost/program_options.hpp>

#include "i3ds/core/communication.hpp"
#include "i3ds/emulators/emulated_camera.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>





namespace po = boost::program_options;
namespace logging = boost::log;



EbusCameraInterface::EbusCameraInterface() {
  BOOST_LOG_TRIVIAL(info) << "EbusCameraInterface constructor";

 
}



bool EbusCameraInterface::connect() {
  BOOST_LOG_TRIVIAL(info) << "Connecting to camera";
  mConnectionID = "10.0.1.111";
  
  BOOST_LOG_TRIVIAL(info) << "--> ConnectDevice " << mConnectionID.GetAscii();

  // Connect to the selected Device
  PvResult lResult = PvResult::Code::INVALID_PARAMETER;
  mDevice = PvDevice::CreateAndConnect(mConnectionID, &lResult);
  if (!lResult.IsOK()) {
      BOOST_LOG_TRIVIAL(info) << "CreateAndConnect problem";
  	return false;
  }
  
  
  // Register this class as an event sink for PvDevice call-backs
  mDevice->RegisterEventSink(this);

  // Clear connection lost flag as we are now connected to the device
  mConnectionLost = false;
  BOOST_LOG_TRIVIAL(info) << "Connected to Camera";
  
  collectParameters();
  return true;
}
  
  
  

void EbusCameraInterface::OnLinkDisconnected(PvDevice *aDevice) {
  BOOST_LOG_TRIVIAL(info) << "=====> PvDeviceEventSink::OnLinkDisconnected callback";

  mConnectionLost = true;

	// IMPORTANT:
	// The PvDevice MUST NOT be explicitly disconnected from this callback.
	// Here we just raise a flag that we lost the device and let the main loop
	// of the application (from the main application thread) perform the
	// disconnect.
	//

}


void EbusCameraInterface::collectParameters() {
  BOOST_LOG_TRIVIAL(info) << "Collecting Camera parameters";
  lParameters = mDevice->GetParameters();
}



int64_t EbusCameraInterface::getParameter(PvString whichParameter) {

  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: " << whichParameter.GetAscii() ;

  PvGenParameter *lParameter = lParameters->Get(whichParameter);
  PvGenInteger *lIntParameter = dynamic_cast<PvGenInteger *>(lParameter);

  if (lIntParameter == NULL) {
    BOOST_LOG_TRIVIAL(info) << "Unable to get the parameter: "<< whichParameter.GetAscii();
  }

  // Read current width value.
  int64_t lParameterValue = 0;
  if (!(lIntParameter->GetValue(lParameterValue).IsOK())) {
    BOOST_LOG_TRIVIAL(info) << "Error retrieving parameter from device";
    return  0;
  } else {
    BOOST_LOG_TRIVIAL(info) << "Parametervalue: " << lParameterValue <<
		" returned from parameter: " << whichParameter.GetAscii();
    return lParameterValue;
  }
}



// Fetching minimum allowed value of parameter
int64_t EbusCameraInterface::getMinParameter(PvString whichParameter) {

	PvGenParameter *lParameter = lParameters->Get(whichParameter);

	// Converter generic parameter to width using dynamic cast. If the
	// type is right, the conversion will work otherwise lWidth will be NULL.
	PvGenInteger *lMinParameter = dynamic_cast<PvGenInteger *>(lParameter);

	if (lMinParameter == NULL) {
	    BOOST_LOG_TRIVIAL(info) << "Unable to get the Minimum parameter for: " 
		 << whichParameter.GetAscii();
	}

	int64_t lMinValue = 0;
	if (!(lMinParameter->GetMin(lMinValue).IsOK())) {
	    BOOST_LOG_TRIVIAL(info) << "Error retrieving minimum value from device";
	} else {
	    BOOST_LOG_TRIVIAL(info) << "Minimum value for parameter: " << whichParameter.GetAscii() 
		<< " : "<< lMinValue;
	    return lMinValue;
	}
}






// Get maximum allowed value of parameter.
int64_t EbusCameraInterface::getMaxParameter(PvString whichParameter) {

  PvGenParameter *lParameter = lParameters->Get(whichParameter);

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lMaxParameter = dynamic_cast<PvGenInteger *>(lParameter);

  if (lMaxParameter == NULL) {
      BOOST_LOG_TRIVIAL(info) << "Unable to get parameter " << whichParameter.GetAscii();
  }

  int64_t lMaxAllowedValue = 0;
  if (!(lMaxParameter->GetMax(lMaxAllowedValue).IsOK())) {
      BOOST_LOG_TRIVIAL(info) << "Error retrieving max value from device for parameter " <<
	  whichParameter.GetAscii();
      return 0;
  } else {
      BOOST_LOG_TRIVIAL(info) << "Max allowed value for parameter: " << whichParameter.GetAscii() 
	  <<  " is " << lMaxAllowedValue;
    return lMaxAllowedValue;
  }

}



char * EbusCameraInterface::getEnum(PvString whichParameter){

  PvGenParameter *lGenParameter = lParameters->Get(whichParameter);

  // Parameter available?
  if ( !lGenParameter->IsAvailable() )
  {
    BOOST_LOG_TRIVIAL(info) << "{Not Available}";
    return nullptr;
  }

  // Parameter readable?
  if ( !lGenParameter->IsReadable() )
  {
    BOOST_LOG_TRIVIAL(info) << "{Not readable}";
    return nullptr;
  }

  // Get the parameter type
  PvGenType lType;
  lGenParameter->GetType( lType );
  if(lType == PvGenTypeEnum)
  {
    PvString lValue;
    static_cast<PvGenEnum *>( lGenParameter )->GetValue( lValue );
    BOOST_LOG_TRIVIAL(info) << "Enum: " << lValue.GetAscii();
    char * str;
    
    asprintf(&str, "%s", lValue.GetAscii());
    return str;
		    
			
        }else
        {
          BOOST_LOG_TRIVIAL(info) << "Not a enum ";
          return nullptr;
        }

}


bool EbusCameraInterface::checkIfEnumOptionIsOK(PvString whichParameter, PvString value){
  BOOST_LOG_TRIVIAL(info) << "checkIfEnumOptionIsOK: Parameter: " <<  whichParameter.GetAscii()
      << "Value: " << value.GetAscii();
			
  //To pull out enums options
  
  int64_t aCount;
  
  auto *lGenParameter = lParameters->Get(whichParameter);
  static_cast<PvGenEnum *>( lGenParameter )->GetEntriesCount (aCount);
  BOOST_LOG_TRIVIAL(info) << "Enum entries: " << aCount;
  
  for(int i=0; i<aCount;i++){
  
    const PvGenEnumEntry *aEntry;

    static_cast<PvGenEnum *>( lGenParameter )->GetEntryByValue(i, &aEntry);
    PvString enumOption;
    aEntry->GetName(enumOption);
    BOOST_LOG_TRIVIAL(info) << "EnumText["<< i << "]: "<< enumOption.GetAscii();
    if(enumOption == value){
	BOOST_LOG_TRIVIAL(info) << "Option found.";
	return true;
    }
  }
  BOOST_LOG_TRIVIAL(info) << "Option not found.";
  return false;

}


void EbusCameraInterface::setEnum(PvString whichParameter, PvString value) {
  BOOST_LOG_TRIVIAL(info) << "setEnum: Parameter: " <<  whichParameter.GetAscii() 
      << "Value: " << value.GetAscii();
  BOOST_LOG_TRIVIAL(info) << "do checkIfEnumOptionIsOK: Parameter first";
  
  if(checkIfEnumOptionIsOK( whichParameter, value)){

      PvGenParameter *lParameter = lParameters->Get(whichParameter);
      auto *lEnumParameter = dynamic_cast<PvGenEnum *>(lParameter);

      if (lEnumParameter == NULL) {
	BOOST_LOG_TRIVIAL(info) << "Unable to get the parameter: "<< whichParameter.GetAscii();
      }

     
      if (!(lEnumParameter->SetValue(value).IsOK())) {
        BOOST_LOG_TRIVIAL(info) << "Error setting parameter for device";
        return;
      } else {
        BOOST_LOG_TRIVIAL(info) << "Parameter value: " << value.GetAscii() <<
    		" set for parameter: " << whichParameter.GetAscii();
        return;
      }
      
      
  }else{
      BOOST_LOG_TRIVIAL(info) << "Illegal parameter value";
  }  
}


bool EbusCameraInterface::getBooleanParameter(PvString whichParameter) {
  PvGenParameter *lParameter = lParameters->Get(whichParameter);

  bool lValue;
  static_cast<PvGenBoolean *>( lParameter )->GetValue( lValue );
  
  if( lValue )
  {
    BOOST_LOG_TRIVIAL(info) << whichParameter.GetAscii() << "Boolean: TRUE";
    return true;
  }
  else
  {
    BOOST_LOG_TRIVIAL(info) << whichParameter.GetAscii() << "Boolean: FALSE";
    return true;
  }
}



// Sets an integer Parameter on device
bool EbusCameraInterface::setIntParameter(PvString whichParameter, int64_t value) {
{
  PvGenParameter *lParameter = lParameters->Get(whichParameter);

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lvalueParameter = dynamic_cast<PvGenInteger *>(lParameter);

  if (lvalueParameter == NULL) {
    BOOST_LOG_TRIVIAL(info) << "SetParameter: Unable to get the parameter: " <<
	  whichParameter.GetAscii();
  }

  int64_t max = getMaxParameter(whichParameter);
  if (value > max) {
    BOOST_LOG_TRIVIAL(info) << "Setting value Error: Parameter: " <<
	  whichParameter.GetAscii() << " value to big " << value <<  "> " << max;
    BOOST_LOG_TRIVIAL(info) <<"Rounding to max";
    value = max;
  };

  int64_t min = getMinParameter(whichParameter);
  if (value < min){
    BOOST_LOG_TRIVIAL(info) << "Error: value to small " << value << "<" << min;
    BOOST_LOG_TRIVIAL(info) <<"Rounding to min";
    value = min;	
  };
  /// TODO: Increment also?


  PvResult res = lvalueParameter->SetValue(value);
  if (res.IsOK() != true) {
      BOOST_LOG_TRIVIAL(info) << "SetValue Error: " << whichParameter.GetAscii();
      return false;
  } else {
      BOOST_LOG_TRIVIAL(info) << "SetValue Ok: " << whichParameter.GetAscii() << "=" << value;
      return true;
  }

}





}


 
// Todo This is actually a boolean!!!!
bool EbusCameraInterface::getAutoExposureEnabled(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: getAutoExposureEnabled";

  char *str = getEnum("AutoExposure");
  bool retval;

  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: getAutoExposureEnabled: " << str;

  if(strcmp (str, "ON")==0){
   retval = true;
  }
   else{
   retval = false;
  }
  BOOST_LOG_TRIVIAL(info) << retval;

  delete(str);
  return retval;
}


bool EbusCameraInterface::getRegionEnabled(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: RegionEnabled";
  return true;
}



int64_t EbusCameraInterface::getExposure(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: exposure"; 
  return getParameter("GainValueLeft");
}

int64_t EbusCameraInterface::getMaxExposure(){

  BOOST_LOG_TRIVIAL(info) << "Fetching max of parameter: exposure"; 
  return getMaxParameter("GainValueLeft");  
}


int64_t EbusCameraInterface::getGain(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: GainValue";
  return getParameter("GainValue");
}


int64_t EbusCameraInterface::getMaxGain(){

   BOOST_LOG_TRIVIAL(info) << "Fetching max of parameter: GainValue"; 
   return getMaxParameter("GainValue");  
}



PlanarRegion EbusCameraInterface::getRegion(){

  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: region";

  int64_t size_x = getParameter("Width");
  int64_t size_y = getParameter("Height");
  int64_t offset_x = getParameter("OffsetY");
  int64_t offset_y = getParameter("RegionHeight");
  
  PlanarRegion region;
   
  region.offset_x = offset_x;
  region.offset_y = offset_y;
  region.size_x = size_x;
  region.size_y= size_y;  

  return region;
}



void EbusCameraInterface::setTriggerInterval(int64_t value) {
  BOOST_LOG_TRIVIAL(info) << "Set TriggerInterval: " << value;
}


bool EbusCameraInterface::checkTriggerInterval(int64_t value) {
  BOOST_LOG_TRIVIAL(info) << "Check TriggerInterval: " << value;
  return true;
}


void EbusCameraInterface::setAutoExposure(int64_t value) {
  BOOST_LOG_TRIVIAL(info) << "Set Exposure (ShutterTimeValue): " << value;
  setIntParameter("ShutterTimeValue", value);
}


void EbusCameraInterface::setGain(int64_t value) {
  BOOST_LOG_TRIVIAL(info) << "Set Gain(GainValue): " << value;
  setIntParameter("GainValue", value);
}


void EbusCameraInterface::setAutoExposureEnabled(bool enabled){
  BOOST_LOG_TRIVIAL(info) << "Set setAutoExposureEnabled(AutoExposure): " << enabled;
  if(enabled) {
    setEnum("AutoExposure", "ON");
  }else{
    setEnum("AutoExposure", "OFF");
    }
  
}

void EbusCameraInterface::setRegionEnabled(bool regionEnabled){
  
  BOOST_LOG_TRIVIAL(info) << "Set setRegionEnabled (Manually done): " << regionEnabled;
  
  if(regionEnabled){
      ;
  }else{
     // getIntParameter("Width");
      //getIntParameter("Height");
      setIntParameter("OffsetY", 0);
      setIntParameter("RegionHeight", 0);
      
  }
}
void EbusCameraInterface::setRegion(PlanarRegion region){
    
  BOOST_LOG_TRIVIAL(info) << " setRegion (Manually done): ";

  int64_t size_x = getParameter("Width");
  int64_t size_y = getParameter("Height");
  int64_t offset_x = getParameter("OffsetY");
  int64_t offset_y = getParameter("RegionHeight");
  
  
     /*
    setregion.offset_x = offset_x;
    if(region.)
    min =
    
    
    region.offset_y = offset_y;
    region.size_x = size_x;
    region.size_y= size_y;  

    return region;
  */
}








-/////-------------------------------- Streaming part


//
// Opens stream, pipeline, allocates buffers
//

bool EBusInterface::OpenStream() {
	std::cout << "--> OpenStream" << std::endl;

	// Creates and open the stream object based on the selected device.
	PvResult lResult = PvResult::Code::INVALID_PARAMETER;
	mStream = PvStream::CreateAndOpen(mConnectionID, &lResult);
	if (!lResult.IsOK()) {
		std::cout << "Unable to open the stream" << std::endl;
		return false;
	}

	mPipeline = new PvPipeline(mStream);

	// Reading payload size from device
	int64_t lSize = mDevice->GetPayloadSize();

	// Create, init the PvPipeline object
	mPipeline->SetBufferSize(static_cast<uint32_t>(lSize));
	mPipeline->SetBufferCount(16);

	// The pipeline needs to be "armed", or started before  we instruct the device to send us images
	lResult = mPipeline->Start();
	if (!lResult.IsOK()) {
		std::cout << "Unable to start pipeline" << std::endl;
		return false;
	}

	// Only for GigE Vision, if supported
	PvGenBoolean *lRequestMissingPackets =
			dynamic_cast<PvGenBoolean *>(mStream->GetParameters()->GetBoolean(
					"RequestMissingPackets"));
	if ((lRequestMissingPackets != NULL)
			&& lRequestMissingPackets->IsAvailable()) {
		// Disabling request missing packets.
		lRequestMissingPackets->SetValue(false);
	}

	return true;
}

//
// Closes the stream, pipeline
//

void EBusInterface::CloseStream() {
	std::cout << "--> CloseStream" << std::endl;

	if (mPipeline != NULL) {
		if (mPipeline->IsStarted()) {
			if (!mPipeline->Stop().IsOK()) {
				std::cout << "Unable to stop the pipeline." << std::endl;
			}
		}

		delete mPipeline;
		mPipeline = NULL;
	}

	if (mStream != NULL) {
		if (mStream->IsOpen()) {
			if (!mStream->Close().IsOK()) {
				std::cout << "Unable to stop the stream." << std::endl;
			}
		}

		PvStream::Free(mStream);
		mStream = NULL;
	}
}

//
// Starts image acquisition
//

bool EBusInterface::StartAcquisition() {
	std::cout << "--> StartAcquisition" << std::endl;

	// Flush packet queue to make sure there is no left over from previous disconnect event
	PvStreamGEV* lStreamGEV = dynamic_cast<PvStreamGEV*>(mStream);
	if (lStreamGEV != NULL) {
		lStreamGEV->FlushPacketQueue();
	}

	// Set streaming destination (only GigE Vision devces)
	PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*>(mDevice);
	if (lDeviceGEV != NULL) {
		// If using a GigE Vision, it is same to assume the stream object is GigE Vision as well
		PvStreamGEV* lStreamGEV = static_cast<PvStreamGEV*>(mStream);

		// Have to set the Device IP destination to the Stream
		PvResult lResult = lDeviceGEV->SetStreamDestination(
				lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort());
		if (!lResult.IsOK()) {
			std::cout << "Setting stream destination failed" << std::endl;
			return false;
		}
	}

	// Enables stream before sending the AcquisitionStart command.
	mDevice->StreamEnable();

	// The pipeline is already "armed", we just have to tell the device to start sending us images
	PvResult lResult = mDevice->GetParameters()->ExecuteCommand(
			"AcquisitionStart");
	if (!lResult.IsOK()) {
		std::cout << "Unable to start acquisition" << std::endl;
		return false;
	}

	return true;
}

//
// Stops acquisition
//

bool EBusInterface::StopAcquisition() {
	std::cout << "--> StopAcquisition" << std::endl;

	// Tell the device to stop sending images.
	mDevice->GetParameters()->ExecuteCommand("AcquisitionStop");

	// Disable stream after sending the AcquisitionStop command.
	mDevice->StreamDisable();

	PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*>(mDevice);
	if (lDeviceGEV != NULL) {
		// Reset streaming destination (optional...)
		lDeviceGEV->ResetStreamDestination();
	}

	return true;
}

//
// Acquisition loop
//

void EBusInterface::ApplicationLoop() {
	std::cout << "--> ApplicationLoop" << std::endl;

	char lDoodle[] = "|\\-|-/";
	int lDoodleIndex = 0;
	bool lFirstTimeout = true;

	int64_t lImageCountVal = 0;
	double lFrameRateVal = 0.0;
	double lBandwidthVal = 0.0;

	// Acquire images until the user instructs us to stop.
	while (!PvKbHit()) {
		// If connection flag is up, teardown device/stream
		if (mConnectionLost && (mDevice != NULL)) {
			// Device lost: no need to stop acquisition
			TearDown(false);
		}

		// If the device is not connected, attempt reconnection
		if (mDevice == NULL) {
			if (ConnectDevice()) {
				// Device is connected, open the stream
				if (OpenStream()) {
					// Device is connected, stream is opened: start acquisition
					if (!StartAcquisition()) {
						TearDown(false);
					}
				} else {
					TearDown(false);
				}
			}
		}

		// If still no device, no need to continue the loop
		if (mDevice == NULL) {
			continue;
		}

		if ((mStream != NULL) && mStream->IsOpen() && (mPipeline != NULL)
				&& mPipeline->IsStarted()) {
			// Retrieve next buffer
			PvBuffer *lBuffer = NULL;
			PvResult lOperationResult;
			PvResult lResult = mPipeline->RetrieveNextBuffer(&lBuffer, 1000,
					&lOperationResult);

			if (lResult.IsOK()) {
				if (lOperationResult.IsOK()) {
					//
					// We now have a valid buffer. This is where you would typically process the buffer.
					// -----------------------------------------------------------------------------------------
					// ...

					mStream->GetParameters()->GetIntegerValue("BlockCount",
							lImageCountVal);
					mStream->GetParameters()->GetFloatValue("AcquisitionRate",
							lFrameRateVal);
					mStream->GetParameters()->GetFloatValue("Bandwidth",
							lBandwidthVal);

					// If the buffer contains an image, display width and height.
					uint32_t lWidth = 0, lHeight = 0;
					if (lBuffer->GetPayloadType() == PvPayloadTypeImage) {
						// Get image specific buffer interface.
						PvImage *lImage = lBuffer->GetImage();

						// Read width, height.
						lWidth = lImage->GetWidth();
						lHeight = lImage->GetHeight();
					}

					std::cout << fixed << setprecision(1);
					std::cout << lDoodle[lDoodleIndex];
					std::cout << " BlockID: " << uppercase << hex
							<< setfill('0') << setw(16) << lBuffer->GetBlockID()
							<< " W: " << dec << lWidth << " H: " << lHeight
							<< " " << lFrameRateVal << " FPS "
							<< (lBandwidthVal / 1000000.0) << " Mb/s  \r";

					lFirstTimeout = true;
				}
				// We have an image - do some processing (...) and VERY IMPORTANT,
				// release the buffer back to the pipeline.
				mPipeline->ReleaseBuffer(lBuffer);
			} else {
				// Timeout

				if (lFirstTimeout) {
					std::cout << "" << std::endl;
					lFirstTimeout = false;
				}

				std::cout << "Image timeout " << lDoodle[lDoodleIndex]
						<< std::endl;
			}

			++lDoodleIndex %= 6;
		} else {
			// No stream/pipeline, must be in recovery. Wait a bit...
			PvSleepMs(100);
		}
	}

	PvGetChar(); // Flush key buffer for next stop.
	std::cout << "" << std::endl;
}

//
// \brief Disconnects the device
//

void EBusInterface::DisconnectDevice() {
	std::cout << "--> DisconnectDevice" << std::endl;

	if (mDevice != NULL) {
		if (mDevice->IsConnected()) {
			// Unregister event sink (call-backs).
			mDevice->UnregisterEventSink(this);
		}

		PvDevice::Free(mDevice);
		mDevice = NULL;
	}
}

//
// Tear down: closes, disconnects, etc.
//

void EBusInterface::TearDown(bool aStopAcquisition) {
	std::cout << "--> TearDown" << std::endl;

	if (aStopAcquisition) {
		StopAcquisition();
	}

	CloseStream();
	DisconnectDevice();
}

//
// PvDeviceEventSink callback
//
// Notification that the device just got disconnected.
//




