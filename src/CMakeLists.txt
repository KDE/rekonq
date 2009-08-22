### ------- SETTING REKONQ FILES..

SET( rekonq_SRCS
    networkaccessmanager.cpp
    autosaver.cpp 
    application.cpp 
    mainwindow.cpp 
    mainview.cpp
    tabbar.cpp
    cookiejar.cpp
    history.cpp
    historymenu.cpp
    bookmarks.cpp
    modelmenu.cpp 
    urlbar.cpp
    findbar.cpp
    settings.cpp
    webview.cpp 
    main.cpp
    sidepanel.cpp
    panelhistory.cpp
    lineedit.cpp
    webpage.cpp
)


KDE4_ADD_UI_FILES( rekonq_SRCS
    password.ui
    proxy.ui
    settings_general.ui
    settings_fonts.ui
    settings_proxy.ui
    settings_webkit.ui
    cleardata.ui
 )

KDE4_ADD_KCFG_FILES( rekonq_SRCS rekonq.kcfgc )


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)


### -------------- ADDING DEFINITIONS...

ADD_DEFINITIONS ( ${KDE4_DEFINITIONS} )

### --------------- ADDING EXECUTABLE...

KDE4_ADD_EXECUTABLE( rekonq ${rekonq_SRCS} )


### --------------- TARGETTING LINK LIBRARIES...

TARGET_LINK_LIBRARIES ( rekonq
                        ${QT_LIBRARIES} 
                        ${QT_QTNETWORK_LIBRARY}
                        ${QT_QTWEBKIT_LIBRARY} 
                        ${QT_QTUITOOLS_LIBRARY}
                        ${KDE4_KUTILS_LIBS}
                        ${KDE4_KDEUI_LIBS}
)


### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
INSTALL( FILES rekonqui.rc  DESTINATION  ${DATA_INSTALL_DIR}/rekonq )
