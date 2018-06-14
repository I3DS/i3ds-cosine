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


#ifdef BASLER_CAMERA
#ifdef TOF_CAMERA
#include "basler_tof_interface.hpp"
#endif


#ifdef HR_CAMERA
#include "../include/basler_high_res_interface.hpp"
#endif
#endif


#ifdef EBUS_CAMERA
#include "../include/ebus_camera_interface.hpp"
#endif




#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>



namespace logging = boost::log;





template <class MeasurementTopic>
i3ds::EmulatedCamera<MeasurementTopic>::EmulatedCamera(Context::Ptr context, NodeID node, int resx, int resy,
					     std::string ipAddress, std::string camera_name, bool free_running)
  : Camera(node),
    resx_(resx),
    resy_(resy),
    free_running_(free_running),
//,
   // sampler_(std::bind(&i3ds::EmulatedCamera::send_sample, this, std::placeholders::_1)),
    publisher_(context, node)
{
  BOOST_LOG_TRIVIAL(info) << "EmulatedCamera::EmulatedCamera()";
#ifdef EBUS_CAMERA
  cameraInterface = new EbusCameraInterface(ipAddress.c_str(), camera_name.c_str(), free_running,
  						std::bind(&i3ds::EmulatedCamera<MeasurementTopic>::send_sample, this,
  							  std::placeholders::_1, std::placeholders::_2));
 /* cameraInterface = new EbusCameraInterface("10.0.1.117",
						std::bind(&i3ds::EmulatedCamera<Codec>::send_sample, this,
							  std::placeholders::_1, std::placeholders::_2));
*/

#endif

#ifdef BASLER_CAMERA
#ifdef HR_CAMERA
  cameraInterface = new BaslerHighResInterface(ipAddress.c_str(), camera_name.c_str(), free_running,
   						std::bind(&i3ds::EmulatedCamera<MeasurementTopic>::send_sample, this,
 							  std::placeholders::_1, std::placeholders::_2));
#endif


#ifdef TOF_CAMERA
  cameraInterface = new Basler_ToF_Interface(	ipAddress.c_str(),camera_name.c_str(), free_running,
  						std::bind(&i3ds::EmulatedCamera<MeasurementTopic>::send_sample, this,
				std::placeholders::_1, std::placeholders::_2));
#endif
#endif



  shutter_ = 0;
  gain_ = 0.0;

  auto_exposure_enabled_ = false;
  max_shutter_ = 0;
  max_gain_ = 0.0;

  region_.size_x = resx;
  region_.size_y = resy;
  region_.offset_x = 0;
  region_.offset_y = 0;

  flash_enabled_ = false;
  flash_strength_ = 0.0;

  pattern_enabled_ = false;
  pattern_sequence_ = 0;
  //CameraMeasurement4MCodec::Initialize(frame_);
  MeasurementTopic::Codec::Initialize(frame_);
#ifdef TOF_CAMERA
  ;
#else
  frame_.frame_mode = mode_mono;
  frame_.data_depth = 12;
  frame_.pixel_size = 2;
#endif

  frame_.region.size_x = resx_;
  frame_.region.size_y = resy_;

#ifdef STEREO_CAMERA
  frame_.image_left.nCount = resx_ * resy_ * 2;
  frame_.image_right.nCount = resx_ * resy_ * 2;

#elif defined(TOF_CAMERA)
  frame_.distances.nCount = resx_ * resy_ * 2;

#else
  frame_.image.nCount = resx_ * resy_ * 2;
#endif
}

template <class MeasurementTopic>
i3ds::EmulatedCamera<MeasurementTopic>::~EmulatedCamera()
{
}

