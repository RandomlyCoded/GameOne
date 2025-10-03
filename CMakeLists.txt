cmake_minimum_required(VERSION 3.28)
project(GameOne LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Svg)
add_definitions(-DQT_RESTRICTED_CAST_FROM_ASCII=1)

option(DEVELOPER_BUILD "Enable stricter build options to aid active contributors" OFF)

if (DEVELOPER_BUILD)
    add_compile_options(-Wall -Wextra -Werror)
    find_program(CLANG_TIDY_EXECUTABLE clang-tidy)
    find_program(CLTCACHE_EXECUTABLE cltcache PATHS $ENV{HOME}/.local/python/bin)
endif()

if (CLANG_TIDY_EXECUTABLE)
    string(
        JOIN "," CLANG_TIDY_CHECKS

        bugprone-*
        modernize-*
        performance-*
        readability-*

        llvm-header-guard
        llvm-include-order
        llvm-namespace-comment

        -bugprone-suspicious-include                    # it complains about "moc_filename.cpp"
        -clang-analyzer-cplusplus.NewDelete             # well intended check, that's unfortunatly very broken and slow
        -modernize-use-trailing-return-type             # nope
    )

    message(STATUS "Using ${CLANG_TIDY_EXECUTABLE}")

    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE}
        --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        --checks=${CLANG_TIDY_CHECKS})

    if (CLTCACHE_EXECUTABLE)
        message(STATUS "Using ${CLTCACHE_EXECUTABLE}")
        list(PREPEND CMAKE_CXX_CLANG_TIDY ${CLTCACHE_EXECUTABLE})
    endif()
else()
    message(STATUS "Not using clang-tidy code checker")
endif()

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
    CMakeFiles SOURCES
    .clang-tidy
)

add_custom_target(
    Documentation SOURCES
    # README.md
    Errors.txt
)
