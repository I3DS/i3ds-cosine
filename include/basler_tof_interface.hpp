///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __I3DS_BASLER_TOF_INTERFACE_HPP
#define __I3DS_BASLER_TOF_INTERFACE_HPP


#include <ConsumerImplHelper/ToFCamera.h>

#include <i3ds/topic.hpp>
#include <i3ds/tof_camera_sensor.hpp>

// #include "i3ds/camera_sensor.hpp"


  // Does sampling operation, returns true if more samples are requested.
  typedef std::function<bool(unsigned char *image, unsigned long timestamp_us)> Operation;


using namespace GenTLConsumerImplHelper;

class Basler_ToF_Interface
  {
  public:

  //typedef Topic<128, ToFMeasurement500KCodec> ToFMeasurement;

    void Basler_ToF_Interface2 ();
    Basler_ToF_Interface(const char *connectionString, Operation operation);
    ~Basler_ToF_Interface ();
    void do_activate();
    void do_start();
    void do_stop();
    void StartSamplingLoop ();
    void do_deactivate ();

    int64_t getShutterTime();
    void setShutterTime(int64_t value);

    void setRegion(PlanarRegion region);
    PlanarRegion getRegion();

    void setTriggerInterval(int64_t);
    bool checkTriggerInterval(int64_t);

    bool getAutoExposureEnabled();
    void setAutoExposureEnabled(bool value);

    int64_t getGain();
    void setGain(int64_t value);


    void setRegionEnabled(bool regionEnabled);
    bool getRegionEnabled();

    int64_t getMaxShutterTime();
    void setMaxShutterTime(int64_t);

    bool connect();



    bool onImageGrabbed (GrabResult grabResult, BufferParts);

  private:
    CToFCamera m_Camera;
    int m_nBuffersGrabbed;

    std::thread threadSamplingLoop;
    bool stopSamplingLoop;
    const char * mConnectionID;


    // Sample operation.
    Operation operation_;


    typedef std::chrono::high_resolution_clock clock;
  };

#endif
