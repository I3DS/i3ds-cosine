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
#include <sstream>
#include <iomanip>
#include <memory>
#include <exception>

#include "cosine_camera.hpp"

#include <PvSampleUtils.h>

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace logging = boost::log;

i3ds::CosineCamera::CosineCamera(Context::Ptr context, NodeID id, GigECamera::Parameters param, int trigger_scale)
  : GigECamera(context, id, param),
    trigger_scale_(trigger_scale)
{
  BOOST_LOG_TRIVIAL ( info ) << "CosineCamera::CosineCamera()";
}

i3ds::CosineCamera::~CosineCamera()
{
}

void
i3ds::CosineCamera::Open()
{
  BOOST_LOG_TRIVIAL ( info ) << "do_activate()";

  // Connect to the selected Device
  PvResult lResult = PvResult::Code::INVALID_PARAMETER;
  mConnectionID = PvString(param_.camera_name.c_str());
  BOOST_LOG_TRIVIAL ( info ) << "--> ConnectDevice Connection string: " << mConnectionID.GetAscii();
  device_ = PvDevice::CreateAndConnect ( mConnectionID, &lResult );

  if ( !lResult.IsOK() )
    {
      BOOST_LOG_TRIVIAL ( info ) << "CreateAndConnect problem";

      // TODO: Throw here.
    }

  // Register this class as an event sink for PvDevice call-backs
  device_->RegisterEventSink ( this );

  // Clear connection lost flag as we are now connected to the device
  mConnectionLost = false;
  BOOST_LOG_TRIVIAL ( info ) << "Connected to Camera";

  PvDeviceGEV *lDeviceGEV = dynamic_cast<PvDeviceGEV *> ( device_ );

  fetched_ipaddress = lDeviceGEV->GetIPAddress();
  BOOST_LOG_TRIVIAL ( info ) << "IP ADDRESS got from camera" << fetched_ipaddress.GetAscii();

  collectParameters();

  if ( param_.image_count > 1)
    {
      setEnum("SourceSelector", "All", true);
    }
}

void
i3ds::CosineCamera::Close()
{
  BOOST_LOG_TRIVIAL ( info ) << "do_deactivate()";

  device_->Disconnect();
}

void
i3ds::CosineCamera::Start()
{
  BOOST_LOG_TRIVIAL ( info ) << "do_start()";

  setEnum ( "AcquisitionMode", "Continuous" );

  if (param_.external_trigger)
    {
      timeout_ = 200;
      setEnum ( "TriggerMode", "EXT_ONLY" );
    }
  else
    {
      timeout_ = (int) (2 * period() / 1000);
      setEnum ( "TriggerMode", "Interval" );
      setIntParameter ( "TriggerInterval", to_trigger ( period() ) );
    }

  running_ = true;
  thread_ = std::thread ( &i3ds::CosineCamera::SamplingLoop, this );
}

void
i3ds::CosineCamera::Stop()
{
  BOOST_LOG_TRIVIAL ( info ) << "do_stop()";

  running_ = false;

  if ( thread_.joinable() )
    {
      thread_.join();
    }

  TearDown ( true );
}

bool
i3ds::CosineCamera::setInternalTrigger(int64_t period_us)
{
  // TODO: Check this computation.
  int64_t trigger = to_trigger(period_us);

  int64_t min = getMinParameter("TriggerInterval");
  int64_t max = getMaxParameter("TriggerInterval");

  BOOST_LOG_TRIVIAL ( info ) << "min: " << min << " max: " << max << "trigger: " << trigger;

  if (min <= trigger && trigger <= max)
    {
      setIntParameter("TriggerInterval", trigger);
      return true;
    }
  else
    {
      return false;
    }
}

int64_t
i3ds::CosineCamera::getSensorWidth() const
{
  return getParameter("Width");
}

int64_t
i3ds::CosineCamera::getSensorHeight() const
{
  return getParameter("Height");
}

bool
i3ds::CosineCamera::isRegionSupported() const
{
  return false;
}

int64_t
i3ds::CosineCamera::getRegionWidth() const
{
  return getSensorWidth();
}

int64_t
i3ds::CosineCamera::getRegionHeight() const
{
  return getSensorHeight();
}

