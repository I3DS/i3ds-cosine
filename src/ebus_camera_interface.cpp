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
#include <thread>
#include <chrono>

#include "i3ds/exception.hpp"
#include "i3ds/communication.hpp"
#include "i3ds/emulated_camera.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <PvSampleUtils.h>

#include <string>


namespace po = boost::program_options;
namespace logging = boost::log;

int gStop;

EbusCameraInterface::EbusCameraInterface(const char *connectionString, const char *camera_name, bool free_running, Operation operation)
: ip_address_(connectionString), free_running_(free_running), operation_(operation)
{
    
  BOOST_LOG_TRIVIAL (info) << "EbusCameraInterface constructor";
  mConnectionID = PvString(camera_name);
  
  //mConnectionID = PvString(connectionString); // Similar to use different connection strings 
  BOOST_LOG_TRIVIAL (info) << "Connection String: "<< mConnectionID.GetAscii ();

}



bool
EbusCameraInterface::connect ()
{
  BOOST_LOG_TRIVIAL (info) << "Connecting to camera";
  //mConnectionID = "10.0.1.111";

  BOOST_LOG_TRIVIAL (info) << "--> ConnectDevice " << mConnectionID.GetAscii ();

  // Connect to the selected Device
  PvResult lResult = PvResult::Code::INVALID_PARAMETER;
  mDevice = PvDevice::CreateAndConnect (mConnectionID, &lResult);
  if (!lResult.IsOK ())
    {
      BOOST_LOG_TRIVIAL (info) << "CreateAndConnect problem";
      return false;
    }

  // Register this class as an event sink for PvDevice call-backs
  mDevice->RegisterEventSink (this);

  // Clear connection lost flag as we are now connected to the device
  mConnectionLost = false;
  BOOST_LOG_TRIVIAL (info) << "Connected to Camera";

  PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*> (mDevice);
  fetched_ipaddress = lDeviceGEV->GetIPAddress();
  BOOST_LOG_TRIVIAL (info) << "IP ADDRESS got from camera" << fetched_ipaddress.GetAscii();
  
  
  collectParameters ();
  trigger_interval_value_ = getParameter ("TriggerInterval");
  return true;
}

void
EbusCameraInterface::OnLinkDisconnected (PvDevice *aDevice)
{
  BOOST_LOG_TRIVIAL (info)
      << "=====> PvDeviceEventSink::OnLinkDisconnected callback";

  mConnectionLost = true;

  // IMPORTANT:
  // The PvDevice MUST NOT be explicitly disconnected from this callback.
  // Here we just raise a flag that we lost the device and let the main loop
  // of the application (from the main application thread) perform the
  // disconnect.
  //

}

void
EbusCameraInterface::collectParameters ()
{
  BOOST_LOG_TRIVIAL (info) << "Collecting Camera parameters";
  lParameters = mDevice->GetParameters ();
}

int64_t
EbusCameraInterface::getParameter (PvString whichParameter)
{

  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: "
      << whichParameter.GetAscii ();
  PvGenParameter *lParameter = lParameters->Get (whichParameter);
  PvGenInteger *lIntParameter = dynamic_cast<PvGenInteger *> (lParameter);

  if (lIntParameter == NULL)
    {
      BOOST_LOG_TRIVIAL (info) << "Unable to get the parameter: "
	  << whichParameter.GetAscii ();
      ostringstream errorDescription;
      errorDescription << "getParameter: Unable to get the parameter: " << whichParameter.GetAscii ();
      throw i3ds::CommandError(error_value, errorDescription.str());
    }

  // Read current width value.
  int64_t lParameterValue = 0;
  if (!(lIntParameter->GetValue (lParameterValue).IsOK ()))
    {
      BOOST_LOG_TRIVIAL (info) << "Error retrieving parameter from device";
      ostringstream errorDescription;
      errorDescription << "getParameter: Error retrieving parameter from device" << whichParameter.GetAscii ();
      throw i3ds::CommandError(error_value, errorDescription.str());
      return 0;
    }
  else
    {
      BOOST_LOG_TRIVIAL (info) << "Parametervalue: " << lParameterValue
	  << " returned from parameter: " << whichParameter.GetAscii ();
      return lParameterValue;
    }
}

