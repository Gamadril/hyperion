add_library(blackborder
	BlackBorderDetector.cpp
	BlackBorderProcessor.cpp
)

target_link_libraries(blackborder
		hyperion-utils
)
