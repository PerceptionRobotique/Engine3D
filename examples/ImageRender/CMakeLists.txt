cmake_minimum_required(VERSION 3.16)

project(ImageRender VERSION 1.0)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT ImageRender)
endif()

find_package(Engine3D REQUIRED)

add_executable(ImageRender main.cpp)

target_include_directories(ImageRender PRIVATE ${Engine3D_INCLUDE_DIRS})
target_link_libraries(ImageRender PRIVATE ${Engine3D_LIBRARIES})

if(WIN32)
    add_custom_command(TARGET ImageRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${Engine3D_DLL}
        $<TARGET_FILE_DIR:ImageRender>
    )

    add_custom_command(TARGET ImageRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${Engine3D_EXTRA_DLL_DIR}
        $<TARGET_FILE_DIR:ImageRender>
    )
endif()