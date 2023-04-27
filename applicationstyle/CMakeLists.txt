app_add_plugin(applicationstyle
    PLUGIN_TYPE styles
    CLASS_NAME ApplicationStylePlugin
    applicationstyle.json
    applicationstyle.h
    applicationstyle.cpp
    applicationstyleplugin.h
    applicationstyleplugin.cpp
)

file(GLOB_RECURSE IMAGES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS images/*
)

qt_add_resources(applicationstyle applicationstyleresources
    PREFIX /
    FILES ${IMAGES}
)

target_precompile_headers(applicationstyle REUSE_FROM pch)

target_link_libraries(applicationstyle
    PRIVATE Qt::WidgetsPrivate # ApplicationStyle
)