// Fetching minimum allowed value of parameter
int64_t
EbusCameraInterface::getMinParameter (PvString whichParameter)
{

  PvGenParameter *lParameter = lParameters->Get (whichParameter);

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lMinParameter = dynamic_cast<PvGenInteger *> (lParameter);

  if (lMinParameter == NULL)
    {
      BOOST_LOG_TRIVIAL (info) << "Unable to get the Minimum parameter for: "
	  << whichParameter.GetAscii ();
    }

  int64_t lMinValue = 0;
  if (!(lMinParameter->GetMin (lMinValue).IsOK ()))
    {
      BOOST_LOG_TRIVIAL (info) << "Error retrieving minimum value from device";
    }
  else
    {
      BOOST_LOG_TRIVIAL (info) << "Minimum value for parameter: "
	  << whichParameter.GetAscii () << " : " << lMinValue;
      return lMinValue;
    }
}

// Get maximum allowed value of parameter.
int64_t
EbusCameraInterface::getMaxParameter (PvString whichParameter)
{

  PvGenParameter *lParameter = lParameters->Get (whichParameter);

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lMaxParameter = dynamic_cast<PvGenInteger *> (lParameter);

  if (lMaxParameter == NULL)
    {
      BOOST_LOG_TRIVIAL (info) << "Unable to get parameter "
	  << whichParameter.GetAscii ();
    }

  int64_t lMaxAllowedValue = 0;
  if (!(lMaxParameter->GetMax (lMaxAllowedValue).IsOK ()))
    {
      BOOST_LOG_TRIVIAL (info)
	  << "Error retrieving max value from device for parameter "
	  << whichParameter.GetAscii ();
      return 0;
    }
  else
    {
      BOOST_LOG_TRIVIAL (info) << "Max allowed value for parameter: "
	  << whichParameter.GetAscii () << " is " << lMaxAllowedValue;
      return lMaxAllowedValue;
    }

}


char *
EbusCameraInterface::getEnum (PvString whichParameter)
{

  PvGenParameter *lGenParameter = lParameters->Get (whichParameter);

  // Parameter available?
  if (!lGenParameter->IsAvailable ())
    {
      BOOST_LOG_TRIVIAL (info) << "{Not Available}";
      ostringstream errorDescription;
      errorDescription << "getEnum: Parameter Not Available: " << whichParameter.GetAscii ();
      throw i3ds::CommandError(error_value, errorDescription.str());
      return nullptr;
    }

  // Parameter readable?
  if (!lGenParameter->IsReadable ())
    {
      BOOST_LOG_TRIVIAL (info) << "{Not readable}";
      ostringstream errorDescription;
      errorDescription << "getEnum: Parameter Not Readable: " << whichParameter.GetAscii ();
      throw i3ds::CommandError(error_value, errorDescription.str());
      return nullptr;
    }

  // Get the parameter type
  PvGenType lType;
  lGenParameter->GetType (lType);
  if (lType == PvGenTypeEnum)
    {
      PvString lValue;
      static_cast<PvGenEnum *> (lGenParameter)->GetValue (lValue);
      BOOST_LOG_TRIVIAL (info) << "Enum: " << lValue.GetAscii ();
      char * str;

      asprintf (&str, "%s", lValue.GetAscii ());
      return str;

    }
  else
    {
      BOOST_LOG_TRIVIAL (info) << "Not a enum ";
      ostringstream errorDescription;
      errorDescription << "getEnum: Parameter Not a enum: " << whichParameter.GetAscii ();
      throw i3ds::CommandError(error_value, errorDescription.str());
      return nullptr;
    }

}

