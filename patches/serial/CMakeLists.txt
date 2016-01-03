cmake_minimum_required(VERSION 2.8)
project(serial)

if(APPLE)
	find_library(IOKIT_LIBRARY IOKit)
	find_library(FOUNDATION_LIBRARY Foundation)
endif()

## Sources
set(serial_SRCS
    src/serial.cc
    include/serial/serial.h
    include/serial/v8stdint.h
)

if(APPLE)
	# If OSX
	list(APPEND serial_SRCS src/impl/unix.cc)
elseif(UNIX)
    # If unix
    list(APPEND serial_SRCS src/impl/unix.cc)
else()
    # If windows
    list(APPEND serial_SRCS src/impl/win.cc)
endif()

## Add serial library
add_library(${PROJECT_NAME} ${serial_SRCS})
if(APPLE)
	target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
	target_link_libraries(${PROJECT_NAME} rt pthread)
else()
	target_link_libraries(${PROJECT_NAME} setupapi)
endif()

## Include headers
include_directories(include)

## Install executable
install(TARGETS ${PROJECT_NAME}
	DESTINATION lib
)

## Install headers
install(FILES include/serial/serial.h include/serial/v8stdint.h
  DESTINATION serial
)