int64_t
i3ds::CosineCamera::getRegionOffsetX() const
{
  return 0;
}

int64_t
i3ds::CosineCamera::getRegionOffsetY() const
{
  return 0;
}

void
i3ds::CosineCamera::setRegionWidth(int64_t width)
{
}

void
i3ds::CosineCamera::setRegionHeight(int64_t height)
{
}

void
i3ds::CosineCamera::setRegionOffsetX(int64_t offset_x)
{
}

void
i3ds::CosineCamera::setRegionOffsetY(int64_t offset_y)
{
}

int64_t
i3ds::CosineCamera::getShutter() const
{
  return getParameter("ShutterTimeValue");
}

int64_t
i3ds::CosineCamera::getMaxShutter() const
{
  return getMaxParameter("MaxShutterTimeValue");
}

int64_t
i3ds::CosineCamera::getMinShutter() const
{
  return getMinParameter("MaxShutterTimeValue");
}

void
i3ds::CosineCamera::setShutter(int64_t shutter_us)
{
  setIntParameter("ShutterTimeValue", shutter_us);
}

bool
i3ds::CosineCamera::isAutoShutterSupported() const
{
  return true;
}

bool
i3ds::CosineCamera::getAutoShutterEnabled() const
{
  return getEnum("AutoExposure") == "ON" && getBooleanParameter("AutoShutterTime");
}

void
i3ds::CosineCamera::setAutoShutterEnabled(bool enable)
{
  if (enable)
    {
      setEnum("AutoExposure", "ON");
      setBooleanParameter("AutoShutterTime", true);
    }
  else
    {
      setBooleanParameter("AutoShutterTime", false);
      setEnum("AutoExposure", "OFF");
    }
}

int64_t
i3ds::CosineCamera::getAutoShutterLimit() const
{
  return getParameter("MaxShutterTimeValue");
}

int64_t
i3ds::CosineCamera::getMaxAutoShutterLimit() const
{
  return getMaxParameter("MaxShutterTimeValue");
}

int64_t
i3ds::CosineCamera::getMinAutoShutterLimit() const
{
  return getMinParameter("MaxShutterTimeValue");
}

void
i3ds::CosineCamera::setAutoShutterLimit(int64_t shutter_limit)
{
  setIntParameter("MaxShutterTimeValue", shutter_limit);
}

double
i3ds::CosineCamera::getGain() const
{
  return raw_to_gain(getParameter("GainValue"));
}

double
i3ds::CosineCamera::getMaxGain() const
{
  // TODO: Is this a correct value?
  return 12.0;
}

double
i3ds::CosineCamera::getMinGain() const
{
  return 0.0;
}

void
i3ds::CosineCamera::setGain(double gain)
{
  setIntParameter("GainValue", gain_to_raw(gain));
}

bool
i3ds::CosineCamera::isAutoGainSupported() const
{
  return true;
}

bool
i3ds::CosineCamera::getAutoGainEnabled() const
{
  return getEnum("AutoExposure") == "ON" && getBooleanParameter("AutoGain");
}

void
i3ds::CosineCamera::setAutoGainEnabled(bool enable)
{
  if (enable)
    {
      setEnum("AutoExposure", "ON");
      setBooleanParameter("AutoGain", true);

    }
  else
    {
      setBooleanParameter("AutoGain", false);
      setEnum("AutoExposure", "OFF");
    }
}

double
i3ds::CosineCamera::getAutoGainLimit() const
{
  return getMaxGain();
}

double
i3ds::CosineCamera::getMaxAutoGainLimit() const
{
  return getMaxGain();
}

double
i3ds::CosineCamera::getMinAutoGainLimit() const
{
  return 0;
}

void
i3ds::CosineCamera::setAutoGainLimit(double gain_limit)
{
  // TODO: Is max gain supported?
}

double
i3ds::CosineCamera::raw_to_gain(int64_t raw) const
{
  // TODO: Need conversion?
  return (double) raw;
}

int64_t
i3ds::CosineCamera::gain_to_raw(double gain) const
{
  // TODO: Need conversion?
  return (int64_t) gain;
}

int64_t
i3ds::CosineCamera::to_trigger(int64_t period)
{
  return (period * trigger_scale_) / 1000000;
}

