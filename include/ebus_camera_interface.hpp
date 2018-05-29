///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __I3DS_EBUS_CAMERA_INTERFACE_HPP
#define __I3DS_EBUS_CAMERA_INTERFACE_HPP

#include "i3ds/camera_sensor.hpp"

#include <thread>


#include <PvDevice.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvDeviceInfoGEV.h>
#include <PvDeviceInfoU3V.h>




//#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvDeviceInfoGEV.h>
#include <PvDeviceInfoU3V.h>



  // Does sampling operation, returns true if more samples are requested.
  typedef std::function<bool(unsigned char *image, unsigned long timestamp_us)> Operation;


class EbusCameraInterface: protected PvDeviceEventSink {

  public:
    EbusCameraInterface(const char *connectionString, Operation operation);

    bool connect();
    void collectParameters();

    int64_t getParameter(PvString whichParameter);
    int64_t getMaxParameter(PvString whichParameter);
    int64_t getMinParameter(PvString whichParameter);
    bool setIntParameter(PvString whichParameter, int64_t value);


    bool getBooleanParameter(PvString whichParameter);



    char * getEnum(PvString whichParameter);
    void setEnum(PvString whichParameter, PvString value, bool dontCheckParameter = false);
    bool checkIfEnumOptionIsOK(PvString whichParameter, PvString value);



    int64_t getShutterTime();
    void setShutterTime(int64_t value);

    bool getAutoExposureEnabled();
    void setAutoExposureEnabled(bool value);


    int64_t getGain();
    void setGain(int64_t value);

    void setRegion(PlanarRegion region);
    PlanarRegion getRegion();

    void setRegionEnabled(bool regionEnabled);
    bool getRegionEnabled();


    void setTriggerInterval(int64_t);
    bool checkTriggerInterval(int64_t);


    int64_t getMaxShutterTime();
    void setMaxShutterTime(int64_t);


    void setSourceBothStreams();

    void do_start();
    void do_stop();





  protected:

    // Inherited from PvDeviceEventSink.
    void OnLinkDisconnected(PvDevice* aDevice);

  private:

    bool mConnectionLost;

    PvString mConnectionID;
    PvDevice* mDevice;
    PvGenParameterArray *lParameters;



    //--streaming part
    bool OpenStream();
    void CloseStream();
    bool StartAcquisition();
    bool StopAcquisition();
    void StartSamplingLoop();
    void DisconnectDevice();
    void TearDown(bool aStopAcquisition);


    PvStream* mStream;
    PvPipeline* mPipeline;



    bool stopSamplingLoop;
    std::thread threadSamplingLoop;

    // Sample operation.
     Operation operation_;


    typedef std::chrono::high_resolution_clock clock;

};

#endif
