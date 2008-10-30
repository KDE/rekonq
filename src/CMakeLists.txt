SET( rekonq_SRCS
    autosaver.cpp 
    bookmarks.cpp
    browserapplication.cpp 
    browsermainwindow.cpp 
    chasewidget.cpp 
    cookiejar.cpp 
    downloadmanager.cpp 
    edittableview.cpp 
    edittreeview.cpp 
    history.cpp 
    modelmenu.cpp 
    networkaccessmanager.cpp
    searchlineedit.cpp 
    settings.cpp 
    squeezelabel.cpp 
    tabwidget.cpp 
    toolbarsearch.cpp 
    urllineedit.cpp 
    webview.cpp 
    xbel.cpp 
    main.cpp
)

KDE4_ADD_UI_FILES( rekonq_SRCS
    addbookmarkdialog.ui
    bookmarks.ui
    cookies.ui
    cookiesexceptions.ui
    downloaditem.ui
    downloads.ui
    history.ui
    passworddialog.ui
    proxy.ui
    settings.ui
 )

# include(${QT_USE_FILE})

ADD_DEFINITIONS( ${KDE4_DEFINITIONS} )

KDE4_ADD_EXECUTABLE( rekonq ${rekonq_SRCS} )

TARGET_LINK_LIBRARIES( rekonq
    ${QT_LIBRARIES} 
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY} 
	${KDE4_KDEUI_LIBS}
    ${QT_QTUITOOLS_LIBRARY}
)

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
# install(FILES kekonq.rc DESTINATION ${DATA_INSTALL_DIR}/rekonq)