bool
EbusCameraInterface::checkIfEnumOptionIsOK (PvString whichParameter,
					    PvString value)
{
  BOOST_LOG_TRIVIAL (info) << "checkIfEnumOptionIsOK: Parameter: "
      << whichParameter.GetAscii () << "Value: " << value.GetAscii ();

  //To pull out enums options

  int64_t aCount;

  auto *lGenParameter = lParameters->Get (whichParameter);
  static_cast<PvGenEnum *> (lGenParameter)->GetEntriesCount (aCount);
  BOOST_LOG_TRIVIAL (info) << "Enum entries: " << aCount;

  for (int i = 0; i < aCount; i++)
    {


      const PvGenEnumEntry *aEntry;

      static_cast<PvGenEnum *> (lGenParameter)->GetEntryByValue (i, &aEntry);
      PvString enumOption;
      aEntry->GetName (enumOption);
      BOOST_LOG_TRIVIAL (info) << "EnumText[" << i << "]: "
	  << enumOption.GetAscii ();
      if (enumOption == value)
	{
	  BOOST_LOG_TRIVIAL (info) << "Option found.";
	  return true;
	}
    }
  BOOST_LOG_TRIVIAL (info) << "Option not found.";
  ostringstream errorDescription;
  errorDescription << "checkEnum: Option: "<< value.GetAscii() << " does not exists for parameter: "
      << whichParameter.GetAscii ();
  throw i3ds::CommandError(error_value, errorDescription.str());
  return false;

}


void
EbusCameraInterface::setEnum (PvString whichParameter, PvString value, bool dontCheckParameter)
{
  BOOST_LOG_TRIVIAL (info) << "setEnum: Parameter: "
      << whichParameter.GetAscii () << "Value: " << value.GetAscii ();
  BOOST_LOG_TRIVIAL (info) << "do checkIfEnumOptionIsOK: Parameter first";

  bool enumOK = true;
  if(dontCheckParameter == false)
    {
      enumOK = checkIfEnumOptionIsOK (whichParameter, value);
    }

  // enumOK== true if not options not checked.
  if (enumOK)
    {

      PvGenParameter *lParameter = lParameters->Get (whichParameter);
      auto *lEnumParameter = dynamic_cast<PvGenEnum *> (lParameter);

      if (lEnumParameter == NULL)
	{
	  BOOST_LOG_TRIVIAL (info) << "Unable to get the parameter: " << whichParameter.GetAscii ();
	  ostringstream errorDescription;
	  errorDescription << "setEnum: Unable to get parameter: " << whichParameter.GetAscii ();
	  throw i3ds::CommandError(error_value, errorDescription.str());
	}

      if (!(lEnumParameter->SetValue (value).IsOK ()))
	{
	  BOOST_LOG_TRIVIAL (info) << "Error setting parameter for device";
	  ostringstream errorDescription;
	  errorDescription << "setEnum: Error setting value: "<< value.GetAscii() <<
	      " for parameter: " << whichParameter.GetAscii();
	  throw i3ds::CommandError(error_value, errorDescription.str());
	  return;
	}
      else
	{
	  BOOST_LOG_TRIVIAL (info) << "Parameter value: " << value.GetAscii()
	      << " set for parameter: " << whichParameter.GetAscii();
	  return;

	}

    }
  else
    {
      BOOST_LOG_TRIVIAL (info) << "Illegal parameter value";
      ostringstream errorDescription;
      errorDescription << "setEnum: Illegal parameter value: "<< value.GetAscii() <<
	  " for parameter: " << whichParameter.GetAscii();
      throw i3ds::CommandError(error_value, errorDescription.str());
    }
}

bool
EbusCameraInterface::getBooleanParameter (PvString whichParameter)
{
  PvGenParameter *lParameter = lParameters->Get (whichParameter);

  bool lValue;
  static_cast<PvGenBoolean *> (lParameter)->GetValue (lValue);

  if (lValue)
    {
      BOOST_LOG_TRIVIAL (info) << whichParameter.GetAscii () << "Boolean: TRUE";
      return true;
    }
  else
    {
      BOOST_LOG_TRIVIAL (info) << whichParameter.GetAscii ()
	  << "Boolean: FALSE";
      return true;
    }
}

