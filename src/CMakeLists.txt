### ------- sub dirs -------

ADD_SUBDIRECTORY( data )


### ------- SETTING REKONQ FILES..

set(rekonq_KDEINIT_SRCS
    #----------------------------------------
    tabwindow/tabbar.cpp
    tabwindow/tabhighlighteffect.cpp
    tabwindow/tabwindow.cpp
    #----------------------------------------
    webwindow/webpage.cpp
    webwindow/webwindow.cpp
    #----------------------------------------
    websnap.cpp
)


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/tabwindow
                        ${CMAKE_CURRENT_SOURCE_DIR}/webwindow
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)

### -------------- ADDING DEFINITIONS...

ADD_DEFINITIONS ( ${KDE4_DEFINITIONS} )


### -------------- ADDING APPLICATION ICON...

KDE4_ADD_APP_ICON( rekonq_KDEINIT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-rekonq.png )


### --------------- ADDING EXECUTABLE...

# NOTE: This is the simple main used to test the tabwindow :)
KDE4_ADD_KDEINIT_EXECUTABLE( rekonq ${rekonq_KDEINIT_SRCS} tabmain.cpp )


### --------------- TARGETTING LINK LIBRARIES...

TARGET_LINK_LIBRARIES (     kdeinit_rekonq
                            ${QT_LIBRARIES}
                            ${QT_QTSCRIPT_LIBRARY}
                            ${QT_QTWEBKIT_LIBRARY}
                            ${KDE4_KDEWEBKIT_LIBS}
                            ${KDE4_KUTILS_LIBS}
                            ${KDE4_KDEUI_LIBS}
                            ${KDE4_KIO_LIBS}
                            ${KDE4_KPARTS_LIBS}
)


### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( TARGETS kdeinit_rekonq  ${INSTALL_TARGETS_DEFAULT_ARGS} )
