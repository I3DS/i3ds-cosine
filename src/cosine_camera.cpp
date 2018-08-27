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

#include "cosine_camera.hpp"
#include "ebus_wrapper.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace logging = boost::log;

i3ds::CosineCamera::CosineCamera(Context::Ptr context,
                                 NodeID node,
				 NodeID flash_node_id,
                                 Parameters param,
				 TriggerClient::Ptr trigger)
  : Camera(node),
    param_(param),
    publisher_(context, node),
    trigger_(trigger),
    flash_client(context, flash_node_id)
{
  using namespace std::placeholders;

  BOOST_LOG_TRIVIAL(info) << "CosineCamera::CosineCamera()";

  auto op = std::bind(&i3ds::CosineCamera::send_sample, this, _1, _2, _3);

  ebus_ = std::unique_ptr<EbusWrapper>(new EbusWrapper(param.camera_name, op));

  flash_enabled_ = false;
  flash_strength_ = 0.0;

  pattern_enabled_ = false;
  pattern_sequence_ = 0;


  if (trigger_)
      {
        // Only wait 100 ms for trigger service.
        trigger_->set_timeout(100);
      }

  //flash_configurator_ = std::unique_ptr<SerialCommunicator>(new SerialCommunicator(param.wa_flash_port.c_str()));


  //i3ds::Context::Ptr context;

  // context = i3ds::Context::Create();
 // BOOST_LOG_TRIVIAL(info) << "Connecting to Wide Angular Flash Server with node ID: " << node;
  //i3ds::FlashClient flash_client(context, node);
 // BOOST_LOG_TRIVIAL(trace) << "---> [OK]";

}

i3ds::CosineCamera::~CosineCamera()
{
}

ShutterTime
i3ds::CosineCamera::shutter() const
{
  const int64_t max = (1l << 32) - 1;
  const int64_t shutter = ebus_->getParameter("ShutterTimeValue");

  return shutter <= max ? ShutterTime(shutter) : ShutterTime(max);
}

SensorGain
i3ds::CosineCamera::gain() const
{
  const int64_t gain = ebus_->getParameter("GainValue");

  // TODO: Needs scaling?
  return SensorGain(gain);
}

bool
i3ds::CosineCamera::auto_exposure_enabled() const
{
  return ebus_->getEnum("AutoExposure") == "ON";
}

ShutterTime
i3ds::CosineCamera::max_shutter() const
{
  const int64_t max = (1l << 32) - 1;
  const int64_t shutter = ebus_->getMaxParameter("MaxShutterTimeValue");

  return shutter <= max ? ShutterTime(shutter) : ShutterTime(max);
}

SensorGain
i3ds::CosineCamera::max_gain() const
{
  // TODO: What is the valid size here?
  return 12.0;
}

PlanarRegion
i3ds::CosineCamera::region() const
{
  int64_t sx = ebus_->getParameter("Width");
  int64_t sy = ebus_->getParameter("Height");

  if (param_.is_stereo)
    {
      sy /= 2;
    }

  PlanarRegion region;

  region.offset_x = 0;
  region.offset_y = 0;
  region.size_x = (T_UInt16) sx; // Known to be safe.
  region.size_y = (T_UInt16) sy; // Known to be safe.

  return region;
}

void
i3ds::CosineCamera::do_activate()
{
  BOOST_LOG_TRIVIAL(info) << "do_activate()";

  ebus_->Connect();

  if (param_.is_stereo)
    {
      ebus_->setEnum("SourceSelector", "All", true);
    }

  if (trigger_) {
    set_trigger(param_.trigger_camera_output, param_.trigger_camera_offset);
  }


  BOOST_LOG_TRIVIAL(info) << "Initial shutter: " << shutter();
  BOOST_LOG_TRIVIAL(info) << "Initial gain:    " << gain();
}

void
i3ds::CosineCamera::do_start()
{
  BOOST_LOG_TRIVIAL(info) << "do_start()";

  int64_t trigger = to_trigger(period());
  int timeout_ms = (int)(2 * period() / 1000);

  if (!param_.free_running)
  {
    timeout_ms = 200;
  }
  if (trigger_) {
    trigger_->set_generator(param_.trigger_generator, period());
    trigger_->enable_channels(trigger_outputs_);
  } else {
    BOOST_LOG_TRIVIAL(warning) << "Not free running without trigger.";
  }

  ebus_->Start(param_.free_running, trigger, timeout_ms);
}

