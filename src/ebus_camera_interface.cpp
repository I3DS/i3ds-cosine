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

#include "../include/ebus_camera_interface.hpp"


#include <boost/program_options.hpp>

#include "i3ds/core/communication.hpp"
#include "i3ds/emulators/emulated_camera.hpp"

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>





namespace po = boost::program_options;
namespace logging = boost::log;



EbusCameraInterface::EbusCameraInterface() {
  BOOST_LOG_TRIVIAL(info) << "EbusCameraInterface constructor";

 
}



bool EbusCameraInterface::connect() {
  BOOST_LOG_TRIVIAL(info) << "Connecting to camera";
  mConnectionID = "10.0.1.111";
  
  std::cout << "--> ConnectDevice " << mConnectionID.GetAscii() << std::endl;

  // Connect to the selected Device
  PvResult lResult = PvResult::Code::INVALID_PARAMETER;
  mDevice = PvDevice::CreateAndConnect(mConnectionID, &lResult);
  if (!lResult.IsOK()) {
      BOOST_LOG_TRIVIAL(info) << "CreateAndConnect problem";
  	return false;
  }
  
  
  // Register this class as an event sink for PvDevice call-backs
  mDevice->RegisterEventSink(this);

  // Clear connection lost flag as we are now connected to the device
  mConnectionLost = false;
  BOOST_LOG_TRIVIAL(info) << "Connected to Camera";
  
  collectParameters();
  return true;
}
  
  
  

void EbusCameraInterface::OnLinkDisconnected(PvDevice *aDevice) {
  BOOST_LOG_TRIVIAL(info) << "=====> PvDeviceEventSink::OnLinkDisconnected callback";

  mConnectionLost = true;

	// IMPORTANT:
	// The PvDevice MUST NOT be explicitly disconnected from this callback.
	// Here we just raise a flag that we lost the device and let the main loop
	// of the application (from the main application thread) perform the
	// disconnect.
	//

}


void EbusCameraInterface::collectParameters() {
  BOOST_LOG_TRIVIAL(info) << "Collecting Camera parameters";
  lParameters = mDevice->GetParameters();
}



int64_t EbusCameraInterface::getParameter(PvString whichParameter) {

  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: " << whichParameter.GetAscii() ;

  PvGenParameter *lParameter = lParameters->Get(whichParameter);
  PvGenInteger *lIntParameter = dynamic_cast<PvGenInteger *>(lParameter);

  if (lIntParameter == NULL) {
    std::cout << "Unable to get the parameter: "<< whichParameter.GetAscii() << std::endl;
  }

  // Read current width value.
  int64_t lParameterValue = 0;
  if (!(lIntParameter->GetValue(lParameterValue).IsOK())) {
    BOOST_LOG_TRIVIAL(info) << "Error retrieving parameter from device";
    return  0;
  } else {
    BOOST_LOG_TRIVIAL(info) << "Parametervalue: " << lParameterValue <<
		" returned from parameter: " << whichParameter.GetAscii();
    return lParameterValue;
  }
}


// Get maximum allowed value of parameter.
int64_t EbusCameraInterface::getMaxParameter(PvString whichParameter) {

  PvGenParameter *lParameter = lParameters->Get(whichParameter);

  // Converter generic parameter to width using dynamic cast. If the
  // type is right, the conversion will work otherwise lWidth will be NULL.
  PvGenInteger *lMaxParameter = dynamic_cast<PvGenInteger *>(lParameter);

  if (lMaxParameter == NULL) {
      BOOST_LOG_TRIVIAL(info) << "Unable to get parameter " << whichParameter.GetAscii();
  }

  int64_t lMaxAllowedValue = 0;
  if (!(lMaxParameter->GetMax(lMaxAllowedValue).IsOK())) {
      BOOST_LOG_TRIVIAL(info) << "Error retrieving max value from device for parameter " <<
	  whichParameter.GetAscii();
      return 0;
  } else {
      BOOST_LOG_TRIVIAL(info) << "Max allowed value for parameter: " << whichParameter.GetAscii() 
	  <<  " is " << lMaxAllowedValue;
    return lMaxAllowedValue;
  }

}



