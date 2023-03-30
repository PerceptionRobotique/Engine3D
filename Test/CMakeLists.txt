cmake_minimum_required(VERSION 3.16)

project(TEST VERSION 1.0)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT TEST)
endif()

add_executable(TEST main.cpp)

target_include_directories(TEST PRIVATE ${Engine3D_INCLUDE_DIRS})

target_link_directories(TEST PRIVATE ${Engine3D_LIBRARIES_DIR})

target_link_libraries(TEST PRIVATE ${Engine3D_LIBRARIES})

if(WIN32)
    set(USE_TERMINAL_OUTPUT ON CACHE BOOL "Launch with a terminal")
    if(NOT ${USE_TERMINAL_OUTPUT})
        set_target_properties(TEST PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()
endif()

if(WIN32)
    add_custom_command(TARGET TEST POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Engine3D_DIR}/$<CONFIG>/Engine3D$<$<CONFIG:Debug>:d>.dll $<TARGET_FILE_DIR:TEST>)

    foreach(qt_dll ${QT_DLL})
        add_custom_command(TARGET TEST POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${qt_dll} $<TARGET_FILE_DIR:TEST>)
    endforeach()

    foreach(opencv_install_dll ${OpenCV_INSTALL_DLL})
        add_custom_command(TARGET TEST POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${opencv_install_dll} $<TARGET_FILE_DIR:TEST>)
    endforeach()

    foreach(visp_extra_dll ${VISP_EXTRA_DLL})
        add_custom_command(TARGET TEST POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${visp_extra_dll} $<TARGET_FILE_DIR:TEST>)
    endforeach()

    if(WITH_OpenVR)
        add_custom_command(TARGET TEST POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OPENVR_DIR}/bin/win64/openvr_api.dll" $<TARGET_FILE_DIR:TEST>)
    endif()

    foreach(qt_plugin ${QT_PLUGINS})
        string(FIND ${qt_plugin} "/" slash)
        string(SUBSTRING ${qt_plugin} 0 ${slash} folder)
        add_custom_command(TARGET TEST POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:TEST>/${folder})
        add_custom_command(TARGET TEST POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${Qt${QT_VERSION_MAJOR}_DIR}/../../../plugins/${qt_plugin}$<$<CONFIG:Debug>:d>.dll" $<TARGET_FILE_DIR:TEST>/${folder})
    endforeach()
endif()