void
i3ds::CosineCamera::do_stop()
{
  BOOST_LOG_TRIVIAL(info) << "do_stop()";
  if (trigger_)
  {
    trigger_->disable_channels(trigger_outputs_);
  }

  ebus_->Stop();
}

void
i3ds::CosineCamera::do_deactivate()
{
  BOOST_LOG_TRIVIAL(info) << "do_deactivate()";

  ebus_->Disconnect();

  pattern_enabled_ = false;
  pattern_sequence_ = 0;

  trigger_outputs_.clear();
}

int64_t
i3ds::CosineCamera::to_trigger(SamplePeriod period)
{
  return (period * param_.trigger_scale) / 1000000;
}

SamplePeriod
i3ds::CosineCamera::to_period(int64_t trigger)
{
  return (trigger * 1000000) / param_.trigger_scale;
}

bool
i3ds::CosineCamera::is_sampling_supported(SampleCommand sample)
{
  BOOST_LOG_TRIVIAL(info) << "is_rate_supported() " << sample.period;

  // TODO: Check this computation.
  int64_t trigger = to_trigger(sample.period);

  int64_t min = ebus_->getMinParameter("TriggerInterval");
  int64_t max = ebus_->getMaxParameter("TriggerInterval");

  BOOST_LOG_TRIVIAL(info) << "min: " << min << " max: " << max << "trigger: " << trigger;

  return min <= trigger && trigger <= max;
}

void
i3ds::CosineCamera::handle_exposure(ExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_exposure()";

  check_active();

  if (auto_exposure_enabled())
    {
      throw i3ds::CommandError(error_value, "handle_exposure: In auto-exposure mode");
    }

  ebus_->setIntParameter("ShutterTimeValue", (int64_t) command.request.shutter);
  ebus_->setIntParameter("GainValue", (int64_t) command.request.gain); // TODO: Needs scaling?
}

void
i3ds::CosineCamera::handle_auto_exposure(AutoExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure()";

  check_standby();

  if (command.request.enable)
    {
      ebus_->setEnum("AutoExposure", "ON");
      ebus_->setBooleanParameter("AutoGain", true);
      ebus_->setBooleanParameter("AutoShutterTime", true);
      ebus_->setIntParameter("MaxShutterTimeValue", (int64_t) command.request.max_shutter);
    }
  else
    {
      ebus_->setBooleanParameter("AutoGain", false);
      ebus_->setBooleanParameter("AutoShutterTime", false);
      ebus_->setEnum("AutoExposure", "OFF");
    }
}

