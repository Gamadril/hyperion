add_library(hyperion-utils
	ColorArgb.cpp
	ColorRgb.cpp
	ColorRgba.cpp
	ImageResampler.cpp
	HsvTransform.cpp
	RgbChannelTransform.cpp
	JsonTools.cpp
)

target_link_libraries(hyperion-utils
    PocoFoundation
    PocoJSON
)

