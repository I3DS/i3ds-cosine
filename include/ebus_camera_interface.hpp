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

#include "i3ds/sensors/camera.hpp"

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






class EbusCameraInterface: protected PvDeviceEventSink {

  public:
    EbusCameraInterface();

    bool connect();
    void collectParameters();
    int64_t getParameter(PvString whichParameter);
    int64_t getMaxParameter(PvString whichParameter);
    int64_t getMinParameter(PvString whichParameter);
    bool getBooleanParameter(PvString whichParameter);
    char * getEnum(PvString whichParameter);
    void setEnum(PvString whichParameter, PvString value);
    bool checkIfEnumOptionIsOK(PvString whichParameter, PvString value);

    bool setIntParameter(PvString whichParameter, int64_t value);

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




};

#endif
