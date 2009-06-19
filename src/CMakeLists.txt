### ------- SETTING REKONQ FILES..

if(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")

SET( rekonq_NAM networkmanager.cpp )

else(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")    

SET( rekonq_NAM kaccessmanager.cpp )

endif(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")

SET( rekonq_SRCS
    ${rekonq_NAM}
    autosaver.cpp 
    application.cpp 
    mainwindow.cpp 
    mainview.cpp
    tabbar.cpp
    cookiejar.cpp 
    edittableview.cpp 
    edittreeview.cpp 
    history.cpp
    historymenu.cpp
    bookmarks.cpp
    modelmenu.cpp 
    urlbar.cpp
    findbar.cpp
    searchbar.cpp
    settings.cpp
    webview.cpp 
    main.cpp
    sidepanel.cpp
    panelhistory.cpp
    lineedit.cpp
    stackedurlbar.cpp
    webpage.cpp
    cookiedialog.cpp
    cookieexceptiondialog.cpp
)


KDE4_ADD_UI_FILES( rekonq_SRCS
    cookies.ui
    cookiesexceptions.ui
    history.ui
    password.ui
    proxy.ui
    settings_general.ui
    settings_fonts.ui
    settings_privacy.ui
    settings_proxy.ui
    settings_webkit.ui
 )

KDE4_ADD_KCFG_FILES( rekonq_SRCS rekonq.kcfgc )



### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
                        ${KDEWEBKIT_INCLUDE_DIR}    
)

if(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")
else(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")    

    INCLUDE_DIRECTORIES( ${KDENETWORK_INCLUDE_DIR} )

endif(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")



### -------------- ADDING DEFINITIONS...

ADD_DEFINITIONS (   ${KDE4_DEFINITIONS}
                    ${KDEWEBKIT_DEFINITIONS}
)

if(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")
else(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")    

    ADD_DEFINITIONS ( ${KDENETWORK_DEFINITIONS} )

endif(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")



### --------------- ADDING EXECUTABLE...

KDE4_ADD_EXECUTABLE( rekonq ${rekonq_SRCS} )



### --------------- TARGETTING LINK LIBRARIES...

TARGET_LINK_LIBRARIES ( rekonq
                        ${QT_LIBRARIES} 
                        ${QT_QTNETWORK_LIBRARY}
                        ${KDE4_KUTILS_LIBS}
                        ${KDE4_KDEUI_LIBS}
                        ${KDE4_KIO_LIBS}
                        ${KDEWEBKIT_LIBRARIES}
)

if(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")
else(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")    

    TARGET_LINK_LIBRARIES( ${KDENETWORK_LIBRARIES} )

endif(${KDE_VERSION_MINOR} EQUAL "2" AND ${KDE_VERSION_RELEASE} GREATER "69")



### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
INSTALL( FILES rekonqui.rc  DESTINATION  ${DATA_INSTALL_DIR}/rekonq )