// Sets an integer Parameter on device
bool
EbusCameraInterface::setIntParameter (PvString whichParameter, int64_t value)
{
    {
      PvGenParameter *lParameter = lParameters->Get (whichParameter);

      // Converter generic parameter to width using dynamic cast. If the
      // type is right, the conversion will work otherwise lWidth will be NULL.
      PvGenInteger *lvalueParameter = dynamic_cast<PvGenInteger *> (lParameter);
      if (lvalueParameter == NULL)
	{
	  BOOST_LOG_TRIVIAL (info)
	      << "SetParameter: Unable to get the parameter: "
	      << whichParameter.GetAscii ();
	  ostringstream errorDescription;
	  errorDescription << "setIntParameter Option: SetParameter: Unable to get the parameter: "
	      << whichParameter.GetAscii ();
	  throw i3ds::CommandError(error_value, errorDescription.str());
	}

      int64_t max = getMaxParameter (whichParameter);
      if (value > max)
	{
	  BOOST_LOG_TRIVIAL (info) << "Setting value Error: Parameter: "
	      << whichParameter.GetAscii () << " value too big " << value << " (Max: "
	      << max <<")";

	  ostringstream errorDescription;
	  errorDescription << "setIntParameter: "<< whichParameter.GetAscii() << " value to large " <<
	      value << ".(Max: " << max <<")" ;
	  throw i3ds::CommandError(error_value, errorDescription.str());



	  BOOST_LOG_TRIVIAL (info) << "Rounding to max";
	  value = max;
	};

      int64_t min = getMinParameter (whichParameter);
      if (value < min)
	{
	  BOOST_LOG_TRIVIAL (info) << "Error: value to small " << value << "<"
	      << min;
	  ostringstream errorDescription;
	  errorDescription << "setIntParameter: "<< whichParameter.GetAscii() << " Value to small " <<
	        value << ".(Min: " << min <<")";
	  throw i3ds::CommandError(error_value, errorDescription.str());





	  BOOST_LOG_TRIVIAL (info) << "Rounding to min";
	  value = min;
	};
      /// TODO: Increment also?

      PvResult res = lvalueParameter->SetValue (value);
      if (res.IsOK () != true)
	{
	  BOOST_LOG_TRIVIAL (info) << "SetValue Error: "
	      << whichParameter.GetAscii ();
	  ostringstream errorDescription;
	  errorDescription << "setIntParameter: SetValue Error "<< whichParameter.GetAscii() ;
	  throw i3ds::CommandError(error_value, errorDescription.str());


	  return false;
	}
      else
	{
	  BOOST_LOG_TRIVIAL (info) << "SetValue Ok: "
	      << whichParameter.GetAscii () << "=" << value;
	  return true;
	}

    }

}


/// \REMARK SourceSelector parameter does not return the Option All with getEnum.
/// It also mixes the strings. But one can set it.
/// \TODO How is it handled in ebusplayer?
void
EbusCameraInterface::setSourceBothStreams()
{
  getEnum("SourceSelector");
  setEnum ("SourceSelector", "All", true);
}




// Todo This is actually a boolean!!!!
bool
EbusCameraInterface::getAutoExposureEnabled ()
{
  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: getAutoExposureEnabled";

  char *str = getEnum ("AutoExposure");
  bool retval;

  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: getAutoExposureEnabled: "
      << str;

  if (strcmp (str, "ON") == 0)
    {
      retval = true;
    }
  else
    {
      retval = false;
    }
  BOOST_LOG_TRIVIAL (info) << retval;

  delete (str);
  return retval;
}

bool
EbusCameraInterface::getRegionEnabled ()
{
  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: RegionEnabled";
  return true;
}

int64_t
EbusCameraInterface::getShutterTime ()
{
  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: exposure";
  return getParameter ("ShutterTimeValue");
}

int64_t
EbusCameraInterface::getMaxShutterTime ()
{

  BOOST_LOG_TRIVIAL (info) << "Fetching max of parameter: MaxShutterTime";
  return getMaxParameter ("MaxShutterTimeValue");
}

int64_t
EbusCameraInterface::getGain ()
{
  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: GainValue";
  return getParameter ("GainValue");
}
/*
int64_t
EbusCameraInterface::getMaxGain ()
{

  BOOST_LOG_TRIVIAL (info) << "Fetching max of parameter: GainValue";
  return getMaxParameter ("GainValue");
}
*/


PlanarRegion
EbusCameraInterface::getRegion ()
{

  BOOST_LOG_TRIVIAL (info) << "Fetching parameter: region";

  int64_t size_x = getParameter ("Width");
  int64_t size_y = getParameter ("Height");
  int64_t offset_x = getParameter ("OffsetY");
  int64_t offset_y = getParameter ("RegionHeight");

  PlanarRegion region;

  region.offset_x = offset_x;
  region.offset_y = offset_y;
  region.size_x = size_x;
  region.size_y = size_y;

  return region;
}


