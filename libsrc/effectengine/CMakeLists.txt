add_library(effectengine
	EffectEngine.cpp
	Effect.cpp
)

target_link_libraries(effectengine
	hyperion
	lua
)
