cmake_minimum_required(VERSION 3.24)

project(NeuralNetworkArtImages VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ARTNET_BUILD_APP "Build the Qt desktop application" ON)
option(ARTNET_BUILD_TESTS "Build unit tests" ON)
option(ARTNET_USE_MLPACK "Use mlpack for clustering when available" OFF)
option(ARTNET_REQUIRE_MLPACK "Fail configuration if ARTNET_USE_MLPACK is ON and mlpack is not found" OFF)

add_library(artnet_core
    src/ArtScanner.cpp
    src/Clusterer.cpp
    src/EmbeddingIndex.cpp
    src/Export.cpp
)

target_include_directories(artnet_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(ARTNET_USE_MLPACK)
    find_package(mlpack CONFIG QUIET)
    if(mlpack_FOUND)
        target_compile_definitions(artnet_core PUBLIC ARTNET_USE_MLPACK=1)
        target_link_libraries(artnet_core PUBLIC mlpack::mlpack)
    elseif(ARTNET_REQUIRE_MLPACK)
        message(FATAL_ERROR "ARTNET_USE_MLPACK is ON, but CMake could not find mlpack. Install mlpack with vcpkg or set -DARTNET_USE_MLPACK=OFF.")
    else()
        message(WARNING "mlpack was requested but not found. Falling back to built-in k-means clustering.")
    endif()
endif()

if(ARTNET_BUILD_APP)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc dnn)

    qt_standard_project_setup()

    qt_add_executable(artnet_desktop
        include/artnet/MainWindow.h
        src/main.cpp
        src/MainWindow.cpp
        src/OpenCvDnnModel.cpp
    )

    target_include_directories(artnet_desktop PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(artnet_desktop
        PRIVATE
            artnet_core
            Qt6::Widgets
            ${OpenCV_LIBS}
    )

    if(WIN32 AND TARGET Qt6::windeployqt)
        add_custom_command(TARGET artnet_desktop POST_BUILD
            COMMAND Qt6::windeployqt
                    --no-translations
                    --no-compiler-runtime
                    --dir "$<TARGET_FILE_DIR:artnet_desktop>"
                    "$<TARGET_FILE:artnet_desktop>"
            COMMENT "Deploying Qt runtime files"
        )
    elseif(WIN32)
        get_filename_component(ARTNET_QT_PREFIX "${Qt6_DIR}/../.." ABSOLUTE)
        set(ARTNET_QWINDOWS_PLUGIN "${ARTNET_QT_PREFIX}/Qt6/plugins/platforms/qwindows.dll")
        if(EXISTS "${ARTNET_QWINDOWS_PLUGIN}")
            add_custom_command(TARGET artnet_desktop POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:artnet_desktop>/platforms"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${ARTNET_QWINDOWS_PLUGIN}"
                        "$<TARGET_FILE_DIR:artnet_desktop>/platforms/qwindows.dll"
                COMMENT "Copying Qt Windows platform plugin"
            )
        endif()
    endif()
endif()

if(ARTNET_BUILD_TESTS)
    enable_testing()

    add_executable(artnet_core_tests
        tests/CoreTests.cpp
    )

    target_link_libraries(artnet_core_tests PRIVATE artnet_core)
    add_test(NAME artnet_core_tests COMMAND artnet_core_tests)
endif()
