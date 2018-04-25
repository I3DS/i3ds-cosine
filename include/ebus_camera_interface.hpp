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




#include <PvDevice.h>


class EbusCameraInterface: protected PvDeviceEventSink {
  
  public:
    EbusCameraInterface();
    
    bool connect();
    
    
  
  protected:
    
    // Inherited from PvDeviceEventSink.
    void OnLinkDisconnected(PvDevice* aDevice);
    
  private:
  
    bool mConnectionLost;
    
    PvString mConnectionID;
    PvDevice* mDevice;
};

#endif