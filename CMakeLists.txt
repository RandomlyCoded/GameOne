cmake_minimum_required(VERSION 3.14)
project(GameOne LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.0 QUIET COMPONENTS Quick Svg)

if (Qt6_FOUND)
    add_library(Qt::Quick ALIAS Qt6::Quick)
    add_library(Qt::SVG ALIAS Qt6::Svg)
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS Quick Svg)
    add_library(Qt::Quick ALIAS Qt5::Quick)
    add_library(Qt::Svg ALIAS Qt5::Svg)
endif()

add_definitions(-DQT_RESTRICTED_CAST_FROM_ASCII=1)

add_library(GameOneCore STATIC)
target_include_directories(GameOneCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(GameOneCore PUBLIC Qt::Quick Qt::Svg)

target_sources(
    GameOneCore PRIVATE
    actors.cpp actors.h
    application.cpp application.h
    assets.qrc
    backend.cpp backend.h
    data.qrc
    imageprovider.cpp imageprovider.h
    inventorymodel.cpp inventorymodel.h
    levelmodel.cpp levelmodel.h
    mapmodel.cpp mapmodel.h
    qml.qrc
    )

add_executable(GameOne WIN32 main.cpp)
target_link_libraries(GameOne PRIVATE GameOneCore)

add_subdirectory(tests)

add_custom_target(
    Documentation SOURCES
    TODO.txt
    )
