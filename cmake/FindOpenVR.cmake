if(WIN32)
    set(OPENVR_DIR "OPENVR_DIR-NOTFOUND" CACHE PATH "Directory of OpenVR")
    if(IS_DIRECTORY ${OPENVR_DIR})
        list(APPEND OPENVR_INCLUDE_DIRS ${OPENVR_DIR}/headers)
        set(OPENVR_LIBRARIES ${OPENVR_DIR}/bin/win64/$<CONFIG>/openvr_api64$<$<CONFIG:Debug>:d>.lib)
    else()
        message(FATAL_ERROR "OpenVR is missing.")
    endif()
elseif(UNIX)
    set(OPENVR_LIBRARIES /usr/lib/x86_64-linux-gnu/libopenvr_api.so)
endif()

list(APPEND HEADERS include/VRheadset.h)
list(APPEND SOURCES src/VRheadset.cpp)

add_compile_definitions(HAVE_OpenVR)

### INSTALL ###

if(WIN32)
    install(FILES "${OPENVR_DIR}/bin/win64/$<CONFIG>/openvr_api64$<$<CONFIG:Debug>:d>.dll" DESTINATION bin/$<CONFIG>/3rdParty)
    install(DIRECTORY ${OPENVR_INCLUDE_DIRS}/ DESTINATION include/3rdParty/OpenVR)
    install(FILES "${OPENVR_DIR}/bin/win64/$<CONFIG>/openvr_api64$<$<CONFIG:Debug>:d>.lib" DESTINATION lib/$<CONFIG>/3rdParty/OpenVR)
endif()