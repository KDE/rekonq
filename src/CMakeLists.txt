### ------- sub dirs -------

ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( tests )


### ------- SETTING REKONQ FILES..

SET( rekonq_KDEINIT_SRCS
    application.cpp
    clicktoflash.cpp
    downloaditem.cpp
    downloadmanager.cpp
    filterurljob.cpp
    findbar.cpp
    iconmanager.cpp
    mainview.cpp
    mainwindow.cpp
    messagebar.cpp
    networkaccessmanager.cpp
    newtabpage.cpp
    paneltreeview.cpp
    previewselectorbar.cpp
    protocolhandler.cpp
    sessionmanager.cpp
    sslinfodialog.cpp
    tabpreviewpopup.cpp
    tabbar.cpp
    tabhighlighteffect.cpp
    urlfilterproxymodel.cpp
    urlpanel.cpp
    walletbar.cpp
    webicon.cpp
    webinspectorpanel.cpp
    webpage.cpp
    webpluginfactory.cpp
    websslinfo.cpp
    websnap.cpp
    webview.cpp
    webtab.cpp
    zoombar.cpp
    #----------------------------------------
    history/autosaver.cpp
    history/historymanager.cpp
    history/historymodels.cpp
    history/historypanel.cpp
    #----------------------------------------
    settings/settingsdialog.cpp
    settings/appearancewidget.cpp
    settings/generalwidget.cpp
    settings/tabswidget.cpp
    settings/webkitwidget.cpp
    settings/networkwidget.cpp
    #----------------------------------------
    bookmarks/bookmarkmanager.cpp
    bookmarks/bookmarkspanel.cpp
    bookmarks/bookmarkstreemodel.cpp
    bookmarks/bookmarkscontextmenu.cpp
    bookmarks/bookmarkstoolbar.cpp
    bookmarks/bookmarkowner.cpp
    #----------------------------------------
    adblock/adblockhostmatcher.cpp
    adblock/adblockmanager.cpp
    adblock/adblocknetworkreply.cpp
    adblock/adblockrule.cpp
    adblock/adblockrulefallbackimpl.cpp
    adblock/adblockrulenullimpl.cpp
    adblock/adblockruletextmatchimpl.cpp
    adblock/adblockwidget.cpp
    #----------------------------------------
    urlbar/stackedurlbar.cpp
    urlbar/urlbar.cpp
    urlbar/completionwidget.cpp
    urlbar/urlresolver.cpp
    urlbar/listitem.cpp
    urlbar/rsswidget.cpp
    urlbar/sslwidget.cpp
    urlbar/bookmarkwidget.cpp
    urlbar/webshortcutwidget.cpp
    urlbar/favoritewidget.cpp
    #----------------------------------------
    analyzer/analyzerpanel.cpp
    analyzer/networkanalyzer.cpp
    #----------------------------------------
    opensearch/searchengine.cpp
    opensearch/opensearchwriter.cpp
    opensearch/opensearchreader.cpp
    opensearch/opensearchmanager.cpp
    opensearch/opensearchengine.cpp
    opensearch/suggestionparser.cpp
    #----------------------------------------
    useragent/useragentinfo.cpp
    useragent/useragentmanager.cpp
    useragent/useragentwidget.cpp
    #----------------------------------------
    sync/syncmanager.cpp
    sync/syncwidget.cpp
    sync/ftpsynchandler.cpp
)


KDE4_ADD_UI_FILES( rekonq_KDEINIT_SRCS
    settings/settings_general.ui
    settings/settings_tabs.ui
    settings/settings_appearance.ui
    settings/settings_webkit.ui
    adblock/settings_adblock.ui
    cleardata.ui
    sslinfo.ui
    webappcreation.ui
    useragent/useragentsettings.ui
    sync/settings_sync.ui
 )

KDE4_ADD_KCFG_FILES( rekonq_KDEINIT_SRCS rekonq.kcfgc )


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/adblock
                        ${CMAKE_CURRENT_SOURCE_DIR}/analyzer
                        ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
                        ${CMAKE_CURRENT_SOURCE_DIR}/history
                        ${CMAKE_CURRENT_SOURCE_DIR}/opensearch
                        ${CMAKE_CURRENT_SOURCE_DIR}/settings
                        ${CMAKE_CURRENT_SOURCE_DIR}/sync
                        ${CMAKE_CURRENT_SOURCE_DIR}/urlbar
                        ${CMAKE_CURRENT_SOURCE_DIR}/useragent
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
                        ${QT_QTSCRIPT_LIBRARY}
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
INSTALL( FILES rekonqui.rc  DESTINATION  ${DATA_INSTALL_DIR}/rekonq )
