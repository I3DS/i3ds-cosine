///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////


#ifndef __COSINE_CAMERA_HPP
#define __COSINE_CAMERA_HPP

#include "i3ds/topic.hpp"
#include "i3ds/publisher.hpp"
#include "i3ds/camera_sensor.hpp"

#include <memory>

class EbusWrapper;

namespace i3ds
{

class CosineCamera : public Camera
{
public:

  CosineCamera(Context::Ptr context, NodeID id,
               std::string ipAddress,
               std::string camera_name,
	       bool is_stereo,
               bool free_running);

  virtual ~CosineCamera();

  // Getters.
  virtual ShutterTime shutter() const;
  virtual SensorGain gain() const;

  virtual bool auto_exposure_enabled() const;

  virtual ShutterTime max_shutter() const;
  virtual SensorGain max_gain() const;

  virtual PlanarRegion region() const {return region_;}

  virtual bool flash_enabled() const {return flash_enabled_;}
  virtual FlashStrength flash_strength() const {return flash_strength_;}

  virtual bool pattern_enabled() const {return pattern_enabled_;}
  virtual PatternSequence pattern_sequence() const {return pattern_sequence_;}

  virtual bool is_sampling_supported(SampleCommand sample);

protected:

  // Actions.
  virtual void do_activate();
  virtual void do_start();
  virtual void do_stop();
  virtual void do_deactivate();

  // Handlers.
  virtual void handle_exposure(ExposureService::Data& command);
  virtual void handle_auto_exposure(AutoExposureService::Data& command);
  virtual void handle_flash(FlashService::Data& command);
  virtual void handle_pattern(PatternService::Data& command);

private:

  bool send_sample(unsigned char * image, unsigned long timestamp_us);

  void updateRegion();

  const bool is_stereo_;
  const bool free_running_;

  PlanarRegion region_;
  
  bool flash_enabled_;
  FlashStrength flash_strength_;

  bool pattern_enabled_;
  PatternSequence pattern_sequence_;

  Publisher publisher_;

  Camera::FrameTopic::Data frame_;

  std::unique_ptr<EbusWrapper> ebus_;
};

} // namespace i3ds

#endif
