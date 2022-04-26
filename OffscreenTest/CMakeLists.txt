cmake_minimum_required(VERSION 3.16)

project(OffscreenTest VERSION 1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_PARALLEL_LEVEL ON)
if(MSVC)
    add_definitions(/MP /bigobj)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(PRINT_CMAKE_VARIABLES OFF CACHE BOOL "Print CMake variables")

if(FRAME_COUNTER)
    add_compile_definitions(FRAME_COUNTER)
endif()

### QT ###
list(APPEND QT_COMPONENTS Gui)
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

set(SOURCES
    main.cpp
)

set(HEADERS
)

set(RESOURCES
)

set(UI
)

set(PROJECT_SOURCES
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${UI}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(OffscreenTest
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET OffscreenTest APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(OffscreenTest SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(OffscreenTest
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(OffscreenTest
    PRIVATE
    ${GLM_INCLUDE_DIRS}
    ${Engine3D_INCLUDE_DIRS}
)

target_link_directories(OffscreenTest PRIVATE ${Engine3D_LIBRARIES_DIR})

target_link_libraries(OffscreenTest
    PRIVATE
    ${QT_LINKS}
    ${Engine3D_LIBRARIES}
)   

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OffscreenTest)
endif()

if(WIN32)
    add_custom_command(TARGET OffscreenTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OffscreenTest>/platforms"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:OffscreenTest>/styles"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qminimal$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qoffscreen$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>/platforms

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/styles/qwindowsvistastyle$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>/styles

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Core$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Gui$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Widgets$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}OpenGL$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}OpenGLWidgets$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin/Qt${QT_VERSION_MAJOR}Concurrent$<$<CONFIG:Debug>:d>.dll"
        $<TARGET_FILE_DIR:OffscreenTest>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Engine3D_LIBRARIES_DIR}/${Engine3D_LIBRARIES}.dll"
        $<TARGET_FILE_DIR:OffscreenTest>
    )
endif()

if(PRINT_CMAKE_VARIABLES)
        get_cmake_property(_variableNames VARIABLES)
        list (SORT _variableNames)
        foreach (_variableName ${_variableNames})
                message(STATUS "${_variableName}=${${_variableName}}")
        endforeach()
endif()
