# list(APPEND OpenCV_COMPONENTS
#     core
#     calib3d
#     dnn
#     features2d
#     flann
#     highgui
#     imgcodecs
#     imgproc
#     video
#     videoio
# )
list(APPEND OpenCV_COMPONENTS
    world
)

# list(APPEND OpenCV_EXTRA_DLL
#     inference_engine$<$<CONFIG:Debug>:d>
#     inference_engine_transformations$<$<CONFIG:Debug>:d>
#     ngraph$<$<CONFIG:Debug>:d>
#     tbb$<$<CONFIG:Debug>:_debug>
# )
find_package(OpenCV COMPONENTS ${OpenCV_COMPONENTS} REQUIRED)
add_compile_definitions(HAVE_OPENCV)

### INSTALL ###
if(WIN32)
    foreach (OpenCV_COMPONENT ${OpenCV_COMPONENTS})
        list(APPEND OpenCV_INSTALL_LIBS
            "${OpenCV_INSTALL_PATH}/${OpenCV_ARCH}/${OpenCV_RUNTIME}/lib/opencv_${OpenCV_COMPONENT}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}$<$<CONFIG:Debug>:d>.lib"
        )
        list(APPEND OpenCV_INSTALL_DLL
            "${OpenCV_INSTALL_PATH}/${OpenCV_ARCH}/${OpenCV_RUNTIME}/bin/opencv_${OpenCV_COMPONENT}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH}$<$<CONFIG:Debug>:d>.dll"
        )
        endforeach()

        foreach (OpenCV_EXTRA ${OpenCV_EXTRA_DLL})
        list(APPEND OpenCV_INSTALL_DLL
            "${OpenCV_INSTALL_PATH}/${OpenCV_ARCH}/${OpenCV_RUNTIME}/bin/${OpenCV_EXTRA}.dll"
        )
    endforeach()
    install(FILES ${OpenCV_INSTALL_DLL} DESTINATION bin/$<CONFIG>/3rdParty)
    install(DIRECTORY ${OpenCV_INCLUDE_DIRS}/ DESTINATION include/3rdParty/OpenCV)
    install(FILES ${OpenCV_INSTALL_LIBS} DESTINATION lib/$<CONFIG>/3rdParty/OpenCV)
endif()