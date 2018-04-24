find_package (Boost COMPONENTS program_options log REQUIRED)

set (SRCS
   emulated_camera.cpp
   periodic_sampler.cpp
  )

set (LIBS
  i3ds-core
  i3ds-sensors
  zmq
  )

set (APPS
  cosine-tir
  i3ds_suite_emulator
  )


include_directories ("include/")
include_directories ("${CMAKE_CURRENT_BINARY_DIR}/src/core")


foreach(a ${APPS})
  add_executable (${a} ${a}.cpp ${SRCS})
  target_link_libraries (${a} ${LIBS} ${Boost_LIBRARIES})
  install(TARGETS ${a} DESTINATION bin)
endforeach(a)