#ifdef HR_CAMERA
const int trigger_interval_factor = 2;

#elif defined(STEREO_CAMERA)
const int trigger_interval_factor = 30;
#else
// The TIR Camera
const int trigger_interval_factor = 30;
#endif



void
EbusCameraInterface::setTriggerInterval ()
{
  BOOST_LOG_TRIVIAL (info) << "Set TriggerInterval: trigger_interval_value_: " << trigger_interval_value_; 
  setIntParameter ("TriggerInterval", trigger_interval_value_);
  
}



bool
EbusCameraInterface::checkTriggerInterval (int64_t period_us)
{
  BOOST_LOG_TRIVIAL (info) << "Check TriggerInterval: Period: " << period_us <<" which equals "
      << 1e6/period_us<< "Hz";
  
  int trigger_interval_value = period_us*trigger_interval_factor/1.0e6;
  

   int min =  getMinParameter ("TriggerInterval");
   int max =  getMaxParameter ("TriggerInterval");
   BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval: min: " << min << " max:" << max;
   BOOST_LOG_TRIVIAL (info) << "Triggerinterval value tested: " << trigger_interval_value;

   if(trigger_interval_value < min || trigger_interval_value > max)
     {
       BOOST_LOG_TRIVIAL (info) << "checkTriggerInterval out of range";
       return false;
     }
   BOOST_LOG_TRIVIAL (info) << "TriggerInterval OK";
   trigger_interval_value_ = trigger_interval_value;
   return true;
  

}

void
EbusCameraInterface::setShutterTime (int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "SetShutterTime (ShutterTimeValue): " << value;
  setIntParameter ("ShutterTimeValue", value);
}



void
EbusCameraInterface::setMaxShutterTime (int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "set MaxShutterTime (MaxShutterTimeValue): " << value;
  setIntParameter ("MaxShutterTimeValue", value);
}


void
EbusCameraInterface::setGain (int64_t value)
{
  BOOST_LOG_TRIVIAL (info) << "Set Gain(GainValue): " << value;
  setIntParameter ("GainValue", value);
}

void
EbusCameraInterface::setAutoExposureEnabled (bool enabled)
{
  BOOST_LOG_TRIVIAL (info) << "Set setAutoExposureEnabled(AutoExposure): "
      << enabled;
  if (enabled)
    {
      setEnum ("AutoExposure", "ON");
    }
  else
    {
      setEnum ("AutoExposure", "OFF");
    }

}