template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::do_activate()
{
  BOOST_LOG_TRIVIAL(info) << "do_activate()";
  cameraInterface->connect();

#ifdef STEREO_CAMERA
  cameraInterface->setSourceBothStreams();
#endif

  shutter_ = cameraInterface->getShutterTime();
  BOOST_LOG_TRIVIAL(info) << "Shutter_: " << shutter_;

  PlanarRegion planarRegion = cameraInterface->getRegion();


 frame_.region.size_x = resx_ = planarRegion.size_x;

 // TODO Can simplifies


#ifdef TOF_CAMERA
   frame_.region.size_y = resy_ = planarRegion.size_y;
 // frame_.image_left.nCount = resx_ * resy_ * 2;
 // frame_.image_right.nCount = resx_ * resy_ * 2;
 ;
#elif defined(STEREO_CAMERA)
  frame_.region.size_y = resy_ = planarRegion.size_y/2;
  frame_.image_left.nCount = resx_ * resy_ * 2;
  frame_.image_right.nCount = resx_ * resy_ * 2;
#else
  frame_.region.size_y = resy_ = planarRegion.size_y;
  frame_.image.nCount = resx_ * resy_ * 2;
#endif


}


// \todo handle rate. What if not sat?
template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::do_start()
{
  BOOST_LOG_TRIVIAL(info) << "do_start()";
  //sampler_.Start(rate());

  cameraInterface->do_start();

}

template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::do_stop()
{
  BOOST_LOG_TRIVIAL(info) << "do_stop()";

  cameraInterface->do_stop();


 // sampler_.Stop();
}

template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::do_deactivate()
{
  BOOST_LOG_TRIVIAL(info) << "do_deactivate()";
  cameraInterface->do_deactivate ();
}



template <class MeasurementTopic>
bool
i3ds::EmulatedCamera<MeasurementTopic>::is_sampling_supported(SampleCommand sample)
{
  BOOST_LOG_TRIVIAL(info) << "is_rate_supported() " << sample.period;
  return cameraInterface->checkTriggerInterval(sample.period);
}


// \todo All parameter must be s at in client or thy wil default to 0. Do we need a don't care state?
// \todo What if first parameter throws, then the second wil not be sat.
template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::handle_exposure(ExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_exposure()";
  if(is_active() == false){
      BOOST_LOG_TRIVIAL(info) << "handle_exposure()-->Not in active state";

      std::ostringstream errorDescription;
      errorDescription << "handle_exposure: Not in active state";
      throw i3ds::CommandError(error_value, errorDescription.str());
    }

  auto_exposure_enabled_ = false;
  shutter_ = command.request.shutter;
  cameraInterface->setShutterTime(shutter_);
  gain_ = command.request.gain;
  cameraInterface->setGain(gain_);
}


// \todo All parameter must be sat in client or they will default to 0. Do we need a don't care state?
// \todo What if first parameter throws, then the second will not be sat.
template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::handle_auto_exposure(AutoExposureService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure()";
  if(!(is_active() || is_operational())){
    BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure()-->Not in active or operational state";

    std::ostringstream errorDescription;
    errorDescription << "handle_auto_exposure: Not in active or operational state";
    throw i3ds::CommandError(error_value, errorDescription.str());
  }
  auto_exposure_enabled_ = command.request.enable;
  BOOST_LOG_TRIVIAL(info) << "handle_auto_exposure: enable: "<< command.request.enable;
  cameraInterface->setAutoExposureEnabled(command.request.enable);

  if (command.request.enable)
    {
      //max_exposure_ = command.request.max_exposure;
      //max_gain_ = command.request.max_gain;
      cameraInterface->setMaxShutterTime(command.request.max_shutter);


    }
}


// \todo All parameter must be sat in client or they will default to 0. Do we need a don't care state?
// \todo What if first parameter throws, then the second will not be set.
template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::handle_region(RegionService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_region()";
  if(!(is_active())){
    BOOST_LOG_TRIVIAL(info) << "handle_region()-->Not in active state";

    std::ostringstream errorDescription;
    errorDescription << "handle_region: Not in active state";
    throw i3ds::CommandError(error_value, errorDescription.str());
  }

  region_enabled_ = command.request.enable;
  cameraInterface->setRegionEnabled(command.request.enable);

  if (command.request.enable)
    {
      cameraInterface->setRegion(command.request.region);
    }
}

template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::handle_flash(FlashService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "handle_flash()";
  if(!(is_active() || is_operational())){
    BOOST_LOG_TRIVIAL(info) << "handle_flash()-->Not in active or operational state";

    std::ostringstream errorDescription;
    errorDescription << "handle_flash(): Not in active or operational state";
    throw i3ds::CommandError(error_value, errorDescription.str());
  }

  flash_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      flash_strength_ = command.request.strength;
    }
}

