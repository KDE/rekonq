### ------- sub dirs -------

ADD_SUBDIRECTORY( data )


### ------- SETTING REKONQ FILES..


# src files
set(rekonq_KDEINIT_SRCS
    #----------------------------------------
    application.cpp
    autosaver.cpp
    searchengine.cpp
    sessionmanager.cpp
    urlresolver.cpp
    websnap.cpp
    #----------------------------------------
    adblock/adblockhostmatcher.cpp
    adblock/adblockmanager.cpp
    adblock/adblockrule.cpp
    adblock/adblockrulefallbackimpl.cpp
    adblock/adblockrulenullimpl.cpp
    adblock/adblockruletextmatchimpl.cpp
    adblock/adblockwidget.cpp
    adblock/blockedelementswidget.cpp
    #----------------------------------------
    bookmarks/bookmarkmanager.cpp
    bookmarks/bookmarkscontextmenu.cpp
    bookmarks/bookmarkstoolbar.cpp
    bookmarks/bookmarkowner.cpp
    #----------------------------------------
    download/downloaditem.cpp
    download/downloadmanager.cpp
    #----------------------------------------
    history/historymanager.cpp
    history/historymodels.cpp
    #----------------------------------------
    icons/icondownloader.cpp
    icons/iconmanager.cpp
    icons/webicon.cpp
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
    sync/ftpsynchandler.cpp
    sync/googlesynchandler.cpp
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
    #----------------------------------------
    tabwindow/tabbar.cpp
    tabwindow/tabhighlighteffect.cpp
    tabwindow/tabpreviewpopup.cpp
    tabwindow/tabwindow.cpp
    #----------------------------------------
    useragent/useragentinfo.cpp
    useragent/useragentmanager.cpp
    useragent/useragentwidget.cpp
    #----------------------------------------
    webwindow/networkaccessmanager.cpp
    webwindow/webpage.cpp
    webwindow/webwindow.cpp
    #----------------------------------------
)


# Nepomuk OPTIONAL src files
IF(HAVE_NEPOMUK)
    SET( rekonq_KDEINIT_SRCS
            ${rekonq_KDEINIT_SRCS}
            urlbar/resourcelinkdialog.cpp
            urlbar/newresourcedialog.cpp
    )
ENDIF(HAVE_NEPOMUK)


# Opera sync optional src files
IF(HAVE_QCA2 AND HAVE_QTOAUTH)
    SET( rekonq_KDEINIT_SRCS
            ${rekonq_KDEINIT_SRCS}
            sync/operasynchandler.cpp
    )
ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)


# ui files
KDE4_ADD_UI_FILES( rekonq_KDEINIT_SRCS
    # ----------------------------------------
    adblock/blocked_elements.ui
    adblock/settings_adblock.ui
    #----------------------------------------
    settings/settings_general.ui
    settings/settings_tabs.ui
    settings/settings_appearance.ui
    settings/settings_webkit.ui
    settings/settings_advanced.ui
    settings/settings_privacy.ui
    settings/password_exceptions.ui
    # ----------------------------------------
    sync/sync_check.ui
    sync/sync_data.ui
    sync/sync_ftp_settings.ui
    sync/sync_google_settings.ui
    sync/sync_opera_settings.ui
    sync/sync_host_type.ui
    # ----------------------------------------
    useragent/useragentsettings.ui
 )


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/adblock
                        ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
                        ${CMAKE_CURRENT_SOURCE_DIR}/download
                        ${CMAKE_CURRENT_SOURCE_DIR}/history
                        ${CMAKE_CURRENT_SOURCE_DIR}/icons
                        ${CMAKE_CURRENT_SOURCE_DIR}/settings
                        ${CMAKE_CURRENT_SOURCE_DIR}/sync
                        ${CMAKE_CURRENT_SOURCE_DIR}/urlbar
                        ${CMAKE_CURRENT_SOURCE_DIR}/tabwindow
                        ${CMAKE_CURRENT_SOURCE_DIR}/useragent
                        ${CMAKE_CURRENT_SOURCE_DIR}/webwindow
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)

# Nepomuk optional include directories
IF(HAVE_NEPOMUK)
    INCLUDE_DIRECTORIES(${SOPRANO_INCLUDE_DIR}
                        ${NEPOMUK_INCLUDE_DIR}
    )

    INCLUDE(SopranoAddOntology)
ENDIF(HAVE_NEPOMUK)

# Opera sync optional include directories
IF(HAVE_QCA2 AND HAVE_QTOAUTH)
    INCLUDE_DIRECTORIES(${QCA2_INCLUDE_DIR}
                        ${QTOAUTH_INCLUDE_DIR}
    )
ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)


### -------------- ADDING DEFINITIONS...

ADD_DEFINITIONS ( ${KDE4_DEFINITIONS} )


### -------------- ADDING APPLICATION ICON...

KDE4_ADD_APP_ICON( rekonq_KDEINIT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-rekonq.png )


### -------------- ADDING APPLICATION KCFG FILES...

KDE4_ADD_KCFG_FILES( rekonq_KDEINIT_SRCS rekonq.kcfgc )


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
)

# Nepomuk optional target link libraries
IF(HAVE_NEPOMUK)
    TARGET_LINK_LIBRARIES(  kdeinit_rekonq
                            ${NEPOMUK_LIBRARIES}
                            ${NEPOMUK_QUERY_LIBRARIES}
                            ${SOPRANO_LIBRARIES}
    )
ENDIF(HAVE_NEPOMUK)

# Opera sync optional link libraries
IF(HAVE_QCA2 AND HAVE_QTOAUTH)
    TARGET_LINK_LIBRARIES(  kdeinit_rekonq
                            ${QCA2_LIBRARY}
                            ${QTOAUTH_LIBRARY}
    )
ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)


### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( TARGETS kdeinit_rekonq  ${INSTALL_TARGETS_DEFAULT_ARGS} )