void
i3ds::CosineCamera::handle_flash(FlashService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_flash()";

  check_standby();

  if (!trigger_)
    {
      throw i3ds::CommandError(error_other, "Flash not supported in free-running mode");
    }

  flash_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      flash_strength_ = command.request.strength;


      if (flash_strength_ > 100)
      {
	throw i3ds::CommandError(error_value, "The flash can not give more than 100%");
      }
      BOOST_LOG_TRIVIAL(info) << "handle_flash()";
      float flash_duration_in_ms;

      if (auto_exposure_enabled())
      {
	//flash_duration_in_ms = ebus_->AutoExposureTimeAbsUpperLimit.GetValue() / 1000.;
	flash_duration_in_ms = ebus_->getParameter("MaxShutterTimeValue") / 1000; // Camera parameter is in uS

      }
      else
      {
	flash_duration_in_ms = ebus_->getParameter("ShutterTimeValue") / 1000; // Camera parameter is in uS
      }
      BOOST_LOG_TRIVIAL(info) << "handle_flash()" << flash_duration_in_ms;

      // Upper limit for flash
      if (flash_duration_in_ms > 3)
      {
	flash_duration_in_ms = 3;
      }

      /// Remark: Err 5 is a out of range warning for one parameter.
      /// It is fixed to valid value.
      /// But, it looks as there is a limit with a relationship between duration and flash strength
      /// http://www.gardasoft.com/downloads/?f=112
      //  Page 13(Read it)
      ///
      //Output brightness		850nm variant    | 			|	White variant
      //			Allowed pulsewidth |Allowed duty cycle 	| Allowed pulse width   |   Allowed duty cycle
      // 0% to  20% 		3ms 			6% 			3ms 			3%
      //21% to  30% 		3ms 			6%	 		2ms 			3%
      //31% to  50% 		3ms 			3% 			2ms 			2%
      //51% to 100% 		2ms 			3% 			1ms 			1%



/*
      flash_configurator_->sendConfigurationParameters (
	1, 			// Configure strobe output
	flash_duration_in_ms,	// Pulse width ms
	0.01, 		// Delay from trigger to pulse in ms(0.01 to 999)
	flash_strength_	/// Settings in percent
	); 			// 5th parameter retrigger delay in ms(optional not used)

  */
    /*
      i3ds::Context::Ptr context;

       context = i3ds::Context::Create();
       BOOST_LOG_TRIVIAL(info) << "Connecting to Wide Angular Flash Server with node ID: " << node_id;
       i3ds::FlashClient flash_client(context, node_id);
       BOOST_LOG_TRIVIAL(trace) << "---> [OK]";
*/

      flash_client.set_flash(flash_duration_in_ms, flash_strength_);



      // Enable trigger for flash.
     // set_trigger(param_.flash_output, param_.flash_offset);


    }
  else
    {
      // Clear trigger, not enabled when operational.
      clear_trigger(param_.flash_output);
    }

}

void
i3ds::CosineCamera::handle_pattern(PatternService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_pattern()";

  check_standby();

  if (!trigger_)
    {
      throw i3ds::CommandError(error_other, "Pattern not supported in free-running mode");
    }

  pattern_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      // Only support one pattern sequence, not controllable as of now.
      if (command.request.sequence != 1)
	{
	  throw i3ds::CommandError(error_value, "Unsupported pattern sequence");
	}

      pattern_sequence_ = command.request.sequence;

      // Enable trigger for flash.
      set_trigger(param_.trigger_pattern_output, param_.trigger_pattern_offset);
    }
  else
    {
      // Reset pattern sequence to disabled.
      pattern_sequence_ = 0;

      // Clear trigger, not enabled when operational.
      clear_trigger(param_.trigger_pattern_output);
    }
}

bool
i3ds::CosineCamera::send_sample(unsigned char *image, int width, int height)
{
  BOOST_LOG_TRIVIAL(info) << "CosineCamera::send_sample()";

  Camera::FrameTopic::Data frame;

  Camera::FrameTopic::Codec::Initialize(frame);

  T_UInt16 sx = (T_UInt16) width;  // Known to be safe.
  T_UInt16 sy = (T_UInt16) height; // Known to be safe.

  if (param_.is_stereo)
    {
      sy /= 2;
    }

  frame.descriptor.region.size_x = sx;
  frame.descriptor.region.size_y = sy;
  frame.descriptor.frame_mode = mode_mono;
  frame.descriptor.data_depth = param_.data_depth;
  frame.descriptor.pixel_size = 2;
  frame.descriptor.image_count = param_.is_stereo ? 2 : 1;

  const size_t size = image_size(frame.descriptor);

  frame.append_image(image, size);

  if (param_.is_stereo)
    {
      frame.append_image(image + size, size);
    }

  publisher_.Send<Camera::FrameTopic>(frame);

  return true;
}


void
i3ds::CosineCamera::set_trigger(TriggerOutput channel, TriggerOffset offset)
{
  // Set the channel to fire at offset with 100 us pulse.
  trigger_->set_internal_channel(channel, param_.trigger_generator, offset, 100, false);

  // Enable the trigger on do_start.
  trigger_outputs_.insert(channel);
  BOOST_LOG_TRIVIAL(info) << "Trigger inserted done Ok. channel no: ";
}

void
i3ds::CosineCamera::clear_trigger(TriggerOutput channel)
{
  // Do not enable the trigger on do_start.
  trigger_outputs_.erase(channel);
}
