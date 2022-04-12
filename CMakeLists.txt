cmake_minimum_required(VERSION 3.16)

project(Engine3D LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install CACHE PATH "Install directory" FORCE)
    endif()
endif()
set(CMAKE_DEBUG_POSTFIX d)

if(WIN32)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library")
elseif(UNIX)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared library")
endif()

set(CMAKE_BUILD_PARALLEL_LEVEL ON)
if(MSVC)
    add_definitions(/MP)
endif()

set(PRINT_CMAKE_VARIABLES OFF CACHE BOOL "Print CMake variables")

set(QT_VERSION "6.3.0" CACHE STRING "Currently installed Qt version")
if(WIN32)
    set(QT_DIR "C:/Qt/${QT_VERSION}/msvc2019_64/lib/cmake/Qt6" CACHE PATH "Qt directory")

    set(WITH_CONTROLLER ON CACHE BOOL "Include controller functions")
    set(WITH_VR OFF CACHE BOOL "Include VR functions")
elseif(UNIX)
    if(IS_DIRECTORY "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6")
        set(QT_DIR "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
    elseif(IS_DIRECTORY "/usr/local/Qt-${QT_VERSION}/lib/cmake/Qt6")
        set(QT_DIR "/usr/local/Qt-${QT_VERSION}/lib/cmake/Qt6" CACHE PATH "Qt directory")
    else()
        set(QT_DIR "QT_DIR-NOTFOUND" CACHE PATH "Qt directory")
    endif()
    set(Qt6DBusTools_DIR "${QT_DIR}DBusTools")
    set(Qt6BundledPcre2_DIR "${QT_DIR}BundledPcre2")
    set(Qt6BundledHarfbuzz_DIR "${QT_DIR}BundledHarfbuzz")
    set(Qt6BundledLibjpeg_DIR "${QT_DIR}BundledLibjpeg")

    set(WITH_CONTROLLER ON CACHE BOOL "Include controller functions")
    set(WITH_VR OFF CACHE BOOL "Include VR functions")
endif()

set(Qt6_DIR "${QT_DIR}")
set(Qt6CoreTools_DIR "${QT_DIR}CoreTools")
set(Qt6GuiTools_DIR "${QT_DIR}GuiTools")
set(Qt6WidgetsTools_DIR "${QT_DIR}WidgetsTools")

set(BUILD_EXAMPLE OFF CACHE BOOL "Build example program")

find_package(QT NAMES Qt6 COMPONENTS Widgets OpenGL Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGL Concurrent REQUIRED)

if(WIN32)
    find_package(GLM REQUIRED)
    if(GLM_FOUND)
            set(GLM_INCLUDE_DIRS ${GLM_DIR}/../../../include)
    endif(GLM_FOUND)
elseif(UNIX)
    find_package(glm REQUIRED)
endif()

if(WITH_VR)
    if(WIN32)
        set(OPENVR_DIR "OPENVR_DIR-NOTFOUND" CACHE PATH "Install directory of OpenVR")
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
endif()

if(WITH_CONTROLLER)
    list(APPEND EXTRA_SOURCES
        include/Controller.h
        src/Controller.cpp
    )
    add_compile_definitions(WITH_CONTROLLER)
endif()

# include_directories(
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
# )

set(HEADERS
    include/Object3D.h
    include/Object3DQt.h
    include/Camera.h
    include/CameraController.h
    include/Model3D.h
    include/ModelPTS.h
    include/ModelBIN.h
    include/Octree.h
    include/Model3DWriter.h
    include/Engine3D_global.h
    include/Engine3D.h
)

set(SOURCES
    src/Object3D.cpp
    src/Object3DQt.cpp
    src/Camera.cpp
    src/CameraController.cpp
    src/Model3D.cpp
    src/ModelPTS.cpp
    src/ModelBIN.cpp
    src/Octree.cpp
    src/Model3DWriter.cpp
    src/Engine3D.cpp
)

set(RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Resources.qrc
)

set(SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Point.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Point.frag

    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Box.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Box.frag
)

qt6_add_resources(${QRC} ${RESOURCES})

if(BUILD_SHARED_LIBS)
    add_library(Engine3D SHARED
        ${HEADERS}
        ${SOURCES}

        ${RESOURCES}
        ${QRC}

        ${SHADERS}

        ${EXTRA_SOURCES}
    )
else()
    add_library(Engine3D STATIC
        ${HEADERS}
        ${SOURCES}

        ${RESOURCES}
        ${QRC}

        ${SHADERS}

        ${EXTRA_SOURCES}
    )
endif()

target_include_directories(Engine3D PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${OPENVR_INCLUDE_DIRS}
    include
    src
)

target_link_libraries(Engine3D PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Concurrent

    ${OPENVR_LIBRARIES}
)

if(WIN32)
    if(WITH_CONTROLLER)
        target_link_libraries(Engine3D PRIVATE XInput)
    endif()
endif()

target_compile_definitions(Engine3D PRIVATE ENGINE3D_LIBRARY)

if(WIN32)
    install(TARGETS Engine3D)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        FILES_MATCHING PATTERN "*.h*"
    )
elseif(UNIX)
    install(TARGETS Engine3D
        DESTINATION Engine3D/lib
    )
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
        DESTINATION Engine3D
        FILES_MATCHING PATTERN "*.h*"
    )
endif()

set(Engine3D_FOUND ON)
set(Engine3D_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(Engine3D_LIBRARIES_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
set(Engine3D_LIBRARIES Engine3D$<$<CONFIG:Debug>:d>)

if(BUILD_EXAMPLE)
    add_subdirectory(Test)
    if(BUILD_SHARED_LIBS)
        if(IS_DIRECTORY $<TARGET_FILE_DIR:Example>)
            if(WIN32)
                add_custom_command(TARGET Engine3D POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${Engine3D_LIBRARIES_DIR}/${Engine3D_LIBRARIES}.dll"
                    $<TARGET_FILE_DIR:Example>
                )
            elseif(UNIX)
                add_custom_command(TARGET Engine3D POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${Engine3D_LIBRARIES_DIR}/${Engine3D_LIBRARIES}.so"
                    $<TARGET_FILE_DIR:Example>
                )
            endif()
        endif()
    endif()
endif()

if(PRINT_CMAKE_VARIABLES)
        get_cmake_property(_variableNames VARIABLES)
        list (SORT _variableNames)
        foreach (_variableName ${_variableNames})
                message(STATUS "${_variableName}=${${_variableName}}")
        endforeach()
endif()