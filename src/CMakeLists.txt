find_package (Boost COMPONENTS program_options log REQUIRED)


set(PUREGEV_ROOT    "$ENV{PUREGEV_ROOT}")
set(PV_LIBRARY_PATH "${PUREGEV_ROOT}/lib")


message(STATUS "PUREGEV_ROOT=${PUREGEV_ROOT}")


set (SRCS
   # emulated_camera.cpp
  # periodic_sampler.cpp
   ebus_camera_interface.cpp
  )

set (LIBS
  i3ds
  zmq
  )

set (PLEORA_LIBS 
  PvBase
  PvDevice
  PvBuffer
  PvGenICam
  PvStream
  PvTransmitter
  PvVirtualDevice
  PvCameraBridge
  PvAppUtils
  PvPersistence
  PvSystem
  PvSerial

  libSimpleImagingLib.so
  )

set (APPS
 # cosine-tir
  i3ds_suite_emulator
  )
  
add_definitions(-D_UNIX_)

set (PLEORA_INCLUDE_DIRECTORIES 
  ${CMAKE_CURRENT_BINARY_DIR}/src/
  ${PUREGEV_ROOT}/include
  )

set (PLEORA_LINK_DIRECTORIES
  ${PUREGEV_ROOT}/lib/
)

include_directories ("../include/")


#find_library(PROTOBUF_LIBRARY protobuf HINTS /usr/lib/x86_64-linux-gnu)


find_library(PLEORA_LIBRARY PvBase HINTS "${PUREGEV_ROOT}/lib/")


link_directories("${PUREGEV_ROOT}/lib/")


#BASLER_TOF
set (GENTL_ROOT /opt/BaslerToF)
set (GENAPI_POSTFIX gcc_v3_0_Basler_pylon_v5_0)
set (BASLER_LIBDIR  ${GENTL_ROOT}/lib64)
set (BASLER_TOF_INCLUDES ${GENTL_ROOT}/include)
set (BASLER_LIBS 
#      L${LIBDIR} 
      GenApi_${GENAPI_POSTFIX} 
      GCBase_${GENAPI_POSTFIX} 
      dl 
      pthread
)

set (BASLER_LIB_FLAGS Wl 
      enable-new-dtags Wl
      rpath ${LIBDIR} Wl
      rpath ${LIBDIR}/gentlproducer/gtl 
 
       Wl E
)
set (BASLER_CXX_FLAGS 
      std=c++11 Wall Wno-unknown-pragmas
)



##foreach(a ${APPS})
##  add_executable (${a} ${a}.cpp ${SRCS})
##  target_link_libraries (${a} ${LIBS} ${Boost_LIBRARIES})
#  install(TARGETS ${a} DESTINATION bin)
#endforeach(a)


# i3ds-thermal:  10.0.1.111
# [no name set]: 10.0.1.115
#  ti3ds-highres:  10.0.1.116
#  i3ds-stereo:   10.0.1.117


#basler  
#10.0.1.110


#PLEORA Normal
#PLEORA HR
#PLEORA Stereo
#PLEORA TIR

#BASLER HR
#BASLER TOF



#PLEORA normal
#add_executable (PleoraNormal i3ds_suite_emulator.cpp ${SRCS})
#  target_compile_definitions(PleoraNormal PRIVATE EBUS_CAMERA=1 DEFAULT_NODE_ID=15 DEFAULT_IP_ADDRESS="10.0.1.115")
#  target_link_libraries (PleoraNormal ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
#  install(TARGETS PleoraNormal DESTINATION bin)
  
  
  
  
#PLEORA hr
add_executable (PleoraHr i3ds_suite_emulator.cpp ${SRCS})
  target_include_directories(PleoraHr PRIVATE ${PLEORA_INCLUDE_DIRECTORIES})
  target_compile_definitions(PleoraHr PRIVATE EBUS_CAMERA=1 HR_CAMERA=1 DEFAULT_NODE_ID=16 DEFAULT_IP_ADDRESS="10.0.1.116")
  target_link_libraries (PleoraHr ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS PleoraHr DESTINATION bin)
  
#PLEORA TIR
add_executable (PleoraTir i3ds_suite_emulator.cpp ${SRCS})
  target_include_directories(PleoraTir PRIVATE ${PLEORA_INCLUDE_DIRECTORIES})
  target_compile_definitions(PleoraTir PRIVATE EBUS_CAMERA=1  DEFAULT_NODE_ID=11 DEFAULT_IP_ADDRESS="10.0.1.111")
  target_link_libraries (PleoraTir ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS PleoraTir DESTINATION bin)
  


#PLEORA Stereo
  add_executable (PleoraStereo i3ds_suite_emulator.cpp ${SRCS})
  target_include_directories(PleoraStereo PRIVATE ${PLEORA_INCLUDE_DIRECTORIES})
  target_compile_definitions(PleoraStereo PRIVATE STEREO_CAMERA=1 EBUS_CAMERA=1 DEFAULT_NODE_ID=17 DEFAULT_IP_ADDRESS="10.0.1.117")
  target_link_libraries (PleoraStereo ${PROTOBUF_LIBRARY}  ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS PleoraStereo DESTINATION bin)
  


#Basler normal
  #add_executable (BaslerNormal i3ds_suite_emulator.cpp ${SRCS})
  #target_compile_definitions(BaslerNormal PRIVATE BASLER_CAMERA=1 DEFAULT_NODE_ID=12 DEFAULT_IP_ADDRESS)
  #target_link_libraries (BaslerNormal ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
  #install(TARGETS BaslerNormal DESTINATION bin)

#Basler HR
# add_executable (BaslerHr i3ds_suite_emulator.cpp ${SRCS})
# target_compile_definitions(BaslerHr PRIVATE BASLER_CAMERA=1 HR_CAMERA=1 DEFAULT_NODE_ID=12 DEFAULT_IP_ADDRESS="10.0.1.110")
# target_link_libraries (BaslerHr ${PLEORA_LIBS} ${LIBS} ${Boost_LIBRARIES})
# install(TARGETS BaslerHr DESTINATION bin)




#Basler ToF
  add_executable (BaslerToF i3ds_suite_emulator.cpp basler_tof_interface.cpp )
  target_include_directories(BaslerToF PRIVATE ${BASLER_TOF_INCLUDES})
  target_compile_definitions(BaslerToF PRIVATE  BASLER_CAMERA=1 TOF_CAMERA=1 DEFAULT_NODE_ID=10 DEFAULT_IP_ADDRESS="10.0.1.110")
  target_link_libraries (BaslerToF ${BASLER_LIBS} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS BaslerToF DESTINATION bin)
  



  


# Wide angle flash
#SerialCommunicator

  add_executable (SerialCommunicator SerialCommunicator.cpp)
  target_link_libraries (SerialCommunicator ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS SerialCommunicator DESTINATION bin)

