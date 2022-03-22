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

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library")

set(CMAKE_BUILD_PARALLEL_LEVEL ON)
if(MSVC)
    add_definitions(/MP)
endif()

set(PRINT_CMAKE_VARIABLES OFF CACHE BOOL "Print CMake variables")

set(QT_VERSION "6.2.3" CACHE STRING "Currently installed Qt version")
if(WIN32)
    set(QT_DIR "C:/Qt/${QT_VERSION}/msvc2019_64/lib/cmake/Qt6" CACHE PATH "Qt directory")

    set(WITH_CONTROLLER OFF CACHE BOOL "Include controller functions")
    set(WITH_VR OFF CACHE BOOL "Include VR functions")

    set(BUILD_EXAMPLE OFF CACHE BOOL "Build example program")
elseif(UNIX)
    set(QT_DIR "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
    set(Qt6DBusTools_DIR "${QT_DIR}DBusTools")

    set(WITH_CONTROLLER ON CACHE BOOL "Include controller functions")
    set(WITH_VR ON CACHE BOOL "Include VR functions")
endif()

set(Qt6_DIR "${QT_DIR}")
set(Qt6CoreTools_DIR "${QT_DIR}CoreTools")
set(Qt6GuiTools_DIR "${QT_DIR}GuiTools")
set(Qt6WidgetsTools_DIR "${QT_DIR}WidgetsTools")

find_package(QT NAMES Qt6 COMPONENTS Widgets OpenGL REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets OpenGL REQUIRED)

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
endif()

# include_directories(
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
#     ${CMAKE_CURRENT_SOURCE_DIR}/src
# )

set(HEADERS
    include/Camera.h
    include/CameraController.h
    include/Model3D.h
    include/Octree.h
    include/ModelPTS.h
    include/ModelBIN.h
    include/Object3D.h
    include/Object3DQt.h
    include/Engine3D_global.h
    include/Engine3D.h
)

set(SOURCES
    src/Camera.cpp
    src/CameraController.cpp
    src/Model3D.cpp
    src/ModelPTS.cpp
    src/ModelBIN.cpp
    src/Octree.cpp
    src/Object3D.cpp
    src/Object3DQt.cpp
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

if(${QT_VERSION_MAJOR} EQUAL 5)
    qt5_add_resources(${QRC} ${RESOURCES})
elseif(${QT_VERSION_MAJOR} EQUAL 6)
    qt6_add_resources(${QRC} ${RESOURCES})
endif()

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

    ${OPENVR_LIBRARIES}
)

if(WIN32)
    if(WITH_CONTROLLER)
        target_link_libraries(Engine3D PRIVATE XInput)
    endif()
endif()

target_compile_definitions(Engine3D PRIVATE ENGINE3D_LIBRARY)

install(TARGETS Engine3D)
if(WIN32)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        FILES_MATCHING PATTERN "*.h*"
    )
elseif(UNIX)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
        DESTINATION ${CMAKE_INSTALL_PREFIX}/Engine3D
        FILES_MATCHING PATTERN "include/*.h*"
    )
endif()

set(Engine3D_FOUND ON)
set(Engine3D_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(Engine3D_LIBRARIES_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
set(Engine3D_LIBRARIES Engine3D$<$<CONFIG:Debug>:d>)

if(BUILD_EXAMPLE)
    add_subdirectory(Test)
    add_custom_command(TARGET Engine3D POST_BUILD
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