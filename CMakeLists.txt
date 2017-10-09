project(plasma-newsfeeds)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(QT_MIN_VERSION "5.6.1")
set(KF5_MIN_VERSION "5.26.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Widgets Quick QuickWidgets Concurrent Test Script Network)
find_package(ECM 1.8.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
                    Plasma DocTools Runner JsEmbed NotifyConfig Su NewStuff Wallet KCMUtils
                    IdleTime Declarative TextWidgets KDELibs4Support Crash GlobalAccel
                    DBusAddons Wayland)
find_package(KF5Syndication)
find_package(KF5NetworkManagerQt ${KF5_MIN_VERSION})
set_package_properties(KF5NetworkManagerQt PROPERTIES DESCRIPTION "Qt wrapper for NetworkManager API"
                       TYPE OPTIONAL
                       PURPOSE "Needed by geolocation data engine."
                      )

# WARNING PlasmaQuick provides unversioned CMake config
find_package(KF5 REQUIRED COMPONENTS PlasmaQuick)
find_package(KF5 REQUIRED COMPONENTS SysGuard)
find_package(KF5 REQUIRED COMPONENTS Package)
# Baloo has a different version scheme than KF5 for now
find_package(KF5 5.1 REQUIRED COMPONENTS Baloo)
find_package(KF5TextEditor)
find_package(KWinDBusInterface CONFIG REQUIRED)

find_package(KScreenLocker REQUIRED)
find_package(ScreenSaverDBusInterface CONFIG REQUIRED)
find_package(KF5Holidays)
set_package_properties(KF5Holidays PROPERTIES DESCRIPTION "Holidays provider for Plasma calendar"
                       TYPE OPTIONAL
                       PURPOSE "Needed to for holidays plugin for Plasma Calendar."
                      )

find_package(Phonon4Qt5 4.6.60 REQUIRED NO_MODULE)
set_package_properties(Phonon4Qt5 PROPERTIES
   DESCRIPTION "Qt-based audio library"
   TYPE REQUIRED)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMPackageConfigHelpers)
include(ECMMarkNonGuiExecutable)
include(CMakePackageConfigHelpers)
include(WriteBasicConfigVersionFile)
include(CheckIncludeFiles)
include(FeatureSummary)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(KDEPackageAppTemplates)

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_engine_newsfeeds\")

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
)

set(newsfeeds_engine_SRCS
    newsfeedsengine.cpp
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(plasma_engine_newsfeeds MODULE ${newsfeeds_engine_SRCS})
kcoreaddons_desktop_to_json(plasma_engine_newsfeeds plasma-dataengine-newsfeeds.desktop)

target_link_libraries(plasma_engine_newsfeeds
    KF5::Plasma
    KF5::KIOGui
    KF5::I18n
    KF5::Service
    KF5::Syndication
    Qt5::Network
)

install(TARGETS plasma_engine_newsfeeds DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-newsfeeds.desktop DESTINATION ${SERVICES_INSTALL_DIR})
