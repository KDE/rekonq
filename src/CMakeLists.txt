### ------- sub dirs -------

ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( tests )


### ------- SETTING REKONQ FILES..

SET( rekonq_KDEINIT_SRCS
    application.cpp 
    clicktoflash.cpp
    filterurljob.cpp
    findbar.cpp
    mainview.cpp
    mainwindow.cpp 
    networkaccessmanager.cpp
    newtabpage.cpp
    paneltreeview.cpp
    previewselectorbar.cpp
    protocolhandler.cpp
    sessionmanager.cpp
    tabbar.cpp
    walletbar.cpp
    webinspectorpanel.cpp
    webpage.cpp
    webpluginfactory.cpp
    websslinfo.cpp
    websnap.cpp
    webview.cpp
    webtab.cpp
    searchengine.cpp
    #----------------------------------------
    history/autosaver.cpp 
    history/historymanager.cpp
    history/historymodels.cpp
    history/historypanel.cpp
    #----------------------------------------
    settings/settingsdialog.cpp
    settings/adblockwidget.cpp
    settings/networkwidget.cpp
    #----------------------------------------
    bookmarks/bookmarksmanager.cpp
    bookmarks/bookmarkspanel.cpp
    bookmarks/bookmarkstreemodel.cpp
    bookmarks/bookmarksproxy.cpp
    bookmarks/bookmarkcontextmenu.cpp
    #----------------------------------------
    adblock/adblockmanager.cpp
    adblock/adblocknetworkreply.cpp
    adblock/adblockrule.cpp
    #----------------------------------------
    urlbar/urlbar.cpp
    urlbar/completionwidget.cpp
    urlbar/urlresolver.cpp
    urlbar/listitem.cpp
)


KDE4_ADD_UI_FILES( rekonq_KDEINIT_SRCS
    settings/settings_general.ui
    settings/settings_tabs.ui
    settings/settings_fonts.ui
    settings/settings_webkit.ui
    settings/settings_adblock.ui
    cleardata.ui
 )

KDE4_ADD_KCFG_FILES( rekonq_KDEINIT_SRCS rekonq.kcfgc )


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/adblock
                        ${CMAKE_CURRENT_SOURCE_DIR}/urlbar
                        ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
                        ${CMAKE_CURRENT_SOURCE_DIR}/history
                        ${CMAKE_CURRENT_SOURCE_DIR}/rekonqpage
                        ${CMAKE_CURRENT_SOURCE_DIR}/settings
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
                        ${KDE4_THREADWEAVER_LIBRARIES}
)


### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( TARGETS kdeinit_rekonq  ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
