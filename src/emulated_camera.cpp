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


#include "../include/emulated_camera.hpp"


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

  ebusCameraInterface = new EbusCameraInterface();

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
  ebusCameraInterface->connect();
  exposure_ = ebusCameraInterface->getShutterTime();
  BOOST_LOG_TRIVIAL(info) << "Exposure: " << exposure_;
}

void
i3ds::EmulatedCamera::do_start()
{
  BOOST_LOG_TRIVIAL(info) << "do_start()";
  //sampler_.Start(rate());

  ebusCameraInterface->do_start();

}

void
i3ds::EmulatedCamera::do_stop()
{
  BOOST_LOG_TRIVIAL(info) << "do_stop()";

  ebusCameraInterface->do_stop();


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
  BOOST_LOG_TRIVIAL(info) << "is_rate_supported()" << rate;
  ebusCameraInterface->checkTriggerInterval(rate);
  //rate_ = rate;
  return 0 < rate && rate <= 10000000;
}

void
i3ds::EmulatedCamera::handle_exposure(ExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_exposure()";
  auto_exposure_enabled_ = false;
  exposure_ = command.request.exposure;
  ebusCameraInterface->setShutterTime(command.request.exposure);
  gain_ = command.request.gain;
  ebusCameraInterface->setGain(command.request.gain);
}

void
i3ds::EmulatedCamera::handle_auto_exposure(AutoExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure()";
  auto_exposure_enabled_ = command.request.enable;
  BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure: enable: "<< command.request.enable;
  ebusCameraInterface->setAutoExposureEnabled(command.request.enable);

  if (command.request.enable)
    {
      //max_exposure_ = command.request.max_exposure;
      //max_gain_ = command.request.max_gain;
      ebusCameraInterface->setMaxShutterTime(command.request.max_exposure);


    }
}

void
i3ds::EmulatedCamera::handle_region(RegionService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_region()";
  region_enabled_ = command.request.enable;
  ebusCameraInterface->setRegionEnabled(command.request.enable);

  if (command.request.enable)
    {
      ebusCameraInterface->setRegion(command.request.region);
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
  return true;
  BOOST_LOG_TRIVIAL(info) << "send_sample()";
  BOOST_LOG_TRIVIAL(info) << "Send: " << auto_exposure_enabled_ << timestamp_us;

  frame_.attributes.timestamp.microseconds = timestamp_us;
  frame_.attributes.validity = sample_valid;

  publisher_.Send<ImageMeasurement>(frame_);

  return true;
}


void
i3ds::EmulatedCamera::handle_configuration(ConfigurationService::Data& config) const
{
  BOOST_LOG_TRIVIAL(info) << "handle_configuration()";

  config.response.exposure = ebusCameraInterface->getShutterTime();
  config.response.gain = ebusCameraInterface->getGain();
  config.response.auto_exposure_enabled = ebusCameraInterface->getAutoExposureEnabled();
  config.response.max_exposure = ebusCameraInterface->getMaxShutterTime();
  config.response.max_gain = ebusCameraInterface->getMaxShutterTime();
  config.response.region_enabled = ebusCameraInterface->getRegionEnabled();
  config.response.region = ebusCameraInterface->getRegion();
  config.response.flash_enabled = flash_enabled();
  config.response.flash_strength = flash_strength();
  config.response.pattern_enabled = pattern_enabled();
  config.response.pattern_sequence = pattern_sequence();
}
