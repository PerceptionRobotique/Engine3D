cmake_minimum_required(VERSION 3.16)

project(Engine3D VERSION 2.3.1 LANGUAGES CXX)

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

### QT ###
set(QT_VERSION "6.3.0" CACHE STRING "Currently installed Qt version")
if(WIN32)
    set(QT_DIR "C:/Qt/${QT_VERSION}/msvc2019_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
elseif(UNIX)
    if(IS_DIRECTORY "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6")
        set(QT_DIR "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
    elseif(IS_DIRECTORY "/usr/local/Qt-${QT_VERSION}/lib/cmake/Qt6")
        set(QT_DIR "/usr/local/Qt-${QT_VERSION}/lib/cmake/Qt6" CACHE PATH "Qt directory")
    else()
        set(QT_DIR "QT_DIR-NOTFOUND" CACHE PATH "Qt directory")
    endif()

    if(IS_DIRECTORY ${QT_DIR})
        set(Qt6DBusTools_DIR "${QT_DIR}DBusTools")
        set(Qt6BundledPcre2_DIR "${QT_DIR}BundledPcre2")
        set(Qt6BundledHarfbuzz_DIR "${QT_DIR}BundledHarfbuzz")
        set(Qt6BundledLibjpeg_DIR "${QT_DIR}BundledLibjpeg")
    endif()
endif()

if(IS_DIRECTORY ${QT_DIR})
    set(Qt6_DIR "${QT_DIR}")
    set(Qt6CoreTools_DIR "${QT_DIR}CoreTools")
    set(Qt6GuiTools_DIR "${QT_DIR}GuiTools")
    set(Qt6WidgetsTools_DIR "${QT_DIR}WidgetsTools")
endif()

list(APPEND QT_COMPONENTS Core Gui Widgets OpenGL Concurrent)
foreach (qt_component ${QT_COMPONENTS})
    list(APPEND QT_LINKS
        Qt${QT_VERSION_MAJOR}::${qt_component}
    )
endforeach()
if(ANDROID)
    list(APPEND QT_LINKS Qt${QT_VERSION_MAJOR}::CorePrivate)
endif()

list(APPEND QT_PLUGINS
    platforms/qminimal
    platforms/qoffscreen
    platforms/qwindows
    styles/qwindowsvistastyle
    imageformats/qjpeg
)

find_package(QT NAMES Qt6 COMPONENTS ${QT_COMPONENTS} REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_COMPONENTS} REQUIRED)

### GLM ###
find_package(glm REQUIRED)
# if(WIN32)
#     find_package(GLM REQUIRED)
#     if(GLM_FOUND)
#             set(GLM_INCLUDE_DIRS ${GLM_DIR}/../../../include)
#     endif(GLM_FOUND)
# elseif(UNIX)
#     find_package(glm REQUIRED)
# endif()

### CONTROLLER ###
set(WITH_CONTROLLER ON CACHE BOOL "Include controller functions")
if(WITH_CONTROLLER)
    list(APPEND HEADERS include/Controller.h)
    list(APPEND SOURCES src/Controller.cpp)
    add_compile_definitions(HAVE_CONTROLLER)
endif()

## OPENCV ###
set(WITH_OPENCV OFF CACHE BOOL "Include OpenCV functions")
if(ANDROID)
    set(OpenCV_DIR "" CACHE PATH "OpenCV directory")
endif()
if(WITH_OPENCV)
    list(APPEND OpenCV_COMPONENTS
        core
        calib3d
        dnn
        features2d
        flann
        highgui
        imgcodecs
        imgproc
        video
        videoio
    )

    list(APPEND OpenCV_EXTRA_DLL
        inference_engine$<$<CONFIG:Debug>:d>
        inference_engine_transformations$<$<CONFIG:Debug>:d>
        ngraph$<$<CONFIG:Debug>:d>
        tbb$<$<CONFIG:Debug>:_debug>
    )
    find_package(OpenCV COMPONENTS ${OpenCV_COMPONENTS} REQUIRED)
    add_compile_definitions(HAVE_OPENCV)
endif()

### ViSP ###
set(WITH_VISP OFF CACHE BOOL "ViSP")
if(ANDROID)
    set(VISP_DIR "" CACHE PATH "ViSP directory")
endif()
if(WITH_VISP)
    if(NOT WITH_OPENCV)
        message(FATAL_ERROR "ViSP needs OpenCV to work !")
    endif()

    list(APPEND VISP_COMPONENTS
        blob
        core
        gui
        io
        klt
        mbt
        me
        vision
        visual_features
    )

    find_package(VISP COMPONENTS ${VISP_COMPONENTS} REQUIRED)
    if(VISP_FOUND)
        add_compile_definitions(HAVE_VISP)
    else()
        message(FATAL_ERROR Can't find ViSP.)
    endif()
    if(ANDROID)
        set(VISP_INCLUDE_DIRS ${VISP_DIR}/include)
    endif()
endif()

### VR ###
set(WITH_VR OFF CACHE BOOL "Include VR functions")
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

    list(APPEND HEADERS include/VRheadset.h)
    list(APPEND SOURCES src/VRheadset.cpp)

    add_compile_definitions(HAVE_VR)
endif()

### SOURCES ###
list(APPEND HEADERS
    include/Object3D.h
    include/Object3DQt.h
    include/Camera.h
    include/CameraController.h
    include/Model3D.h
    include/ModelPTS.h
    include/ModelBIN.h
    include/ModelOBJ.h
    include/Octree.h
    include/Model3DWriter.h
    include/Engine3D_global.h
    include/Engine3D.h

    include/TrajectoryManager.h
    include/VideoWriter.h
)

list(APPEND SOURCES
    src/Object3D.cpp
    src/Object3DQt.cpp
    src/Camera.cpp
    src/CameraController.cpp
    src/Model3D.cpp
    src/ModelPTS.cpp
    src/ModelBIN.cpp
    src/ModelOBJ.cpp
    src/Octree.cpp
    src/Model3DWriter.cpp
    src/Engine3D.cpp

    src/TrajectoryManager.cpp
    src/VideoWriter.cpp
)

list(APPEND ENGINE3D_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/Engine3D.qrc)

if(WIN32)
    set(Engine3D_FFmpeg_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/FFmpeg.qrc)
endif()

set(SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Point.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Point.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/SimpleOBJ.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/SimpleOBJ.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/AdvancedOBJ.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/AdvancedOBJ.frag

    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Box.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/Shaders/Box.frag
)

if(BUILD_SHARED_LIBS)
    add_library(Engine3D SHARED
        ${HEADERS}
        ${SOURCES}
        ${ENGINE3D_RESOURCES}
        ${Engine3D_FFmpeg_RESOURCES}
        ${SHADERS}
    )
else()
    add_library(Engine3D STATIC
        ${HEADERS}
        ${SOURCES}

        ${ENGINE3D_RESOURCES}
        ${Engine3D_FFmpeg_RESOURCES}
        ${SHADERS}
    )
endif()

target_include_directories(Engine3D PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${VISP_INCLUDE_DIRS}
    ${OPENVR_INCLUDE_DIRS}
    include
    src
)

target_link_libraries(Engine3D PRIVATE
    ${QT_LINKS}
    ${VISP_LIBRARIES}
    ${OPENVR_LIBRARIES}
)

if(WIN32)
    if(WITH_CONTROLLER)
        target_link_libraries(Engine3D PRIVATE XInput)
    endif()
endif()

target_compile_definitions(Engine3D PRIVATE ENGINE3D_LIBRARY)

if(WIN32)
    install(
        TARGETS Engine3D
        RUNTIME DESTINATION bin/$<CONFIG>
        ARCHIVE DESTINATION lib/$<CONFIG>
    )

    install(FILES ${HEADERS} DESTINATION include)

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ DESTINATION .)

    ### QT ###
    foreach (qt_component ${QT_COMPONENTS})
        list(APPEND QT_DLL "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}${qt_component}$<$<CONFIG:Debug>:d>.dll")
        # list(APPEND QT_LIBS "${Qt${QT_VERSION_MAJOR}_DIR}/../../../lib/Qt${QT_VERSION_MAJOR}${qt_component}$<$<CONFIG:Debug>:d>.lib")
        # install(DIRECTORY ${Qt${QT_VERSION_MAJOR}_DIR}/../../../include/Qt${qt_component} DESTINATION include/3rdParty/Qt)
    endforeach()
    install(FILES ${QT_DLL} DESTINATION bin/$<CONFIG>/3rdParty)
    # install(FILES ${QT_LIBS} DESTINATION lib/$<CONFIG>/3rdParty/Qt)

    foreach (qt_plugin ${QT_PLUGINS})
        string(FIND ${qt_plugin} "/" slash)
        string(SUBSTRING ${qt_plugin} 0 ${slash} folder)
        add_custom_command(TARGET Engine3D POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/bin/$<CONFIG>/3rdParty/${folder}")
        install(FILES "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/${qt_plugin}$<$<CONFIG:Debug>:d>.dll" DESTINATION bin/$<CONFIG>/3rdParty/${folder})
    endforeach()
    file(GLOB QT_INCLUDE_FILES ${Qt${QT_VERSION_MAJOR}_DIR}/../../../include/*.h)
    # install(FILES ${QT_INCLUDE_FILES} DESTINATION include/3rdParty/Qt)

    ### GLM ###
    install(DIRECTORY ${GLM_INCLUDE_DIRS}/glm DESTINATION include/3rdParty/glm)

    if(WITH_OPENCV)
        foreach (OpenCV_COMPONENT ${OpenCV_COMPONENTS})
            list(APPEND OpenCV_INSTALL_LIBS
                "${OpenCV_INSTALL_PATH}/lib/opencv_${OpenCV_COMPONENT}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}$<$<CONFIG:Debug>:d>.lib"
            )
            list(APPEND OpenCV_INSTALL_DLL
                "${OpenCV_DIR}/../bin/opencv_${OpenCV_COMPONENT}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}$<$<CONFIG:Debug>:d>.dll"
            )
        endforeach()

        foreach (OpenCV_EXTRA ${OpenCV_EXTRA_DLL})
            list(APPEND OpenCV_INSTALL_DLL
                "${OpenCV_DIR}/../bin/${OpenCV_EXTRA}.dll"
            )
        endforeach()
        install(FILES ${OpenCV_INSTALL_DLL} DESTINATION bin/$<CONFIG>/3rdParty)
        install(DIRECTORY ${OpenCV_INCLUDE_DIRS}/ DESTINATION include/3rdParty/OpenCV)
        install(FILES ${OpenCV_INSTALL_LIBS} DESTINATION lib/$<CONFIG>/3rdParty/OpenCV)
    endif()

    if(WITH_VISP)
        foreach (visp_component ${VISP_COMPONENTS})
            list(APPEND VISP_EXTRA_LIBS
                "${VISP_DIR}/${VISP_ARCH}/${VISP_RUNTIME}/lib/visp_${visp_component}${VISP_VERSION_MAJOR}${VISP_VERSION_MINOR}${VISP_VERSION_PATCH}$<$<CONFIG:Debug>:d>.lib"
            )
            list(APPEND VISP_EXTRA_DLL
                "${VISP_DIR}/${VISP_ARCH}/${VISP_RUNTIME}/bin/visp_${visp_component}${VISP_VERSION_MAJOR}${VISP_VERSION_MINOR}${VISP_VERSION_PATCH}$<$<CONFIG:Debug>:d>.dll"
            )
        endforeach()
        install(FILES ${VISP_EXTRA_DLL} DESTINATION bin/$<CONFIG>/3rdParty)
        # install(DIRECTORY ${VISP_DIR}/include/ DESTINATION include/3rdParty/ViSP)
        # install(FILES ${VISP_EXTRA_LIBS} DESTINATION lib/$<CONFIG>/3rdParty/ViSP)

        install(DIRECTORY ${VISP_DIR}/ DESTINATION 3rdParty/ViSP)
    endif()

    if(WITH_VR)
        install(FILES "${OPENVR_DIR}/lib/openvr_api64$<$<CONFIG:Debug>:d>.dll" DESTINATION bin/$<CONFIG>/3rdParty)
        install(DIRECTORY ${OPENVR_INCLUDE_DIRS}/ DESTINATION include/3rdParty/OpenVR)
        install(FILES "${OPENVR_DIR}/lib/openvr_api64$<$<CONFIG:Debug>:d>.lib" DESTINATION lib/$<CONFIG>/3rdParty/OpenVR)
    endif()

    file(WRITE Engine3DExtras.cmake)
    if(WITH_CONTROLLER)
        file(APPEND Engine3DExtras.cmake "set(WITH_CONTROLLER ON)\n")
    endif()
    if(WITH_OPENCV)
        file(APPEND Engine3DExtras.cmake "set(WITH_OPENCV ON)\n")
        file(APPEND Engine3DExtras.cmake "set(OpenCV_VERSION ${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH})\n")
        file(APPEND Engine3DExtras.cmake "list(APPEND OpenCV_COMPONENTS ${OpenCV_COMPONENTS})\n")
    endif()
    if(WITH_VISP)
        file(APPEND Engine3DExtras.cmake "set(WITH_VISP ON)\n")
        file(APPEND Engine3DExtras.cmake "set(VISP_VERSION ${VISP_VERSION_MAJOR}${VISP_VERSION_MINOR}${VISP_VERSION_PATCH})\n")
        file(APPEND Engine3DExtras.cmake "list(APPEND VISP_COMPONENTS ${VISP_COMPONENTS})\n")
    endif()
    if(WITH_VR)
        file(APPEND Engine3DExtras.cmake "set(WITH_VR ON)")
    endif()
    install(FILES Engine3DExtras.cmake DESTINATION .)

### 3rdParty ###
    # install(DIRECTORY ${PACKAGE_PREFIX_DIR}/ DESTINATION 3rdParty/Qt)
    # if(WITH_OPENCV)
    #     install(DIRECTORY ${OpenCV_INSTALL_PATH}/ DESTINATION 3rdParty/OpenCV)
    # endif()
    # if(WITH_VISP)
    #     install(DIRECTORY ${VISP_DIR}/ DESTINATION 3rdParty/ViSP)
    # endif()
    # if(WITH_VR)
    #     install(DIRECTORY ${OPENVR_DIR}/ DESTINATION 3rdParty/OpenVR)
    # endif()

    install(DIRECTORY ${CMAKE_INSTALL_PREFIX}/ DESTINATION .)

    set(CPACK_PACKAGE_NAME Engine3D)
    set(CPACK_PACKAGE_VENDOR MIS)
    set(CPACK_PACKAGE_VERSION_MAJOR ${TARGET_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${TARGET_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${TARGET_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY Engine3D)
    include(CPack)
elseif(UNIX)
    find_program(LSB_RELEASE_EXEC lsb_release)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
        OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -rs
        OUTPUT_VARIABLE LSB_RELEASE_VERSION_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    install(TARGETS Engine3D DESTINATION lib)
    if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/libEngine3Dd.so)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEngine3Dd.so DESTINATION lib)
    endif()

    install(FILES ${HEADERS} DESTINATION include/Engine3D)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/
        DESTINATION lib/cmake/Engine3D
    )

    file(WRITE Engine3DExtras.cmake)
    if(WITH_CONTROLLER)
        file(APPEND Engine3DExtras.cmake "set(WITH_CONTROLLER ON)\n")
    endif()
    if(WITH_OPENCV)
        file(APPEND Engine3DExtras.cmake "set(WITH_OPENCV ON)\n")
    endif()
    if(WITH_VISP)
        file(APPEND Engine3DExtras.cmake "list(APPEND VISP_COMPONENTS ${VISP_COMPONENTS})\n")
        file(APPEND Engine3DExtras.cmake "set(WITH_VISP ON)\n")
    endif()
    if(WITH_VR)
        file(APPEND Engine3DExtras.cmake "set(WITH_VR ON)")
    endif()
    install(FILES Engine3DExtras.cmake DESTINATION lib/cmake/Engine3D)

    set(CPACK_PACKAGE_NAME Engine3D)
    set(CPACK_PACKAGE_VENDOR MIS)
    set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY Engine3D)
    set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER n.villette@u-picardie.fr)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_PACKAGE_FILE_NAME Engine3D-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${LSB_RELEASE_ID_SHORT}-${LSB_RELEASE_VERSION_SHORT})
    set(CPACK_GENERATOR DEB)
    include(CPack)
endif()

set(Engine3D_FOUND ON)
set(Engine3D_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(WIN32)
    set(Engine3D_LIBRARIES_DIR ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
elseif(UNIX)
    set(Engine3D_LIBRARIES_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()
set(Engine3D_LIBRARIES Engine3D$<$<CONFIG:Debug>:d>)

set(BUILD_TEST OFF CACHE BOOL "Build test program")
if(BUILD_TEST)
    add_subdirectory(Test)
    if(BUILD_SHARED_LIBS)
        if(WIN32)
            add_custom_command(TARGET Engine3D POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${Engine3D_LIBRARIES_DIR}/${Engine3D_LIBRARIES}.dll"
                $<TARGET_FILE_DIR:Test>/${Engine3D_LIBRARIES}.dll
            )
        endif()
    endif()
endif()

set(BUILD_OFFSCREEN_TEST OFF CACHE BOOL "Build test program")
if(BUILD_OFFSCREEN_TEST)
    add_subdirectory(OffscreenTest)
    if(BUILD_SHARED_LIBS)
        if(WIN32)
            add_custom_command(TARGET Engine3D POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${Engine3D_LIBRARIES_DIR}/${Engine3D_LIBRARIES}.dll"
                $<TARGET_FILE_DIR:OffscreenTest>/${Engine3D_LIBRARIES}.dll
        )
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
