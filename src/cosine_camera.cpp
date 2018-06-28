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
                                 Parameters param)
  : Camera(node),
    param_(param),
    publisher_(context, node)
{
  using namespace std::placeholders;

  BOOST_LOG_TRIVIAL(info) << "CosineCamera::CosineCamera()";

  auto op = std::bind(&i3ds::CosineCamera::send_sample, this, _1, _2, _3);

  ebus_ = std::unique_ptr<EbusWrapper>(new EbusWrapper(param.camera_name, op));

  flash_enabled_ = false;
  flash_strength_ = 0.0;

  pattern_enabled_ = false;
  pattern_sequence_ = 0;
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

  BOOST_LOG_TRIVIAL(info) << "Initial shutter: " << shutter();
  BOOST_LOG_TRIVIAL(info) << "Initial gain:    " << gain();
}

void
i3ds::CosineCamera::do_start()
{
  BOOST_LOG_TRIVIAL(info) << "do_start()";

  int64_t trigger = to_trigger(period());
  int timeout_ms = (int)(2 * period() / 1000);

  ebus_->Start(param_.free_running, trigger, timeout_ms);
}

void
i3ds::CosineCamera::do_stop()
{
  BOOST_LOG_TRIVIAL(info) << "do_stop()";

  ebus_->Stop();
}

void
i3ds::CosineCamera::do_deactivate()
{
  BOOST_LOG_TRIVIAL(info) << "do_deactivate()";

  ebus_->Disconnect();
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

  flash_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      // TODO: Use flash and trigger clients.
      flash_strength_ = command.request.strength;
    }
}

void
i3ds::CosineCamera::handle_pattern(PatternService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "do_pattern()";

  check_standby();

  pattern_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      // TODO: Use flash and trigger clients.
      pattern_sequence_ = command.request.sequence;
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