char * EbusCameraInterface::getEnum(PvString whichParameter){

  PvGenParameter *lGenParameter = lParameters->Get(whichParameter);

  // Parameter available?
  if ( !lGenParameter->IsAvailable() )
  {
    BOOST_LOG_TRIVIAL(info) << "{Not Available}";
    return nullptr;
  }

  // Parameter readable?
  if ( !lGenParameter->IsReadable() )
  {
    BOOST_LOG_TRIVIAL(info) << "{Not readable}";
    return nullptr;
  }

  // Get the parameter type
  PvGenType lType;
  lGenParameter->GetType( lType );
  if(lType == PvGenTypeEnum)
  {
    PvString lValue;
    static_cast<PvGenEnum *>( lGenParameter )->GetValue( lValue );
    BOOST_LOG_TRIVIAL(info) << "Enum: " << lValue.GetAscii();
    char * str;
    
    asprintf(&str, "%s", lValue.GetAscii());
    return str;
		    
			    /*
			
			To pull out enums options
			
			int64_t aCount;
			static_cast<PvGenEnum *>( lGenParameter )->GetEntriesCount (aCount);
			cout << "Enum entries: " << aCount << endl;

			for(int i=0; i<aCount;i++){

				const PvGenEnumEntry *aEntry;

				static_cast<PvGenEnum *>( lGenParameter )->GetEntryByValue(i, &aEntry);
				PvString s;
				aEntry->GetName(s);
				cout << "EnumText: "<< s.GetAscii() << endl;
			}
			
			*/
        }else
        {
          BOOST_LOG_TRIVIAL(info) << "Not a enum ";
          return nullptr;
        }

}





bool EbusCameraInterface::getBooleanParameter(PvString whichParameter) {
  PvGenParameter *lParameter = lParameters->Get(whichParameter);

  bool lValue;
  static_cast<PvGenBoolean *>( lParameter )->GetValue( lValue );
  
  if( lValue )
  {
    BOOST_LOG_TRIVIAL(info) << whichParameter.GetAscii() << "Boolean: TRUE";
    return true;
  }
  else
  {
    BOOST_LOG_TRIVIAL(info) << whichParameter.GetAscii() << "Boolean: FALSE";
    return true;
  }
}

 
// Todo This is actually a boolean!!!!
bool EbusCameraInterface::getAutoExposureEnabled(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: getAutoExposureEnabled";

  char *str = getEnum("AutoExposure");
  bool retval;

  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: getAutoExposureEnabled: " << str;

  if(strcmp (str, "ON")==0){
   retval = true;
  }
   else{
   retval = false;
  }
  BOOST_LOG_TRIVIAL(info) << retval;

  free(str);
  return retval;
}


bool EbusCameraInterface::getRegionEnabled(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: RegionEnabled";
  return true;
}


int64_t EbusCameraInterface::getExposure(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: exposure"; 
  return getParameter("GainValueLeft");
}

int64_t EbusCameraInterface::getMaxExposure(){

  BOOST_LOG_TRIVIAL(info) << "Fetching max of parameter: exposure"; 
  return getMaxParameter("GainValueLeft");  
}


int64_t EbusCameraInterface::getGain(){
  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: GainValue";
  return getParameter("GainValue");
}


int64_t EbusCameraInterface::getMaxGain(){

   BOOST_LOG_TRIVIAL(info) << "Fetching max of parameter: GainValue"; 
   return getMaxParameter("GainValue");  
}



PlanarRegion EbusCameraInterface::getRegion(){

  BOOST_LOG_TRIVIAL(info) << "Fetching parameter: region";

  int64_t size_x = getParameter("Width");
  int64_t size_y = getParameter("Height");
  int64_t offset_x = getParameter("OffsetY");
  int64_t offset_y = getParameter("RegionHeight");
  
  PlanarRegion region;
   
  region.offset_x = offset_x;
  region.offset_y = offset_y;
  region.size_x = size_x;
  region.size_y= size_y;  

  return region;
}

