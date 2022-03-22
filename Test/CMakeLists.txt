cmake_minimum_required(VERSION 3.16)

project(Example VERSION 1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_PARALLEL_LEVEL ON)
if(MSVC)
    add_definitions(/MP /bigobj)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT Example)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PRINT_CMAKE_VARIABLES OFF CACHE BOOL "Print CMake variables")

if(FRAME_COUNTER)
    add_compile_definitions(FRAME_COUNTER)
endif()

set(QT_VERSION "6.2.3" CACHE STRING "Currently installed Qt version")
if(WIN32)
    set(QT_DIR "C:/Qt/${QT_VERSION}/msvc2019_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
elseif(UNIX)
    set(QT_DIR "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
    set(Qt6DBusTools_DIR "${QT_DIR}DBusTools")
endif()

set(Qt6_DIR "${QT_DIR}")
set(Qt6CoreTools_DIR "${QT_DIR}CoreTools")
set(Qt6GuiTools_DIR "${QT_DIR}GuiTools")
set(Qt6WidgetsTools_DIR "${QT_DIR}WidgetsTools")

# set(WITH_OPENCV OFF CACHE BOOL "Include OpenCV functions")
# set(WITH_VISP OFF CACHE BOOL "Include visp functions")
# set(WITH_PER OFF CACHE BOOL "Include PER functions")
# set(WITH_CONTROLLER OFF CACHE BOOL "Include controller functions")
# set(WITH_VR OFF CACHE BOOL "Include VR functions")

find_package(QT NAMES Qt6 COMPONENTS Widgets OpenGL OpenGLWidgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGL OpenGLWidgets REQUIRED)

if(WIN32)
    find_package(GLM REQUIRED)
    if(GLM_FOUND)
        set(GLM_INCLUDE_DIRS ${GLM_DIR}/../../../include)
    endif()
elseif(UNIX)
    find_package(glm REQUIRED)
endif()

if(NOT Engine3D_FOUND)
    set(Engine3D_DIR "Engine3D-NOTFOUND" CACHE PATH "Engine3D directory")
    if(IS_DIRECTORY ${Engine3D_DIR})
        set(Engine3D_INCLUDE_DIRS ${Engine3D_DIR}/include)
        set(Engine3D_LIBRARIES_DIR ${Engine3D_DIR}/lib)
        set(Engine3D_LIBRARIES Engine3D$<$<CONFIG:Debug>:d>)
    else()
        message(FATAL_ERROR "Engine3D is missing.")
    endif()
endif()

set(SOURCES
    main.cpp
    MainWindow.cpp
    OpenGLWidget.cpp
)

set(HEADERS
    MainWindow.h
    OpenGLWidget.h
)

set(RESOURCES
    Resources.qrc
)

set(UI
    MainWindow.ui
)

if(${QT_VERSION_MAJOR} EQUAL 5)
    qt5_add_resources(${QRC} ${RESOURCES})
elseif(${QT_VERSION_MAJOR} EQUAL 6)
    qt6_add_resources(${QRC} ${RESOURCES})
endif()

set(PROJECT_SOURCES
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${QRC}
    ${UI}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Example
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Example APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Example SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Example
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(Example
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${Engine3D_INCLUDE_DIRS}
)

target_link_directories(Example PRIVATE ${Engine3D_LIBRARIES_DIR})

target_link_libraries(Example
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets

    ${Engine3D_LIBRARIES}
)

set_target_properties(Example PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Example)
endif()

if(WIN32)
    add_custom_command(TARGET Example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Example>/platforms"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Example>/styles"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qminimal$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qoffscreen$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/styles/qwindowsvistastyle$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>/styles

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Core$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Gui$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Widgets$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}OpenGL$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}OpenGLWidgets$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:Example>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Engine3D_LIBRARIES_DIR}/${Engine3D_LIBRARIES}.dll"
        $<TARGET_FILE_DIR:Example>
    )
endif()

if(PRINT_CMAKE_VARIABLES)
        get_cmake_property(_variableNames VARIABLES)
        list (SORT _variableNames)
        foreach (_variableName ${_variableNames})
                message(STATUS "${_variableName}=${${_variableName}}")
        endforeach()
endif()
