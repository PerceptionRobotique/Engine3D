if(NOT WITH_OPENCV)
    message(FATAL_ERROR "ViSP needs OpenCV to work !")
endif()

if(ANDROID)
    list(APPEND VISP_COMPONENTS ar)
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

if(WIN32)
    list(APPEND VISP_COMPONENTS vs)
endif()

find_package(VISP COMPONENTS ${VISP_COMPONENTS} REQUIRED)
if(VISP_FOUND)
    add_compile_definitions(HAVE_ViSP)
else()
    message(FATAL_ERROR Can't find ViSP.)
endif()
if(ANDROID)
    set(VISP_INCLUDE_DIRS ${VISP_DIR}/include)
endif()

### INSTALL ###
if(WIN32)
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