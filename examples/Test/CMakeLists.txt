cmake_minimum_required(VERSION 3.16)

project(Test VERSION 1.0)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT Test)
endif()

find_package(Engine3D REQUIRED)

add_executable(Test main.cpp)

target_include_directories(Test PRIVATE ${Engine3D_INCLUDE_DIRS})
target_link_libraries(Test PRIVATE ${Engine3D_LIBRARIES})

if(WIN32)
    add_custom_command(TARGET Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${Engine3D_DLL}
        $<TARGET_FILE_DIR:Test>
    )

    add_custom_command(TARGET Test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${Engine3D_EXTRA_DLL_DIR}
        $<TARGET_FILE_DIR:Test>
    )
endif()