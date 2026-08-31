cmake_minimum_required(VERSION 3.16)

project(MultiCamera VERSION 1.0)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT MultiCamera)
endif()

find_package(Engine3D REQUIRED)

add_executable(MultiCamera main.cpp)

target_include_directories(MultiCamera PRIVATE ${Engine3D_INCLUDE_DIRS})
target_link_libraries(MultiCamera PRIVATE ${Engine3D_LIBRARIES})

if(WIN32)
    add_custom_command(TARGET MultiCamera POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${Engine3D_DLL}
        $<TARGET_FILE_DIR:MultiCamera>
    )

    add_custom_command(TARGET MultiCamera POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${Engine3D_EXTRA_DLL_DIR}
        $<TARGET_FILE_DIR:MultiCamera>
    )
endif()