int64_t
i3ds::CosineCamera::to_period(int64_t trigger)
{
  return (trigger * 1000000) / trigger_scale_;
}

void
i3ds::CosineCamera::OnLinkDisconnected ( PvDevice *aDevice )
{
  BOOST_LOG_TRIVIAL ( info )
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
i3ds::CosineCamera::collectParameters()
{
  BOOST_LOG_TRIVIAL ( info ) << "Collecting Camera parameters";
  lParameters = device_->GetParameters();
}

int64_t
i3ds::CosineCamera::getParameter ( PvString whichParameter ) const
{

  BOOST_LOG_TRIVIAL ( info ) << "Fetching parameter: "
                             << whichParameter.GetAscii();
  PvGenParameter *lParameter = lParameters->Get ( whichParameter );
  PvGenInteger *lIntParameter = dynamic_cast<PvGenInteger *> ( lParameter );

  if ( lIntParameter == NULL )
    {
      ostringstream errorDescription;

      BOOST_LOG_TRIVIAL ( info ) << "Unable to get the parameter: "
                                 << whichParameter.GetAscii();

      errorDescription << "getParameter: Unable to get the parameter: " << whichParameter.GetAscii();

      throw i3ds::CommandError ( error_value, errorDescription.str() );
    }

  // Read current width value.
  int64_t lParameterValue = 0;

  if ( ! ( lIntParameter->GetValue ( lParameterValue ).IsOK() ) )
    {
      ostringstream errorDescription;

      BOOST_LOG_TRIVIAL ( info ) << "Error retrieving parameter from device";

      errorDescription << "getParameter: Error retrieving parameter from device" << whichParameter.GetAscii();

      throw i3ds::CommandError ( error_value, errorDescription.str() );
    }

  BOOST_LOG_TRIVIAL ( info ) << "Parametervalue: " << lParameterValue
                             << " returned from parameter: " << whichParameter.GetAscii();
  return lParameterValue;
}

// Fetching minimum allowed value of parameter
int64_t
i3ds::CosineCamera::getMinParameter ( PvString whichParameter ) const
{

  PvGenParameter *lParameter = lParameters->Get ( whichParameter );

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lMinParameter = dynamic_cast<PvGenInteger *> ( lParameter );

  if ( lMinParameter == NULL )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Unable to get the Minimum parameter for: "
                                 << whichParameter.GetAscii();
    }

  int64_t lMinValue = 0;

  if ( ! ( lMinParameter->GetMin ( lMinValue ).IsOK() ) )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Error retrieving minimum value from device";
    }
  else
    {
      BOOST_LOG_TRIVIAL ( info ) << "Minimum value for parameter: "
                                 << whichParameter.GetAscii() << " : " << lMinValue;
    }

  return lMinValue;
}

// Get maximum allowed value of parameter.
int64_t
i3ds::CosineCamera::getMaxParameter ( PvString whichParameter ) const
{

  PvGenParameter *lParameter = lParameters->Get ( whichParameter );

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lMaxParameter = dynamic_cast<PvGenInteger *> ( lParameter );

  if ( lMaxParameter == NULL )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Unable to get parameter "
                                 << whichParameter.GetAscii();
    }

  int64_t lMaxAllowedValue = 0;
  if ( ! ( lMaxParameter->GetMax ( lMaxAllowedValue ).IsOK() ) )
    {
      BOOST_LOG_TRIVIAL ( info )
          << "Error retrieving max value from device for parameter "
          << whichParameter.GetAscii();
      return 0;
    }
  else
    {
      BOOST_LOG_TRIVIAL ( info ) << "Max allowed value for parameter: "
                                 << whichParameter.GetAscii() << " is " << lMaxAllowedValue;
      return lMaxAllowedValue;
    }

}

