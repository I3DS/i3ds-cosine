find_package (Boost COMPONENTS program_options log REQUIRED)

set(PUREGEV_ROOT "/opt/pleora/ebus_sdk/Ubuntu-x86_64")

set(PV_LIBRARY_PATH "${PUREGEV_ROOT}/lib")

message(STATUS "PUREGEV_ROOT=${PUREGEV_ROOT}")


set (SRCS
   ebus_wrapper.cpp
   cosine_camera.cpp
   )

set (LIBS
  i3ds
  zmq
  pthread
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
  
add_definitions(-D_UNIX_)

set (PLEORA_INCLUDE_DIRECTORIES 
  ${CMAKE_CURRENT_BINARY_DIR}/src/
  ${PUREGEV_ROOT}/include
  )

set (PLEORA_LINK_DIRECTORIES
  ${PUREGEV_ROOT}/lib/
)

include_directories (${PLEORA_INCLUDE_DIRECTORIES})
include_directories ("../include/")

add_executable (i3ds_cosine_camera i3ds_cosine_camera.cpp ${SRCS})
target_compile_options(i3ds_cosine_camera PRIVATE -Wno-unknown-pragmas)
target_link_libraries (i3ds_cosine_camera -L${PUREGEV_ROOT}/lib/ ${LIBS} ${Boost_LIBRARIES})
install(TARGETS i3ds_cosine_camera DESTINATION bin)

set (CAMERA_TYPES "hr" "stereo"  "tir")

foreach(CAMERA_TYPE ${CAMERA_TYPES})
  configure_file(start_cosine.sh i3ds_cosine_${CAMERA_TYPE})
  install(FILES i3ds_cosine_${CAMERA_TYPE} DESTINATION bin)
endforeach(CAMERA_TYPE)
