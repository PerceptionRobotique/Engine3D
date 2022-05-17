cmake_minimum_required(VERSION 3.16)

project(Test VERSION 1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_PARALLEL_LEVEL ON)
if(MSVC)
    add_definitions(/MP /bigobj)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT Test)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(PRINT_CMAKE_VARIABLES OFF CACHE BOOL "Print CMake variables")

### QT ###
list(APPEND QT_COMPONENTS OpenGLWidgets)
foreach (qt_component ${QT_COMPONENTS})
    list(APPEND QT_LINKS
        Qt${QT_VERSION_MAJOR}::${qt_component}
    )
endforeach()

find_package(QT NAMES Qt6 COMPONENTS ${QT_COMPONENTS} REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_COMPONENTS} REQUIRED)

if(NOT Engine3D_FOUND)
    set(Engine3D_DIR "Engine3D-NOTFOUND" CACHE PATH "Engine3D directory")
    message(${Engine3D_DIR})
    if(IS_DIRECTORY ${Engine3D_DIR})
        set(Engine3D_INCLUDE_DIRS ${Engine3D_DIR}/include)
        set(Engine3D_LIBRARIES_DIR ${Engine3D_DIR}/lib)
        set(Engine3D_LIBRARIES Engine3D$<$<CONFIG:Debug>:d>)
    else()
        message(FATAL_ERROR "Engine3D is missing.")
    endif()
endif()

### VR ###
if(WITH_VR)
    if(WIN32)
        if(IS_DIRECTORY ${OPENVR_DIR})
            list(APPEND OPENVR_INCLUDE_DIRS ${OPENVR_DIR}/include)
            set(OPENVR_LIBRARIES ${OPENVR_DIR}/lib/openvr_api64$<$<CONFIG:Debug>:d>.lib)
        else()
            message(FATAL_ERROR "OpenVR is missing.")
        endif()
    elseif(UNIX)
        set(OPENVR_LIBRARIES /usr/lib/x86_64-linux-gnu/libopenvr_api.so)
    endif()

    list(APPEND EXTRA_SOURCES
        include/VRheadset.h
        src/VRheadset.cpp
    )

    add_compile_definitions(HAVE_VR)
endif()

set(SOURCES
    main.cpp
    MainWindow.cpp
    OpenGLWidget.cpp
    GraphicsView.cpp
)

set(HEADERS
    MainWindow.h
    OpenGLWidget.h
    GraphicsView.h
)

set(RESOURCES
    Resources.qrc
)

set(UI
    MainWindow.ui
)

set(PROJECT_SOURCES
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${UI}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Test
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Test APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Test SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Test
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(Test
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${VISP_INCLUDE_DIRS}
    ${OPENVR_INCLUDE_DIRS}
    ${Engine3D_INCLUDE_DIRS}
)

target_link_directories(Test PRIVATE ${Engine3D_LIBRARIES_DIR})

target_link_libraries(Test
    PRIVATE
    ${QT_LINKS}
    ${VISP_LIBRARIES}
    ${OPENVR_LIBRARIES}
    ${Engine3D_LIBRARIES}
)

### TERMINAL OUTPUT ###
if(WIN32)
    set(USE_TERMINAL_OUTPUT OFF CACHE BOOL "Launch with a terminal")
    if(NOT ${USE_TERMINAL_OUTPUT})
        set_target_properties(Test PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Test)
endif()

if(WIN32)
    add_custom_command(TARGET Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Test>/platforms"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Test>/styles"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Test>/imageformats"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qminimal$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qoffscreen$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/styles/qwindowsvistastyle$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>/styles

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/imageformats/qjpeg$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>/imageformats

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Core$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Gui$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Widgets$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}OpenGL$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}OpenGLWidgets$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Concurrent$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Test>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Engine3D_LIBRARIES_DIR}/${Engine3D_LIBRARIES}.dll"
        $<TARGET_FILE_DIR:Test>
    )

    if(WITH_OPENCV)
        foreach (OpenCV_COMPONENT ${OpenCV_COMPONENTS})
            add_custom_command(TARGET Test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OpenCV_DIR}/../bin/opencv_${OpenCV_COMPONENT}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}.dll"
                $<TARGET_FILE_DIR:Test>
            )
        endforeach()

        foreach (OpenCV_EXTRA ${OpenCV_EXTRA_DLL})
            add_custom_command(TARGET Test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${OpenCV_DIR}/../bin/${OpenCV_EXTRA}.dll"
                $<TARGET_FILE_DIR:Test>
            )
        endforeach()
    endif()

    if(WITH_VISP)
        foreach (visp_component ${VISP_COMPONENTS})
            add_custom_command(TARGET Test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${VISP_DIR}/${VISP_ARCH}/${VISP_RUNTIME}/bin/visp_${visp_component}${VISP_VERSION_MAJOR}${VISP_VERSION_MINOR}${VISP_VERSION_PATCH}.dll"
                $<TARGET_FILE_DIR:Test>
            )
        endforeach()
    endif()
endif()

if(PRINT_CMAKE_VARIABLES)
        get_cmake_property(_variableNames VARIABLES)
        list (SORT _variableNames)
        foreach (_variableName ${_variableNames})
                message(STATUS "${_variableName}=${${_variableName}}")
        endforeach()
endif()
