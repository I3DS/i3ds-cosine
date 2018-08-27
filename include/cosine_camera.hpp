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
#include <i3ds/trigger_client.hpp>

#include <i3ds/flash_client.hpp>



#include "serial_communicator.hpp"

#include <memory>

class EbusWrapper;

namespace i3ds
{

class CosineCamera : public Camera
{
public:

  struct Parameters
  {
    std::string camera_name;
    bool is_stereo;
    bool free_running;
    int trigger_scale;
    int data_depth;

    TriggerGenerator trigger_generator;
    TriggerOutput trigger_camera_output;
    TriggerOffset trigger_camera_offset;

    TriggerOutput flash_output;
    TriggerOffset flash_offset;

    bool trigger_camera_inverted; // Not used at the moment

    TriggerOutput trigger_pattern_output;
    TriggerOffset trigger_pattern_offset;
    std::string wa_flash_port;


  };

  CosineCamera(Context::Ptr context, NodeID id, NodeID flash_node_id, Parameters param, TriggerClient::Ptr trigger = nullptr);

  virtual ~CosineCamera();

  // Getters.
  virtual ShutterTime shutter() const;
  virtual SensorGain gain() const;

  virtual bool auto_exposure_enabled() const;

  virtual ShutterTime max_shutter() const;
  virtual SensorGain max_gain() const;

  virtual PlanarRegion region() const;

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

  const Parameters param_;

  int64_t to_trigger(SamplePeriod period);
  SamplePeriod to_period(int64_t trigger);
  void set_trigger(TriggerOutput channel, TriggerOffset offset);
  void clear_trigger(TriggerOutput channel);


  bool send_sample(unsigned char* image, int width, int height);

  std::unique_ptr<SerialCommunicator> flash_configurator_;


  void updateRegion();

  bool flash_enabled_;
  FlashStrength flash_strength_;

  bool pattern_enabled_;
  PatternSequence pattern_sequence_;

  Publisher publisher_;

  std::unique_ptr<EbusWrapper> ebus_;

  TriggerClient::Ptr trigger_;
  TriggerOutputSet trigger_outputs_;

  FlashClient flash_client;

};

} // namespace i3ds

#endif
