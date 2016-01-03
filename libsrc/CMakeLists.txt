add_subdirectory(hyperion)
add_subdirectory(blackborder)

if (ENABLE_SERVER_JSON)
    add_subdirectory(jsonserver)
endif ()

if (ENABLE_SERVER_BOBLIGHT)
    add_subdirectory(boblightserver)
endif ()

add_subdirectory(leddevice)
add_subdirectory(utils)

if (ENABLE_KODI_CONNECTION)
    add_subdirectory(kodiconnector)
endif ()

if (ENABLE_EFFECT_ENGINE)
    add_subdirectory(effectengine)
endif ()

if (ENABLE_GRABBER)
    add_subdirectory(grabber)
endif ()
