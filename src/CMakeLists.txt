SET( rekonq_SRCS
    autosaver.cpp 
    application.cpp 
    mainwindow.cpp 
    mainview.cpp
    tabbar.cpp
    cookiejar.cpp 
    edittableview.cpp 
    edittreeview.cpp 
    history.cpp
    download.cpp
    bookmarks.cpp
    modelmenu.cpp 
    networkaccessmanager.cpp
    urlbar.cpp
    findbar.cpp
    searchbar.cpp
    settings.cpp 
    squeezelabel.cpp 
    webview.cpp 
    main.cpp
)

KDE4_ADD_UI_FILES( rekonq_SRCS
    cookies.ui
    cookiesexceptions.ui
    history.ui
    passworddialog.ui
    proxy.ui
    settings_general.ui
    settings_fonts.ui
    settings_privacy.ui
    settings_proxy.ui
 )

KDE4_ADD_KCFG_FILES( rekonq_SRCS rekonq.kcfgc )

### ------------------------------------------

ADD_DEFINITIONS( ${KDE4_DEFINITIONS} )

KDE4_ADD_EXECUTABLE( rekonq ${rekonq_SRCS} )

TARGET_LINK_LIBRARIES( rekonq
    ${QT_LIBRARIES} 
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY} 
	${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
INSTALL( FILES rekonqui.rc  DESTINATION  ${DATA_INSTALL_DIR}/rekonq )