template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::handle_pattern(PatternService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "do_pattern()";
  if(!(is_active() || is_operational())){
     BOOST_LOG_TRIVIAL(info) << "do_pattern()-->Not in active or operational state";

     std::ostringstream errorDescription;
     errorDescription << "do_pattern(): Not in active or operational state";
     throw i3ds::CommandError(error_value, errorDescription.str());
   }



  pattern_enabled_ = command.request.enable;

  if (command.request.enable)
    {
      pattern_sequence_ = command.request.sequence;
    }
}

template <class MeasurementTopic>
bool
i3ds::EmulatedCamera<MeasurementTopic>::send_sample(unsigned char *image, unsigned long timestamp_us)
{

  BOOST_LOG_TRIVIAL(info) << "EmulatedCamera::send_sample()x";
/* BOOST_LOG_TRIVIAL(info) << "Send: " << auto_exposure_enabled_ << timestamp_us;
  printf("frame_.image.arr: %p\n", frame_.image.arr);
  printf("image: %p\n", image);
  printf("frame_.image.nCount: %d\n",frame_.image.nCount);
  printf("frame_.region.size_x: %d\n",frame_.region.size_x );
  printf("frame_.region.size_y: %d\n",frame_.region.size_y );
*/

#ifdef TOF_CAMERA
  /*
  typedef struct {
      SampleAttributes attributes;
      ToFMeasurement500K_distances distances;
      ToFMeasurement500K_validity validity;
      PlanarRegion region;
  } ToFMeasurement500K;
  */


  //memcpy(frame_.image.distances, image,  frame_.image.nCount);
  //memcpy(frame_.image.validity, image,  frame_.image.nCount);
;
BOOST_LOG_TRIVIAL(info) << "TOF:send_sample() ";
#endif

#ifdef STEREO_CAMERA
  BOOST_LOG_TRIVIAL(info) << "Stereo:send_sample() nCount left:right : "
      << frame_.image_left.nCount
      << frame_.image_right.nCount;
  memcpy(frame_.image_left.arr, image,  frame_.image_left.nCount);
  memcpy(frame_.image_right.arr, image + frame_.image_left.nCount,  frame_.image_right.nCount);
  BOOST_LOG_TRIVIAL(info) << "send_sample()"<< +frame_.image_left.arr[100]<<" "<< +frame_.image_left.arr[101];
#endif

#ifndef STEREO_CAMERA
#ifndef TOF_CAMERA
  BOOST_LOG_TRIVIAL(info) << "Other camera:send_sample() nCount  : "
       << frame_.image.nCount;
  printf("frame_.image.arr: %p\n", frame_.image.arr);
  printf("image: %p\n", image);
  memcpy(frame_.image.arr, image,  frame_.image.nCount);
  BOOST_LOG_TRIVIAL(info) << "Other camera:send_sample() end";
#endif
#endif

  frame_.attributes.timestamp.microseconds = timestamp_us;
  frame_.attributes.validity = sample_valid;
  publisher_.Send<MeasurementTopic>(frame_);

  return true;
}


template <class MeasurementTopic>
void
i3ds::EmulatedCamera<MeasurementTopic>::handle_configuration(ConfigurationService::Data& config) const
{
  BOOST_LOG_TRIVIAL(info) << "handle_configuration()";

  config.response.shutter = cameraInterface->getShutterTime();
  config.response.gain = cameraInterface->getGain();
  config.response.auto_exposure_enabled = cameraInterface->getAutoExposureEnabled();
  config.response.max_shutter = cameraInterface->getMaxShutterTime();
  config.response.max_gain = cameraInterface->getMaxShutterTime();
  config.response.region_enabled = cameraInterface->getRegionEnabled();
  config.response.region = cameraInterface->getRegion();
  config.response.flash_enabled = flash_enabled();
  config.response.flash_strength = flash_strength();
  config.response.pattern_enabled = pattern_enabled();
  config.response.pattern_sequence = pattern_sequence();
}