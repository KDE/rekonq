### ------- sub dirs -------

ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( tests )


### ------- SETTING REKONQ FILES..

SET( rekonq_KDEINIT_SRCS
    application.cpp
    clicktoflash.cpp
    downloaditem.cpp
    downloadmanager.cpp
    findbar.cpp
    icondownloader.cpp
    iconmanager.cpp
    mainview.cpp
    mainwindow.cpp
    messagebar.cpp
    networkaccessmanager.cpp
    newtabpage.cpp
    paneltreeview.cpp
    previewselectorbar.cpp
    protocolhandler.cpp
    rekonqmenu.cpp
    searchenginebar.cpp
    sessionmanager.cpp
    sslinfodialog.cpp
    tabpreviewpopup.cpp
    tabbar.cpp
    tabhighlighteffect.cpp
    thumbupdater.cpp
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
    settings/advancedwidget.cpp
    settings/generalwidget.cpp
    settings/privacywidget.cpp
    settings/tabswidget.cpp
    settings/webkitwidget.cpp
    settings/passexceptionswidget.cpp
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
    adblock/blockedelementswidget.cpp
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
    sync/ftpsynchandler.cpp
    sync/googlesynchandler.cpp
    sync/operasynchandler.cpp
    sync/syncassistant.cpp
    sync/synchandler.cpp
    sync/syncmanager.cpp
    #
    sync/synccheckwidget.cpp
    sync/syncdatawidget.cpp
    sync/synchosttypewidget.cpp
    sync/syncftpsettingswidget.cpp
    sync/syncgooglesettingswidget.cpp
    sync/syncoperasettingswidget.cpp
)

IF(HAVE_NEPOMUK)
    SET( rekonq_KDEINIT_SRCS
            ${rekonq_KDEINIT_SRCS}
            urlbar/resourcelinkdialog.cpp
            urlbar/newresourcedialog.cpp
    )
ENDIF(HAVE_NEPOMUK)

KDE4_ADD_UI_FILES( rekonq_KDEINIT_SRCS
    settings/settings_general.ui
    settings/settings_tabs.ui
    settings/settings_appearance.ui
    settings/settings_webkit.ui
    settings/settings_advanced.ui
    settings/settings_privacy.ui
    settings/password_exceptions.ui
    # ----------------------------------------
    adblock/settings_adblock.ui
    adblock/blocked_elements.ui
    # ----------------------------------------
    cleardata.ui
    sslinfo.ui
    webappcreation.ui
    useragent/useragentsettings.ui
    # ----------------------------------------
    sync/sync_check.ui
    sync/sync_data.ui
    sync/sync_ftp_settings.ui
    sync/sync_google_settings.ui
    sync/sync_opera_settings.ui
    sync/sync_host_type.ui
 )

KDE4_ADD_KCFG_FILES( rekonq_KDEINIT_SRCS rekonq.kcfgc )

IF(HAVE_NEPOMUK)
    INCLUDE(SopranoAddOntology)
ENDIF(HAVE_NEPOMUK)

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
                        ${QCA2_INCLUDE_DIR}
                        ${QTOAUTH_INCLUDE_DIR}
)

IF(HAVE_NEPOMUK)
    INCLUDE_DIRECTORIES(${SOPRANO_INCLUDE_DIR}
                        ${NEPOMUK_INCLUDE_DIR}
    )
ENDIF(HAVE_NEPOMUK)

### -------------- ADDING DEFINITIONS...

ADD_DEFINITIONS ( ${KDE4_DEFINITIONS} )

### -------------- ADDING APPLICATION ICON...

KDE4_ADD_APP_ICON( rekonq_KDEINIT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-rekonq.png )

### --------------- ADDING EXECUTABLE...

KDE4_ADD_KDEINIT_EXECUTABLE( rekonq ${rekonq_KDEINIT_SRCS} main.cpp )


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
                        ${QCA2_LIBRARY}
                        ${QTOAUTH_LIBRARY}
)

IF(HAVE_NEPOMUK)
    TARGET_LINK_LIBRARIES(  kdeinit_rekonq
                            ${NEPOMUK_LIBRARIES}
                            ${NEPOMUK_QUERY_LIBRARIES}
                            ${SOPRANO_LIBRARIES}
    )
ENDIF(HAVE_NEPOMUK)

### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( TARGETS kdeinit_rekonq  ${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
INSTALL( FILES rekonqui.rc  DESTINATION  ${DATA_INSTALL_DIR}/rekonq )