std::string
i3ds::CosineCamera::getEnum ( PvString whichParameter ) const
{
  PvGenParameter *lGenParameter = lParameters->Get ( whichParameter );

  // Parameter available?
  if ( !lGenParameter->IsAvailable() )
    {
      BOOST_LOG_TRIVIAL ( info ) << "{Not Available}";
      throw i3ds::CommandError ( error_value, "eBUS: Enum not available" );
    }

  // Parameter readable?
  if ( !lGenParameter->IsReadable() )
    {
      BOOST_LOG_TRIVIAL ( info ) << "{Not readable}";
      throw i3ds::CommandError ( error_value, "eBUS: Enum not readable" );
    }

  // Get the parameter type
  PvGenType lType;
  lGenParameter->GetType ( lType );

  if ( lType != PvGenTypeEnum )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Not an enum";
      throw i3ds::CommandError ( error_value, "eBUS: Not an enum" );
    }

  PvString lValue;

  static_cast<PvGenEnum *> ( lGenParameter )->GetValue ( lValue );

  BOOST_LOG_TRIVIAL ( info ) << "Enum: " << lValue.GetAscii();

  return std::string ( lValue.GetAscii() );
}

bool
i3ds::CosineCamera::checkIfEnumOptionIsOK ( PvString whichParameter,
    PvString value ) const
{
  BOOST_LOG_TRIVIAL ( info ) << "checkIfEnumOptionIsOK: Parameter: "
                             << whichParameter.GetAscii() << "Value: " << value.GetAscii();

  //To pull out enums options

  int64_t aCount;

  auto *lGenParameter = lParameters->Get ( whichParameter );
  static_cast<PvGenEnum *> ( lGenParameter )->GetEntriesCount ( aCount );
  BOOST_LOG_TRIVIAL ( info ) << "Enum entries: " << aCount;

  for ( int i = 0; i < aCount; i++ )
    {
      const PvGenEnumEntry *aEntry;

      static_cast<PvGenEnum *> ( lGenParameter )->GetEntryByIndex ( i, &aEntry );
      PvString enumOption;
      aEntry->GetName ( enumOption );
      BOOST_LOG_TRIVIAL ( info ) << "EnumText[" << i << "]: "
                                 << enumOption.GetAscii();
      if ( enumOption == value )
        {
          BOOST_LOG_TRIVIAL ( info ) << "Option found.";
          return true;
        }
    }

  BOOST_LOG_TRIVIAL ( info ) << "Option not found.";

  ostringstream errorDescription;

  errorDescription << "checkEnum: Option: " << value.GetAscii() << " does not exists for parameter: "
                   << whichParameter.GetAscii();

  throw i3ds::CommandError ( error_value, errorDescription.str() );
}


void
i3ds::CosineCamera::setEnum ( PvString whichParameter, PvString value, bool dontCheckParameter )
{
  BOOST_LOG_TRIVIAL ( info ) << "setEnum: Parameter: "
                             << whichParameter.GetAscii() << " Value: " << value.GetAscii();
  BOOST_LOG_TRIVIAL ( info ) << "do checkIfEnumOptionIsOK: Parameter first";

  bool enumOK = true;

  if ( dontCheckParameter == false )
    {
      enumOK = checkIfEnumOptionIsOK ( whichParameter, value );
    }

  // enumOK== true if not options not checked.
  if ( enumOK )
    {

      PvGenParameter *lParameter = lParameters->Get ( whichParameter );
      auto *lEnumParameter = dynamic_cast<PvGenEnum *> ( lParameter );

      if ( lEnumParameter == NULL )
        {
          BOOST_LOG_TRIVIAL ( info ) << "Unable to get the parameter: " << whichParameter.GetAscii();
          ostringstream errorDescription;
          errorDescription << "setEnum: Unable to get parameter: " << whichParameter.GetAscii();

          throw i3ds::CommandError ( error_value, errorDescription.str() );
        }

      if ( ! ( lEnumParameter->SetValue ( value ).IsOK() ) )
        {
          BOOST_LOG_TRIVIAL ( info ) << "Error setting parameter for device";
          ostringstream errorDescription;
          errorDescription << "setEnum: Error setting value: " << value.GetAscii() <<
                           " for parameter: " << whichParameter.GetAscii();

          throw i3ds::CommandError ( error_value, errorDescription.str() );
        }
      else
        {
          BOOST_LOG_TRIVIAL ( info ) << "Parameter value: " << value.GetAscii()
                                     << " set for parameter: " << whichParameter.GetAscii();
          return;
        }

    }
  else
    {
      BOOST_LOG_TRIVIAL ( info ) << "Illegal parameter value";
      ostringstream errorDescription;
      errorDescription << "setEnum: Illegal parameter value: " << value.GetAscii() <<
                       " for parameter: " << whichParameter.GetAscii();
      throw i3ds::CommandError ( error_value, errorDescription.str() );
    }
}

