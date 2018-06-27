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
				 std::string ipAddress,
				 std::string camera_name,
				 bool is_stereo,
				 bool free_running)
  : Camera(node),
    is_stereo_(is_stereo),
    free_running_(free_running),
    publisher_(context, node)
{
  using namespace std::placeholders;
  
  BOOST_LOG_TRIVIAL(info) << "CosineCamera::CosineCamera()";

  auto op = std::bind(&i3ds::CosineCamera::send_sample, this, _1, _2);
  
  ebus_ = std::unique_ptr<EbusWrapper>(new EbusWrapper(ipAddress,
						       camera_name,
						       op));

  flash_enabled_ = false;
  flash_strength_ = 0.0;

  pattern_enabled_ = false;
  pattern_sequence_ = 0;

  Camera::FrameTopic::Codec::Initialize(frame_);

  frame_.descriptor.frame_mode = mode_mono;
  frame_.descriptor.data_depth = 12;
  frame_.descriptor.pixel_size = 2;

  frame_.descriptor.image_count = is_stereo_ ? 2 : 1;
}

i3ds::CosineCamera::~CosineCamera()
{
}

ShutterTime
i3ds::CosineCamera::shutter() const
{
  const int64_t max = (1l << 32) - 1;
  const int64_t shutter = ebus_->getShutterTime();

  return shutter <= max ? ShutterTime(shutter) : ShutterTime(max);
}

SensorGain
i3ds::CosineCamera::gain() const
{
  const int64_t gain = ebus_->getGain();

  // TODO: Needs scaling?
  return SensorGain(gain);
}

bool
i3ds::CosineCamera::auto_exposure_enabled() const
{
  return ebus_->getAutoExposureEnabled();
}

ShutterTime
i3ds::CosineCamera::max_shutter() const
{
  const int64_t max = (1l << 32) - 1;
  const int64_t shutter = ebus_->getMaxShutterTime();

  return shutter <= max ? ShutterTime(shutter) : ShutterTime(max);
}

SensorGain
i3ds::CosineCamera::max_gain() const
{
  // TODO: What is the valid size here?
  return 12.0;
}


void
i3ds::CosineCamera::updateRegion()
{
  int64_t max = (1l << 16) - 1;
  int64_t sx, sy;
  
  ebus_->getSize(sx, sy);

  if (is_stereo_)
    {
      sy /= 2;
    }

  if (sx > max) sx = max;
  if (sy > max) sy = max;

  region_.offset_x = 0;
  region_.offset_y = 0;
  region_.size_x =  (T_UInt16) sx;
  region_.size_y = (T_UInt16) sy;

  frame_.descriptor.region = region_;
}

void
i3ds::CosineCamera::do_activate()
{
  BOOST_LOG_TRIVIAL(info) << "do_activate()";

  ebus_->connect();

  if (is_stereo_)
    {
      ebus_->setSourceBothStreams();
    }

  BOOST_LOG_TRIVIAL(info) << "Shutter_: " << ebus_->getShutterTime();

  updateRegion();
}

void
i3ds::CosineCamera::do_start()
{
  BOOST_LOG_TRIVIAL(info) << "do_start()";

  ebus_->do_start(free_running_);
}

void
i3ds::CosineCamera::do_stop()
{
  BOOST_LOG_TRIVIAL(info) << "do_stop()";

  ebus_->do_stop();
}

void
i3ds::CosineCamera::do_deactivate()
{
  BOOST_LOG_TRIVIAL(info) << "do_deactivate()";

  ebus_->do_deactivate();
}

bool
i3ds::CosineCamera::is_sampling_supported(SampleCommand sample)
{
  BOOST_LOG_TRIVIAL(info) << "is_rate_supported() " << sample.period;

  return ebus_->checkTriggerInterval(sample.period);
}

void
i3ds::CosineCamera::handle_exposure(ExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_exposure()";

  check_active();

  if(ebus_->getAutoExposureEnabled())
    {
      throw i3ds::CommandError(error_value, "handle_exposure: In auto-exposure mode");
    }

  ebus_->setShutterTime(command.request.shutter);
  ebus_->setGain(command.request.gain);
}

void
i3ds::CosineCamera::handle_auto_exposure(AutoExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure()";

  check_standby();
  
  ebus_->setAutoExposureEnabled(command.request.enable);

  if (command.request.enable)
    {
      ebus_->setMaxShutterTime(command.request.max_shutter);
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
i3ds::CosineCamera::send_sample(unsigned char *image, unsigned long timestamp_us)
{
  BOOST_LOG_TRIVIAL(info) << "CosineCamera::send_sample()";

  const size_t size = image_size(frame_.descriptor);
  
  frame_.append_image(image, size);
  
  if (is_stereo_)
    {
      frame_.append_image(image + size, size);
    }

  publisher_.Send<Camera::FrameTopic>(frame_);

  return true;
}
