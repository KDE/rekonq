SET( rekonq_SRCS
    autosaver.cpp 
    browserapplication.cpp 
    mainwindow.cpp 
#     rekonqview.cpp
    cookiejar.cpp 
    downloadmanager.cpp 
    edittableview.cpp 
    edittreeview.cpp 
    history.cpp 
    bookmarks.cpp
    modelmenu.cpp 
    networkaccessmanager.cpp
    urlbar.cpp
    findbar.cpp
    searchbar.cpp
    settings.cpp 
    squeezelabel.cpp 
    tabwidget.cpp
    webview.cpp 
    main.cpp
)

KDE4_ADD_UI_FILES( rekonq_SRCS
    cookies.ui
    cookiesexceptions.ui
    downloaditem.ui
    downloads.ui
    history.ui
    passworddialog.ui
    proxy.ui
    settings_general.ui
    settings_appearance.ui
    settings_privacy.ui
    settings_proxy.ui
 )

# include(${QT_USE_FILE})

ADD_DEFINITIONS( ${KDE4_DEFINITIONS} )

KDE4_ADD_EXECUTABLE( rekonq ${rekonq_SRCS} )

TARGET_LINK_LIBRARIES( rekonq
    ${QT_LIBRARIES} 
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY} 
	${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
#     ${QT_QTUITOOLS_LIBRARY}
)

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

#INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
INSTALL( FILES rekonqui.rc  DESTINATION  ${DATA_INSTALL_DIR}/rekonq )