bool
i3ds::CosineCamera::getBooleanParameter ( PvString whichParameter ) const
{
  PvGenParameter *lParameter = lParameters->Get ( whichParameter );

  bool lValue;
  static_cast<PvGenBoolean *> ( lParameter )->GetValue ( lValue );

  if ( lValue )
    {
      BOOST_LOG_TRIVIAL ( info ) << whichParameter.GetAscii() << "Boolean: TRUE";
      return true;
    }
  else
    {
      BOOST_LOG_TRIVIAL ( info ) << whichParameter.GetAscii()
                                 << "Boolean: FALSE";
      return true;
    }
}

void
i3ds::CosineCamera::setBooleanParameter ( PvString whichParameter, bool status )
{
  PvGenParameter *lParameter = lParameters->Get ( whichParameter );

  PvResult result = static_cast<PvGenBoolean *> ( lParameter )->SetValue ( status );

  if ( result != PvResult::Code::OK )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Error setting boolean parameter: " << whichParameter.GetAscii() << " to " << status;

      ostringstream errorDescription;

      errorDescription << "setBooleanParameter Option: Unable to set the parameter: "
                       << whichParameter.GetAscii();

      throw i3ds::CommandError ( error_value, errorDescription.str() );
    }

  BOOST_LOG_TRIVIAL ( info ) << "Boolean parameter: " << whichParameter.GetAscii() << " set to " << status;
}


// Sets an integer Parameter on device
bool
i3ds::CosineCamera::setIntParameter ( PvString whichParameter, int64_t value )
{
  PvGenParameter *lParameter = lParameters->Get ( whichParameter );

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lvalueParameter = dynamic_cast<PvGenInteger *> ( lParameter );
  if ( lvalueParameter == NULL )
    {
      BOOST_LOG_TRIVIAL ( info )
          << "SetParameter: Unable to get the parameter: "
          << whichParameter.GetAscii();
      ostringstream errorDescription;
      errorDescription << "setIntParameter Option: SetParameter: Unable to get the parameter: "
                       << whichParameter.GetAscii();
      throw i3ds::CommandError ( error_value, errorDescription.str() );
    }

  int64_t max = getMaxParameter ( whichParameter );
  if ( value > max )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Setting value Error: Parameter: "
                                 << whichParameter.GetAscii() << " value too big " << value << " (Max: "
                                 << max << ")";


      ostringstream errorDescription;
      errorDescription << "setIntParameter: " << whichParameter.GetAscii() << " value to large " <<
                       value << ".(Max: " << max << ")" ;
      throw i3ds::CommandError ( error_value, errorDescription.str() );

    };

  int64_t min = getMinParameter ( whichParameter );
  if ( value < min )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Error: value to small " << value << "<"
                                 << min;
      ostringstream errorDescription;
      errorDescription << "setIntParameter: " << whichParameter.GetAscii() << " Value to small " <<
                       value << ".(Min: " << min << ")";
      throw i3ds::CommandError ( error_value, errorDescription.str() );

    };
  /// TODO: Increment also?

  PvResult res = lvalueParameter->SetValue ( value );

  if ( !res.IsOK() )
    {
      BOOST_LOG_TRIVIAL ( info ) << "SetValue Error: "
                                 << whichParameter.GetAscii();

      ostringstream errorDescription;
      errorDescription << "setIntParameter: SetValue Error " << whichParameter.GetAscii() ;

      throw i3ds::CommandError ( error_value, errorDescription.str() );
    }

  BOOST_LOG_TRIVIAL ( info ) << "SetValue Ok: " << whichParameter.GetAscii() << "=" << value;

  return true;
}