void
EbusCameraInterface::setRegionEnabled (bool regionEnabled)
{

  BOOST_LOG_TRIVIAL (info) << "Set setRegionEnabled (Manually done): "
      << regionEnabled;

  if (regionEnabled)
    {
      ;
    }
  else
    {
      // getIntParameter("Width");
      //getIntParameter("Height");
      setIntParameter ("OffsetY", 0);
      setIntParameter ("RegionHeight", 0);

    }
}
void
EbusCameraInterface::setRegion (PlanarRegion region)
{

  BOOST_LOG_TRIVIAL (info) << " setRegion (Manually done): ";

  int64_t size_x = getParameter ("Width");
  int64_t size_y = getParameter ("Height");
  int64_t offset_x = getParameter ("OffsetY");
  int64_t offset_y = getParameter ("RegionHeight");

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



void
EbusCameraInterface::do_start ()
{
  PV_SAMPLE_INIT ();
  BOOST_LOG_TRIVIAL (info) << "--> EbusCameraInterface::do_start";
  // Set some parameters to be able to stream continuous
  if(free_running_){
      setEnum ("AcquisitionMode", "Continuous");
      setEnum ("TriggerMode", "Interval");
      setIntParameter ("TriggerInterval", trigger_interval_value_);
      BOOST_LOG_TRIVIAL (info)  << " trigger_interval_value_:"<< trigger_interval_value_;
      BOOST_LOG_TRIVIAL (info)  << " trigger_interval_factor:"<< trigger_interval_factor;

      samplingsTimeout_ = 1000.*(float)trigger_interval_value_ / (float)trigger_interval_factor*2.0;
      BOOST_LOG_TRIVIAL (info)  << " Tid:"<< samplingsTimeout_;
  }
  else{
      setEnum ("AcquisitionMode", "SingleFrame");
      setEnum ("TriggerMode", "EXT");
  }
  stopSamplingLoop = false;
  threadSamplingLoop = std::thread(&EbusCameraInterface::StartSamplingLoop, this);

 // TearDown (true);
}

/////-------------------------------- Streaming part

//
// Opens stream, pipeline, allocates buffers
//

bool
EbusCameraInterface::OpenStream ()
{
  BOOST_LOG_TRIVIAL (info) << "--> OpenStream: "<< mConnectionID.GetAscii ();

  // Creates and open the stream object based on the selected device.
  PvResult lResult = PvResult::Code::INVALID_PARAMETER; 
  
  BOOST_LOG_TRIVIAL (info) << "--> OpenStream: "<<":"<<ip_address_<<":"
      << mConnectionID.GetAscii ()<<":"<<fetched_ipaddress.GetAscii();
  
  mStream = PvStream::CreateAndOpen (fetched_ipaddress.GetAscii(), &lResult);
  if (!lResult.IsOK ())
    {
      BOOST_LOG_TRIVIAL (info) << "Unable to open the stream";
      ostringstream errorDescription;
      errorDescription << "OpenStream: Unable to open the stream";
      throw i3ds::CommandError(error_value, errorDescription.str());

      return false;
    }

  mPipeline = new PvPipeline (mStream);

  // Reading payload size from device
  int64_t lSize = mDevice->GetPayloadSize ();

  // Create, init the PvPipeline object
  mPipeline->SetBufferSize (static_cast<uint32_t> (lSize));
  mPipeline->SetBufferCount (16);
  // The pipeline needs to be "armed", or started before  we instruct the device to send us images
  lResult = mPipeline->Start ();
  if (!lResult.IsOK ())
    {
      BOOST_LOG_TRIVIAL (info) << "Unable to start pipeline";
      ostringstream errorDescription;
      errorDescription << "OpenStream: Unable to start pipeline";
      throw i3ds::CommandError(error_value, errorDescription.str());
      return false;
    }

  // Only for GigE Vision, if supported
  PvGenBoolean *lRequestMissingPackets =
      dynamic_cast<PvGenBoolean *> (mStream->GetParameters ()->GetBoolean (
	  "RequestMissingPackets"));

  if ((lRequestMissingPackets != NULL)
      && lRequestMissingPackets->IsAvailable ())
    {
      // Disabling request missing packets.
      lRequestMissingPackets->SetValue (false);
    }
  return true;
}

//
// Closes the stream, pipeline
//

void
EbusCameraInterface::CloseStream ()
{
  BOOST_LOG_TRIVIAL (info) << "--> CloseStream";

  if (mPipeline != NULL)
    {
      if (mPipeline->IsStarted ())
	{
	  if (!mPipeline->Stop ().IsOK ())
	    {
	      BOOST_LOG_TRIVIAL (info) << "Unable to stop the pipeline.";
	      ostringstream errorDescription;
	      errorDescription << "CloseStream: Unable to stop the pipeline.";
	      throw i3ds::CommandError(error_value, errorDescription.str());

	    }
	}

      delete mPipeline;
      mPipeline = NULL;
    }

  if (mStream != NULL)
    {
      if (mStream->IsOpen ())
	{
	  if (!mStream->Close ().IsOK ())
	    {
	      BOOST_LOG_TRIVIAL (info) << "Unable to stop the stream.";
	      ostringstream errorDescription;
	      errorDescription << "CloseStream: Unable to stop the stream.";
	      throw i3ds::CommandError(error_value, errorDescription.str());
	    }
	}

      PvStream::Free (mStream);
      mStream = NULL;
    }
}

//
// Starts image acquisition
//

bool
EbusCameraInterface::StartAcquisition ()
{
  BOOST_LOG_TRIVIAL (info) << "--> StartAcquisition";

  // Flush packet queue to make sure there is no left over from previous disconnect event
  PvStreamGEV* lStreamGEV = dynamic_cast<PvStreamGEV*> (mStream);
  if (lStreamGEV != NULL)
    {
      lStreamGEV->FlushPacketQueue ();
    }
  // Set streaming destination (only GigE Vision devces)
  PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*> (mDevice);
  if (lDeviceGEV != NULL)
    {
      // If using a GigE Vision, it is same to assume the stream object is GigE Vision as well
      PvStreamGEV* lStreamGEV = static_cast<PvStreamGEV*> (mStream);

      // Have to set the Device IP destination to the Stream
      PvResult lResult = lDeviceGEV->SetStreamDestination (
      lStreamGEV->GetLocalIPAddress (), lStreamGEV->GetLocalPort ());
      if (!lResult.IsOK ())
	{
	  BOOST_LOG_TRIVIAL (info) << "Setting stream destination failed"
	  << lStreamGEV->GetLocalIPAddress ().GetAscii () << ":"
	  << lStreamGEV->GetLocalPort ();
	  ostringstream errorDescription;
	  errorDescription << "StartAcquisition: Setting stream destination failed"
	  		  << lStreamGEV->GetLocalIPAddress ().GetAscii () << ":"
	  		  << lStreamGEV->GetLocalPort ();
	  throw i3ds::CommandError(error_value, errorDescription.str());


	  return false;
	}
    }

  // Enables stream before sending the AcquisitionStart command.
  mDevice->StreamEnable ();

  // The pipeline is already "armed", we just have to tell the device to start sending us images
  PvResult lResult = mDevice->GetParameters ()->ExecuteCommand (
    "AcquisitionStart");
  if (!lResult.IsOK ())
  {
    BOOST_LOG_TRIVIAL (info) << "Unable to start acquisition";
    ostringstream errorDescription;
    errorDescription << "StreamEnable: Unable to start acquisition";
    throw i3ds::CommandError(error_value, errorDescription.str());
    return false;
  }

return true;
}

//
// Stops acquisition
//

bool
EbusCameraInterface::StopAcquisition (){
  BOOST_LOG_TRIVIAL (info) << "--> StopAcquisition";

  // Tell the device to stop sending images.
  mDevice->GetParameters ()->ExecuteCommand ("AcquisitionStop");

  // Disable stream after sending the AcquisitionStop command.
  mDevice->StreamDisable ();

  PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV*> (mDevice);
  if (lDeviceGEV != NULL)
    {
      // Reset streaming destination (optional...)
      lDeviceGEV->ResetStreamDestination ();
    }

return true;
}



void EbusCameraInterface::do_stop()
{
  BOOST_LOG_TRIVIAL (info) << "--> EbusCameraInterface::do_stop: ";
  stopSamplingLoop = true;
  if(threadSamplingLoop.joinable()) {
      threadSamplingLoop.join();
  }
  TearDown (true);
}


//
// Acquisition loop
//

void EbusCameraInterface::do_deactivate (){
  mDevice->Disconnect();
}



void
EbusCameraInterface::StartSamplingLoop ()
{
  BOOST_LOG_TRIVIAL (info) << "--> StartSamplingLoop";

  bool first = true;

  char lDoodle[] = "|\\-|-/";
  int lDoodleIndex = 0;

  int64_t lImageCountVal = 0;
  double lFrameRateVal = 0.0;
  double lBandwidthVal = 0.0;

  // Acquire images until the user instructs us to stop.
  while (!stopSamplingLoop)
    {

      //If connection flag is up, teardown device/stream
      if (mConnectionLost && (mDevice != NULL))
	{
	  // Device lost: no need to stop acquisition
	  TearDown (false);
	}

      // Only set up tream first time and do not try to reconnect
      if (first == true)
	{
	  BOOST_LOG_TRIVIAL (info) << "First sample-> round initialise";
	  // Device is connected, open the stream
	  if (OpenStream ())
	    {
	      BOOST_LOG_TRIVIAL (info) << "OpenStream went well--> startAcquistion";
	      // Device is connected, stream is opened: start acquisition
	      if (!StartAcquisition ())
		{
		  BOOST_LOG_TRIVIAL (info) << "--> StartAcquisition error";
		  TearDown (false);
		}
	    }
	  else
	    {
	      BOOST_LOG_TRIVIAL (info) << "-->erg";
	      TearDown (false);
	    }
	  first = false;
	}

      // If still no device, no need to continue the loop
      if (mDevice == NULL)
	{
	  continue;
	}
      if ((mStream != NULL) && mStream->IsOpen () && (mPipeline != NULL)
	  && mPipeline->IsStarted ())
	{
	  // Retrieve next buffer
	  PvBuffer *lBuffer = NULL;
	  PvResult lOperationResult;
	  BOOST_LOG_TRIVIAL (info) << "sampling timeout:" << samplingsTimeout_;
	  PvResult lResult = mPipeline->RetrieveNextBuffer (&lBuffer,
							    samplingsTimeout_,
							    &lOperationResult);

	  if (lResult.IsOK ())
	    {
	      if (lOperationResult.IsOK ())
		{
		  //
		  // We now have a valid buffer. This is where you would typically process the buffer.
		  // -----------------------------------------------------------------------------------------
		  // ...

		  mStream->GetParameters ()->GetIntegerValue ("BlockCount", lImageCountVal);
		  mStream->GetParameters ()->GetFloatValue ("AcquisitionRate", lFrameRateVal);
		  mStream->GetParameters ()->GetFloatValue ("Bandwidth", lBandwidthVal);

		  // If the buffer contains an image, display width and height.
		  uint32_t lWidth = 0, lHeight = 0;
		  if (lBuffer->GetPayloadType () == PvPayloadTypeImage)
		    {
		      // Get image specific buffer interface.
		      PvImage *lImage = lBuffer->GetImage ();

		      // Read width, height.
		      lWidth = lImage->GetWidth ();
		      lHeight = lImage->GetHeight ();
		      BOOST_LOG_TRIVIAL (info) << "Width: " << lWidth << " Height: "<< lHeight;

		      clock::time_point next = clock::now();
		     // BOOST_LOG_TRIVIAL (info) << "--> datapointer: " << lImage->GetDataPointer();
		     // printf("Datapointer %p\n", lImage->GetDataPointer());
		      operation_(lImage->GetDataPointer(), std::chrono::duration_cast<std::chrono::microseconds>(next.time_since_epoch()).count());

		    }

		  std::cout << fixed << setprecision (1);
		  std::cout << lDoodle[lDoodleIndex];
		  std::cout << " BlockID: " << uppercase << hex << setfill ('0')
		      << setw (16) << lBuffer->GetBlockID () << " W: " << dec
		      << lWidth << " H: " << lHeight << " " << lFrameRateVal
		      << " FPS " << (lBandwidthVal / 1000000.0) << " Mb/s  \r";




		}
	      // We have an image - do some processing (...) and VERY IMPORTANT,
	      // release the buffer back to the pipeline.
	      mPipeline->ReleaseBuffer (lBuffer);
	    }
	  else
	    {
	      // Timeout


	      std::cout << "Image timeout " << lDoodle[lDoodleIndex] << std::endl;
	    }

	  ++lDoodleIndex %= 6;
	}
      else
	{
	  // No stream/pipeline, must be in recovery. Wait a bit...
	  PvSleepMs (100);
	}
    }

  BOOST_LOG_TRIVIAL (info) << "--> Sampling Loop Exiting";
}

//
// \brief Disconnects the device
//

// \TODO: End streaming loop gracefully, change state and throw exception
void
EbusCameraInterface::DisconnectDevice ()
{
  BOOST_LOG_TRIVIAL (info) << "--> DisconnectDevice";

  if (mDevice != NULL)
    {
      if (mDevice->IsConnected ())
	{
	  // Unregister event sink (call-backs).
	  mDevice->UnregisterEventSink (this);
	}

      PvDevice::Free (mDevice);
      mDevice = NULL;
    }
}

//
// Tear down: closes, disconnects, etc.
//

void
EbusCameraInterface::TearDown (bool aStopAcquisition)
{
  BOOST_LOG_TRIVIAL (info) << "--> TearDown";

  if (aStopAcquisition)
    {
      StopAcquisition ();
    }

  CloseStream ();
  //DisconnectDevice();
}

