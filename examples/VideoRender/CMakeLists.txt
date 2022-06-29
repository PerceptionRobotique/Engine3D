cmake_minimum_required(VERSION 3.16)

project(VideoRender VERSION 1.0)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT VideoRender)
endif()

find_package(Engine3D REQUIRED)

add_executable(VideoRender main.cpp)

target_include_directories(VideoRender PRIVATE ${Engine3D_INCLUDE_DIRS})
target_link_libraries(VideoRender PRIVATE ${Engine3D_LIBRARIES})

if(WIN32)
    add_custom_command(TARGET VideoRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${Engine3D_DLL}
        $<TARGET_FILE_DIR:VideoRender>
    )

    add_custom_command(TARGET VideoRender POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${Engine3D_EXTRA_DLL_DIR}
        $<TARGET_FILE_DIR:VideoRender>
    )
endif()