bool
i3ds::CosineCamera::OpenStream()
{
  BOOST_LOG_TRIVIAL ( info ) << "--> OpenStream: " << mConnectionID.GetAscii();

  // Creates and open the stream object based on the selected device.
  PvResult lResult = PvResult::Code::INVALID_PARAMETER;

  BOOST_LOG_TRIVIAL ( info ) << "--> OpenStream "
                             << " id: " << mConnectionID.GetAscii()
                             << " address: " << fetched_ipaddress.GetAscii();

  mStream = PvStream::CreateAndOpen ( fetched_ipaddress.GetAscii(), &lResult );

  if ( !lResult.IsOK() )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Unable to open the stream";
      return false;
    }

  mPipeline = new PvPipeline ( mStream );

  // Reading payload size from device
  int64_t lSize = device_->GetPayloadSize();

  // Create, init the PvPipeline object
  mPipeline->SetBufferSize ( static_cast<uint32_t> ( lSize ) );
  mPipeline->SetBufferCount ( 16 );

  // The pipeline needs to be "armed", or started before  we instruct the device to send us images
  lResult = mPipeline->Start();

  if ( !lResult.IsOK() )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Unable to start pipeline";
      return false;
    }

  // Only for GigE Vision, if supported
  PvGenBoolean *lRequestMissingPackets =
    dynamic_cast<PvGenBoolean *> ( mStream->GetParameters()->GetBoolean (
                                     "RequestMissingPackets" ) );

  if ( ( lRequestMissingPackets != NULL )
       && lRequestMissingPackets->IsAvailable() )
    {
      // Disabling request missing packets.
      lRequestMissingPackets->SetValue ( false );
    }

  return true;
}

//
// Closes the stream, pipeline
//

void
i3ds::CosineCamera::CloseStream()
{
  BOOST_LOG_TRIVIAL ( info ) << "--> CloseStream";

  if ( mPipeline != NULL )
    {
      if ( mPipeline->IsStarted() )
        {
          if ( !mPipeline->Stop().IsOK() )
            {
              BOOST_LOG_TRIVIAL ( info ) << "Unable to stop the pipeline.";
            }
        }

      delete mPipeline;
      mPipeline = NULL;
    }

  if ( mStream != NULL )
    {
      if ( mStream->IsOpen() )
        {
          if ( !mStream->Close().IsOK() )
            {
              BOOST_LOG_TRIVIAL ( info ) << "Unable to stop the stream.";
            }
        }

      PvStream::Free ( mStream );
      mStream = NULL;
    }
}

//
// Starts image acquisition
//

bool
i3ds::CosineCamera::StartAcquisition()
{
  BOOST_LOG_TRIVIAL ( info ) << "--> StartAcquisition";

  // Flush packet queue to make sure there is no left over from previous disconnect event
  PvStreamGEV *lStreamGEV = dynamic_cast<PvStreamGEV *> ( mStream );
  if ( lStreamGEV != NULL )
    {
      lStreamGEV->FlushPacketQueue();
    }

  // Set streaming destination (only GigE Vision devces)
  PvDeviceGEV *lDeviceGEV = dynamic_cast<PvDeviceGEV *> ( device_ );
  if ( lDeviceGEV != NULL )
    {
      // If using a GigE Vision, it is same to assume the stream object is GigE Vision as well
      PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *> ( mStream );

      // Have to set the Device IP destination to the Stream
      PvResult lResult = lDeviceGEV->SetStreamDestination (
                           lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() );
      if ( !lResult.IsOK() )
        {
          BOOST_LOG_TRIVIAL ( info ) << "Setting stream destination failed"
                                     << lStreamGEV->GetLocalIPAddress().GetAscii() << ":"
                                     << lStreamGEV->GetLocalPort();

          return false;
        }
    }

  // Enables stream before sending the AcquisitionStart command.
  device_->StreamEnable();

  // The pipeline is already "armed", we just have to tell the device to start sending us images
  PvResult lResult = device_->GetParameters()->ExecuteCommand ( "AcquisitionStart" );

  if ( !lResult.IsOK() )
    {
      BOOST_LOG_TRIVIAL ( info ) << "Unable to start acquisition";

      return false;
    }

  return true;
}

//
// Stops acquisition
//

bool
i3ds::CosineCamera::StopAcquisition()
{
  BOOST_LOG_TRIVIAL ( info ) << "--> StopAcquisition";

  // Tell the device to stop sending images.
  device_->GetParameters()->ExecuteCommand ( "AcquisitionStop" );

  // Disable stream after sending the AcquisitionStop command.
  device_->StreamDisable();

  PvDeviceGEV *lDeviceGEV = dynamic_cast<PvDeviceGEV *> ( device_ );
  if ( lDeviceGEV != NULL )
    {
      // Reset streaming destination (optional...)
      lDeviceGEV->ResetStreamDestination();
    }

  return true;
}

