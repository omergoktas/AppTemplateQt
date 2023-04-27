cmake_minimum_required(VERSION 3.24)

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

project(AppTemplateQt
    LANGUAGES CXX
    VERSION 1.0.0
    HOMEPAGE_URL omergoktas.com
    DESCRIPTION "Project template for Qt-based cross-platform apps."
)

include(Options)
include(Setup)
include(Deploy)

find_package(Qt${QT_VERSION_MAJOR}
    REQUIRED
        Widgets
        Svg
        Network
    OPTIONAL_COMPONENTS
        LinguistTools
)

qt_add_executable(AppTemplateQt
    main.cpp
    application.h
    application.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui.h
    mainwindow.ui.cpp
)

file(GLOB_RECURSE IMAGES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS resources/images/*
)

file(GLOB FONTS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS resources/fonts/*.ttf
)

file(GLOB TRANSLATIONS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS resources/translations/*.ts
)

qt_add_resources(AppTemplateQt
    resources
    PREFIX /
    BASE resources
    FILES ${FONTS} ${IMAGES}
)

app_add_translations(AppTemplateQt TS_FILES ${TRANSLATIONS})

add_subdirectory(pch)
add_subdirectory(utils)
add_subdirectory(applicationstyle)

target_precompile_headers(AppTemplateQt REUSE_FROM pch)

target_link_libraries(AppTemplateQt PRIVATE
    Qt::WidgetsPrivate # Application::setDprAdjustmentHook
    Qt::Network
    utils
)

if(NOT BUILD_SHARED_LIBS)
    target_link_libraries(AppTemplateQt PRIVATE applicationstyle)
endif()

deploy(AppTemplateQt deploy)
