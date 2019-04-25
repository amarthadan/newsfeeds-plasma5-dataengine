project(plasma-newsfeeds)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(QT_MIN_VERSION "5.5.0")
set(KF5_MIN_VERSION "5.21.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets Quick QuickWidgets Network)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
Plasma PlasmaQuick I18n GlobalAccel Syndication)

include(KDECMakeSettings)
include(KDECompilerSettings)
include(KDEInstallDirs)
include(ECMPackageConfigHelpers)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_engine_newsfeeds\")

set(newsfeeds_engine_SRCS
    fileretriever.cpp
    faviconstorage.cpp
    faviconrequestjob.cpp
    newsfeedsengine.cpp
)

add_library(plasma_engine_newsfeeds MODULE ${newsfeeds_engine_SRCS})
kcoreaddons_desktop_to_json(plasma_engine_newsfeeds plasma-dataengine-newsfeeds.desktop)

target_link_libraries(plasma_engine_newsfeeds
    KF5::Plasma
    KF5::I18n
    KF5::Service
    KF5::GlobalAccel
    KF5::Syndication
    Qt5::Network
)

install(TARGETS plasma_engine_newsfeeds DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES plasma-dataengine-newsfeeds.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