//
// Tear down: closes, disconnects, etc.
//
void
i3ds::CosineCamera::TearDown ( bool aStopAcquisition )
{
  BOOST_LOG_TRIVIAL ( info ) << "--> TearDown";

  if ( aStopAcquisition )
    {
      StopAcquisition();
    }

  CloseStream();
  //DisconnectDevice();
}

void
i3ds::CosineCamera::SamplingLoop()
{
  BOOST_LOG_TRIVIAL ( info ) << "--> SamplingLoop";

  bool first = true;

  int64_t lImageCountVal = 0;
  double lFrameRateVal = 0.0;
  double lBandwidthVal = 0.0;

  // Acquire images until the user instructs us to stop.
  while (running_)
    {

      //If connection flag is up, teardown device/stream
      if ( mConnectionLost && ( device_ != NULL ) )
        {
          // Device lost: no need to stop acquisition
          samplingErrorFlag = true;
          strncpy ( samplingErrorText, "Connection to camera lost", 25 );
          TearDown ( false );
        }

      // Only set up stream first time and do not try to reconnect
      if ( first )
        {
          BOOST_LOG_TRIVIAL ( info ) << "First sample-> round initialise";
          // Device is connected, open the stream
          if ( OpenStream() )
            {
              BOOST_LOG_TRIVIAL ( info ) << "OpenStream went well--> startAcquistion";
              // Device is connected, stream is opened: start acquisition
              if ( !StartAcquisition() )
                {
                  BOOST_LOG_TRIVIAL ( info ) << "--> StartAcquisition error";
                  samplingErrorFlag = true;
                  strncpy ( samplingErrorText, "StartAcqisition error", 25 );

                  TearDown ( false );
                }
            }
          else
            {
              BOOST_LOG_TRIVIAL ( info ) << "-->OpenStream Error";
              samplingErrorFlag = true;
              strncpy ( samplingErrorText, "StartAcqisition error", 25 );

              TearDown ( false );
            }

          first = false;
        }

      // If still no device, no need to continue the loop
      if ( device_ == NULL )
        {
          break;
        }

      if ( ( mStream != NULL ) && mStream->IsOpen() && ( mPipeline != NULL )
           && mPipeline->IsStarted() )
        {
          // Retrieve next buffer
          PvBuffer *lBuffer = NULL;
          PvResult lOperationResult;


          PvResult lResult = mPipeline->RetrieveNextBuffer ( &lBuffer, timeout_, &lOperationResult );

          if ( lResult.IsOK() )
            {
              if ( lOperationResult.IsOK() )
                {
                  //
                  // We now have a valid buffer. This is where you would typically process the buffer.
                  // -----------------------------------------------------------------------------------------
                  // ...

                  mStream->GetParameters()->GetIntegerValue ( "BlockCount", lImageCountVal );
                  mStream->GetParameters()->GetFloatValue ( "AcquisitionRate", lFrameRateVal );
                  mStream->GetParameters()->GetFloatValue ( "Bandwidth", lBandwidthVal );

                  // If the buffer contains an image, display width and height.

                  if ( lBuffer->GetPayloadType() == PvPayloadTypeImage )
                    {
                      // Get image specific buffer interface.
                      PvImage *lImage = lBuffer->GetImage();
                      uint32_t lWidth = lImage->GetWidth();
                      uint32_t lHeight = lImage->GetHeight();

                      BOOST_LOG_TRIVIAL ( info ) << "Width: " << lWidth << " Height: " << lHeight;

                      send_sample ( lImage->GetDataPointer(), lWidth, lHeight );
                    }
                }

              // We have an image - do some processing (...) and VERY IMPORTANT,
              // release the buffer back to the pipeline.
              mPipeline->ReleaseBuffer ( lBuffer );
            }
          BOOST_LOG_TRIVIAL ( info ) << "sampling timeout without receiving good image: " << timeout_ << "ms";
        }
      else
        {
          // No stream/pipeline, must be in recovery. Wait a bit...
          PvSleepMs ( 100 );

        }
    }

  BOOST_LOG_TRIVIAL ( info ) << "--> Sampling Loop Exiting";
}
