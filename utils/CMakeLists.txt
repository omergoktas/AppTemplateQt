app_add_library(utils
    utils.h
    utils.cpp
    signalhandler.h
    signalhandler.cpp
)

generate_export_header(utils)

target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} # utils_export.h
)

target_link_libraries(utils PUBLIC
    Qt::Network # SignalHandler
    Qt::Widgets # Utils
)
