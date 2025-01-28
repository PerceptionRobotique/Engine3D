### FIND ###

# set(QT_VERSION "6.3.2" CACHE STRING "Currently installed Qt version")
if(WIN32)
    # set(QT_DIR "C:/Qt/${QT_VERSION}/msvc2019_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
    set(QT_DIR "" CACHE PATH "Qt directory")
elseif(UNIX)
    if(IS_DIRECTORY "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6")
        set(QT_DIR "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64/lib/cmake/Qt6" CACHE PATH "Qt directory")
    elseif(IS_DIRECTORY "/usr/local/Qt-${QT_VERSION}/lib/cmake/Qt6")
        set(QT_DIR "/usr/local/Qt-${QT_VERSION}/lib/cmake/Qt6" CACHE PATH "Qt directory")
    else()
        set(QT_DIR "QT_DIR-NOTFOUND" CACHE PATH "Qt directory")
    endif()

    if(IS_DIRECTORY ${QT_DIR})
        set(Qt6DBusTools_DIR "${QT_DIR}DBusTools")
        set(Qt6BundledPcre2_DIR "${QT_DIR}BundledPcre2")
        set(Qt6BundledHarfbuzz_DIR "${QT_DIR}BundledHarfbuzz")
        set(Qt6BundledLibjpeg_DIR "${QT_DIR}BundledLibjpeg")
    endif()
endif()

if(IS_DIRECTORY ${QT_DIR})
    set(Qt6_DIR "${QT_DIR}")
    set(Qt6CoreTools_DIR "${QT_DIR}CoreTools")
    set(Qt6GuiTools_DIR "${QT_DIR}GuiTools")
    set(Qt6WidgetsTools_DIR "${QT_DIR}WidgetsTools")
endif()

list(APPEND QT_COMPONENTS Core Gui Widgets OpenGL Concurrent)
foreach (qt_component ${QT_COMPONENTS})
    list(APPEND QT_LINKS
        Qt${QT_VERSION_MAJOR}::${qt_component}
    )
endforeach()
if(ANDROID)
    list(APPEND QT_LINKS Qt${QT_VERSION_MAJOR}::CorePrivate)
endif()

list(APPEND QT_PLUGINS
    platforms/qminimal
    platforms/qoffscreen
    platforms/qwindows
    styles/qwindowsvistastyle
    imageformats/qjpeg
)

find_package(QT NAMES Qt6 COMPONENTS ${QT_COMPONENTS} REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_COMPONENTS} REQUIRED)

### INSTALL ###

if(WIN32)
    #foreach (qt_component ${QT_COMPONENTS})
    #    list(APPEND QT_DLL "${Qt${QT_VERSION_MAJOR}_DIR}/../../bin/Qt${QT_VERSION_MAJOR}${qt_component}$<$<CONFIG:Debug>:d>.dll")
    #endforeach()
    #install(FILES ${QT_DLL} DESTINATION bin/$<CONFIG>/3rdParty)
    
    #foreach (qt_plugin ${QT_PLUGINS})
    #    string(FIND ${qt_plugin} "/" slash)
    #    string(SUBSTRING ${qt_plugin} 0 ${slash} folder)
    #    install(FILES "${Qt${QT_VERSION_MAJOR}_DIR}/../../Qt6/plugins/${qt_plugin}$<$<CONFIG:Debug>:d>.dll" DESTINATION bin/$<CONFIG>/3rdParty/${folder})
    #endforeach()
    #file(GLOB QT_INCLUDE_FILES ${Qt${QT_VERSION_MAJOR}_DIR}/../../include/*.h)
endif()