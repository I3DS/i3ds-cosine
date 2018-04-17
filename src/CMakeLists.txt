set (SRCS
  )

set (LIBS
  i3ds
  zmq
  )

set (APPS
  cosine-tir
  )

foreach(a ${APPS})
  add_executable (${a} ${a}.cpp ${SRCS})
  target_link_libraries (${a} ${LIBS})
  install(TARGETS ${a} DESTINATION bin)
endforeach(a)
