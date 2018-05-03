find_package (Boost COMPONENTS program_options log REQUIRED)


set(PUREGEV_ROOT    "$ENV{PUREGEV_ROOT}")
set(PV_LIBRARY_PATH "${PUREGEV_ROOT}/lib")


message(STATUS "PUREGEV_ROOT=${PUREGEV_ROOT}")


set (SRCS
   emulated_camera.cpp
  # periodic_sampler.cpp
   ebus_camera_interface.cpp
  )

set (LIBS
  i3ds
  zmq

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


include_directories ("../include/")
include_directories ("${CMAKE_CURRENT_BINARY_DIR}/src/core")

include_directories ("${PUREGEV_ROOT}/include") 

link_directories("${PUREGEV_ROOT}/lib/")



foreach(a ${APPS})
  add_executable (${a} ${a}.cpp ${SRCS})
  target_link_libraries (${a} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS ${a} DESTINATION bin)
endforeach(a)
