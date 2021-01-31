cmake_minimum_required(VERSION 3.14)
project(GameOne LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.0 COMPONENTS Quick)

if (Qt6_FOUND)
    add_library(Qt::Quick ALIAS Qt6::Quick)
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS Quick)
    add_library(Qt::Quick ALIAS Qt5::Quick)
endif()

add_definitions(-DQT_RESTRICTED_CAST_FROM_ASCII=1)

add_executable(GameOne WIN32)
target_link_libraries(GameOne PRIVATE Qt::Quick)

target_sources(
    GameOne PRIVATE
    actors.cpp actors.h
    assets.qrc
    backend.cpp backend.h
    levelmodel.cpp levelmodel.h
    main.cpp main.qrc
    mapmodel.cpp mapmodel.h
    )

add_custom_target(
    Documentation SOURCES
    TODO.txt
    )
