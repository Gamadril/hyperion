add_library(hyperion
	Hyperion.cpp
	ImageProcessor.cpp
	ImageProcessorFactory.cpp
	LedString.cpp
	PriorityMuxer.cpp
	ImageToLedsMap.cpp
	MultiColorTransform.cpp
	LinearColorSmoothing.cpp
)

target_link_libraries(hyperion
	blackborder
	hyperion-utils
	leddevice
)

if(ENABLE_EFFECT_ENGINE)
	target_link_libraries(hyperion effectengine)
endif()

if(UNIX AND NOT APPLE)
	target_link_libraries(hyperion pthread)
endif()
