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


#include "i3ds/emulators/emulated_camera.hpp"


#include "../include/ebus_camera_interface.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>



namespace logging = boost::log;






i3ds::EmulatedCamera::EmulatedCamera(Context::Ptr context, NodeID node, int resx, int resy)
  : Camera(node),
    resx_(resx),
    resy_(resy),
    sampler_(std::bind(&i3ds::EmulatedCamera::send_sample, this, std::placeholders::_1)),
    publisher_(context, node)
{

  EbusCameraInterface * ebusCameraInterface = new EbusCameraInterface();

  exposure_ = 0;
  gain_ = 0.0;

  auto_exposure_enabled_ = false;
  max_exposure_ = 0;
  max_gain_ = 0.0;

  region_.size_x = resx;
  region_.size_y = resy;
  region_.offset_x = 0;
  region_.offset_y = 0;

  flash_enabled_ = false;
  flash_strength_ = 0.0;

  pattern_enabled_ = false;
  pattern_sequence_ = 0;

  CameraMeasurement4MCodec::Initialize(frame_);

  frame_.frame_mode = mode_mono;
  frame_.data_depth = 12;
  frame_.pixel_size = 2;
  frame_.region.size_x = resx_;
  frame_.region.size_y = resy_;
  frame_.image.nCount = resx_ * resy_ * 2;
}

i3ds::EmulatedCamera::~EmulatedCamera()
{
}

void
i3ds::EmulatedCamera::do_activate()
{
  BOOST_LOG_TRIVIAL(info) << "do_activate()";
}

void
i3ds::EmulatedCamera::do_start()
{
  BOOST_LOG_TRIVIAL(info) << "do_start()";
  sampler_.Start(rate());
}

void
i3ds::EmulatedCamera::do_stop()
{
	BOOST_LOG_TRIVIAL(info) << "do_stop()";
  sampler_.Stop();
}

void
i3ds::EmulatedCamera::do_deactivate()
{
  BOOST_LOG_TRIVIAL(info) << "do_deactivate()";
}

bool
i3ds::EmulatedCamera::is_rate_supported(SampleRate rate)
{
  BOOST_LOG_TRIVIAL(info) << "is_rate_supported()";
  return 0 < rate && rate <= 10000000;
}

void
i3ds::EmulatedCamera::handle_exposure(ExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_exposure()";
  auto_exposure_enabled_ = false;
  exposure_ = command.request.exposure;
  gain_ = command.request.gain;
}

void
i3ds::EmulatedCamera::handle_auto_exposure(AutoExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure()";
  auto_exposure_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      max_exposure_ = command.request.max_exposure;
      max_gain_ = command.request.max_gain;
    }
}

void
i3ds::EmulatedCamera::handle_region(RegionService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_region()";
  region_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      region_ = command.request.region;
    }
}

void
i3ds::EmulatedCamera::handle_flash(FlashService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_flash()";
  flash_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      flash_strength_ = command.request.strength;
    }
}

void
i3ds::EmulatedCamera::handle_pattern(PatternService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "do_pattern()";
  pattern_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      pattern_sequence_ = command.request.sequence;
    }
}

bool
i3ds::EmulatedCamera::send_sample(unsigned long timestamp_us)
{
  BOOST_LOG_TRIVIAL(info) << "send_sample()";
  std::cout << "Send: " << timestamp_us << std::endl;

  frame_.attributes.timestamp.microseconds = timestamp_us;
  frame_.attributes.validity = sample_valid;

  publisher_.Send<ImageMeasurement>(frame_);

  return true;
}
