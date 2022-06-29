cmake_minimum_required(VERSION 3.16)

project(VRTest VERSION 1.0)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT VRTest)
endif()

find_package(Engine3D REQUIRED)

add_executable(VRTest main.cpp)

target_include_directories(VRTest PRIVATE ${Engine3D_INCLUDE_DIRS})
target_link_libraries(VRTest PRIVATE ${Engine3D_LIBRARIES})

if(WIN32)
    add_custom_command(TARGET VRTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${Engine3D_DLL}
        $<TARGET_FILE_DIR:VRTest>
    )

    add_custom_command(TARGET VRTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${Engine3D_EXTRA_DLL_DIR}
        $<TARGET_FILE_DIR:VRTest>
    )
endif()