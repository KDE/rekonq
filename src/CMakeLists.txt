### ------- sub dirs -------

ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( tests )


### ------- SETTING REKONQ FILES..

SET( rekonq_KDEINIT_SRCS
    newtabpage.cpp
    previewimage.cpp
    websnap.cpp
    autosaver.cpp 
    application.cpp 
    mainwindow.cpp 
    mainview.cpp
    tabbar.cpp
    history.cpp
    historymodels.cpp
    bookmarks.cpp
    modelmenu.cpp 
    urlbar.cpp
    findbar.cpp
    settings.cpp
    webview.cpp
    sidepanel.cpp
    panelhistory.cpp
    lineedit.cpp
    webpage.cpp
    sessionmanager.cpp
    webpluginfactory.cpp
)


KDE4_ADD_UI_FILES( rekonq_KDEINIT_SRCS
    settings_general.ui
    settings_tabs.ui
    settings_fonts.ui
    settings_webkit.ui
    cleardata.ui
 )

KDE4_ADD_KCFG_FILES( rekonq_KDEINIT_SRCS rekonq.kcfgc )


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)


### -------------- ADDING DEFINITIONS...

ADD_DEFINITIONS ( ${KDE4_DEFINITIONS} )

### --------------- ADDING EXECUTABLE...

KDE4_ADD_KDEINIT_EXECUTABLE( rekonq ${rekonq_KDEINIT_SRCS} main.cpp )


### --------------- TARGETTING LINK LIBRARIES...

TARGET_LINK_LIBRARIES ( kdeinit_rekonq
                        ${QT_LIBRARIES} 
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
INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
