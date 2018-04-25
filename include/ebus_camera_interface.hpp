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


#include <PvDevice.h>


class EbusCameraInterface: protected PvDeviceEventSink {
  
  public:
    EbusCameraInterface();
    
    bool connect();
    void collectParameters();
    int64_t getParameter(PvString whichParameter);
    int64_t getMaxParameter(PvString whichParameter);
    bool getBooleanParameter(PvString whichParameter);
    char * getEnum(PvString whichParameter);
    int64_t getExposure();
    int64_t getGain();
    PlanarRegion getRegion();
    int64_t getMaxGain();
    int64_t getMaxExposure();
    bool getAutoExposureEnabled();
    bool getRegionEnabled();


    
    
  
  protected:
    
    // Inherited from PvDeviceEventSink.
    void OnLinkDisconnected(PvDevice* aDevice);
    
  private:
  
    bool mConnectionLost;
    
    PvString mConnectionID;
    PvDevice* mDevice;
    PvGenParameterArray *lParameters;
};

#endif
