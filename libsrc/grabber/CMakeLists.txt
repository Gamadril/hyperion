if(APPLE AND ENABLE_GRABBER_OSX)
	find_library(CG_LIBRARY CoreGraphics)
	find_library(FOUNDATION_LIBRARY Foundation)
endif()

if(ENABLE_GRABBER_X11)
	find_package(X11 REQUIRED)
	find_library(X11 X11)
	find_package(Threads REQUIRED)
endif()

SET(FrameGrabber_SOURCES
	FrameGrabberFactory.cpp
	FrameGrabber.cpp
)

if(ENABLE_GRABBER_DISPMANX)
	list(APPEND FrameGrabber_SOURCES DispmanxFrameGrabber.cpp)
endif()

if(ENABLE_GRABBER_FB)
	list(APPEND FrameGrabber_SOURCES FramebufferFrameGrabber.cpp)
endif()

if(ENABLE_GRABBER_V4L2)
	list(APPEND FrameGrabber_SOURCES V42LFrameGrabber.cpp)
endif()

if(ENABLE_GRABBER_X11)
	list(APPEND FrameGrabber_SOURCES X11FrameGrabber.cpp)
endif()

if(ENABLE_GRABBER_OSX)
	list(APPEND FrameGrabber_SOURCES OSXFrameGrabber.cpp)
endif()

add_library(framegrabber
	${FrameGrabber_SOURCES}
)

target_link_libraries(framegrabber
	hyperion-utils
	hyperion
)

if(ENABLE_GRABBER_DISPMANX)
	target_link_libraries(framegrabber bcm_host)
endif()

if(ENABLE_GRABBER_X11)
	target_link_libraries(framegrabber ${X11})
	#target_link_libraries(framegrabber pthread)
endif()

if(APPLE AND ENABLE_GRABBER_OSX)
	target_link_libraries(framegrabber ${FOUNDATION_LIBRARY})
	target_link_libraries(framegrabber ${CG_LIBRARY})
endif()
