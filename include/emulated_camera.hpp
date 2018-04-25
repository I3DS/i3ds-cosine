///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////





#ifndef __I3DS_EMULATED_CAMERA_HPP
#define __I3DS_EMULATED_CAMERA_HPP

#include "i3ds/core/topic.hpp"
#include "i3ds/core/publisher.hpp"
#include "i3ds/sensors/camera.hpp"
#include "i3ds/emulators/periodic_sampler.hpp"

#include "ebus_camera_interface.hpp"

namespace i3ds
{

class EmulatedCamera : public Camera
{
public:

  typedef Topic<128, CameraMeasurement4MCodec> ImageMeasurement;

  EmulatedCamera(Context::Ptr context, NodeID id, int resx, int resy);
  virtual ~EmulatedCamera();

  // Getters.
  virtual ExposureTime exposure() const {return exposure_;}
  virtual SensorGain gain() const {return gain_;}
  virtual bool auto_exposure_enabled() const {return auto_exposure_enabled_;}
  virtual ExposureTime max_exposure() const {return max_exposure_;}
  virtual SensorGain max_gain() const {return max_gain_;}
  virtual bool region_enabled() const {return region_enabled_;}
  virtual PlanarRegion region() const {return region_;}
  virtual bool flash_enabled() const {return flash_enabled_;}
  virtual FlashStrength flash_strength() const {return flash_strength_;}
  virtual bool pattern_enabled() const {return pattern_enabled_;}
  virtual PatternSequence pattern_sequence() const {return pattern_sequence_;}

  // Supported rate.
  virtual bool is_rate_supported(SampleRate rate);

protected:

  // Actions.
  virtual void do_activate();
  virtual void do_start();
  virtual void do_stop();
  virtual void do_deactivate();

  // Handlers.
  virtual void handle_exposure(ExposureService::Data& command);
  virtual void handle_region(RegionService::Data& command);
  virtual void handle_auto_exposure(AutoExposureService::Data& command);
  virtual void handle_flash(FlashService::Data& command);
  virtual void handle_pattern(PatternService::Data& command);

private:

  bool send_sample(unsigned long timestamp_us);

  const int resx_;
  const int resy_;

  ExposureTime exposure_;
  SensorGain gain_;
  bool auto_exposure_enabled_;
  ExposureTime max_exposure_;
  SensorGain max_gain_;
  bool region_enabled_;
  PlanarRegion region_;
  bool flash_enabled_;
  FlashStrength flash_strength_;
  bool pattern_enabled_;
  PatternSequence pattern_sequence_;

  Sampler sampler_;

  Publisher publisher_;
  CameraMeasurement4M frame_;

  EbusCameraInterface * ebusCameraInterface;

};

} // namespace i3ds

#endif
