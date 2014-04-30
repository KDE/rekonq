### ------- sub dirs -------

ADD_SUBDIRECTORY (data)


### ------- SETTING REKONQ FILES...
#ADD_DEFINITIONS (-DQT_STATICPLUGIN)
# QT4_WRAP_CPP(wk_HEADERS_MOC qwebkitplatformplugin.h)

# src files
SET (rekonq_KDEINIT_SRCS
    #----------------------------------------
    application.cpp
    autosaver.cpp
    cleardatadialog.cpp
    searchengine.cpp
    sessionmanager.cpp
    sessiondialog.cpp
    urlresolver.cpp
    webappshortcutdialog.cpp
    websnap.cpp
    #----------------------------------------
    adblock/adblockelementhiding.cpp
    adblock/adblockhostmatcher.cpp
    adblock/adblockmanager.cpp
    adblock/adblockrule.cpp
    adblock/adblockrulefallbackimpl.cpp
    adblock/adblockrulenullimpl.cpp
    adblock/adblockruletextmatchimpl.cpp
    adblock/adblocksettingwidget.cpp
    #----------------------------------------
    bookmarks/bookmarkmanager.cpp
    bookmarks/bookmarkscontextmenu.cpp
    bookmarks/bookmarksmenu.cpp
    bookmarks/bookmarkstoolbar.cpp
    bookmarks/bookmarkstreemodel.cpp
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
    panels/bookmarkspanel.cpp
    panels/historypanel.cpp
    panels/paneltreeview.cpp
    panels/urlfilterproxymodel.cpp
    panels/urlpanel.cpp
    #----------------------------------------
    rekonqpage/newtabpage.cpp
    rekonqpage/thumbupdater.cpp
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
    sync/sshsynchandler.cpp
    sync/syncgooglesettingswidget.cpp
    sync/syncoperasettingswidget.cpp
    sync/syncsshsettingswidget.cpp
    #----------------------------------------
    tabwindow/rekonqwindow.cpp
    tabwindow/tabbar.cpp
    tabwindow/tabhighlighteffect.cpp
    tabwindow/tabpreviewpopup.cpp
    tabwindow/tabwidget.cpp
    #----------------------------------------
    urlbar/urlbar.cpp
    urlbar/completionwidget.cpp
    urlbar/listitem.cpp
    urlbar/adblockwidget.cpp
    urlbar/bookmarkwidget.cpp
    urlbar/rsswidget.cpp
    urlbar/sslwidget.cpp
    urlbar/urlsuggester.cpp
    #----------------------------------------
    useragent/useragentdialog.cpp
    useragent/useragentinfo.cpp
    useragent/useragentmanager.cpp
    #----------------------------------------
    webtab/clicktoflash.cpp
    webtab/crashmessagebar.cpp
    webtab/networkaccessmanager.cpp
    webtab/previewselectorbar.cpp
    webtab/protocolhandler.cpp
    webtab/knetworkaccessmanager.cpp
    webtab/searchenginebar.cpp
    webtab/sslinfodialog.cpp
    webtab/walletbar.cpp
    webtab/webpage.cpp
    webtab/webpluginfactory.cpp
    webtab/websslinfo.cpp
    webtab/webview.cpp
    webtab/webtab.cpp
    #----------------------------------------
    webwindow/findbar.cpp
    webwindow/maintoolbar.cpp
    webwindow/rekonqfactory.cpp
    webwindow/rekonqmenu.cpp
    webwindow/webwindow.cpp
    #----------------------------------------
#    kspellplugin.cpp
#    ${wk_HEADERS_MOC}
)


# # Nepomuk OPTIONAL src files
# IF(NepomukCore_FOUND)
#     SET( rekonq_KDEINIT_SRCS
#             ${rekonq_KDEINIT_SRCS}
#             urlbar/resourcelinkdialog.cpp
#             urlbar/newresourcedialog.cpp
#             nepomuk/utils/simpleresourcemodel.cpp
#             nepomuk/utils/resourcemodel.cpp
#     )
# ENDIF(NepomukCore_FOUND)


# # Opera sync optional src files
# IF(HAVE_QCA2 AND HAVE_QTOAUTH)
#     SET( rekonq_KDEINIT_SRCS
#             ${rekonq_KDEINIT_SRCS}
#             sync/operasynchandler.cpp
#     )
# ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)

# ui files
QT5_WRAP_UI (rekonq_KDEINIT_SRCS
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
    sync/sync_ssh_settings.ui
    sync/sync_host_type.ui
    # ----------------------------------------
    useragent/useragentsettings.ui
    # ----------------------------------------
    webtab/sslinfo.ui
    # ----------------------------------------
    cleardata.ui
    session.ui
    webappcreation.ui
 )


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/adblock
                        ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
                        ${CMAKE_CURRENT_SOURCE_DIR}/download
                        ${CMAKE_CURRENT_SOURCE_DIR}/history
                        ${CMAKE_CURRENT_SOURCE_DIR}/icons
                        ${CMAKE_CURRENT_SOURCE_DIR}/panels
                        ${CMAKE_CURRENT_SOURCE_DIR}/rekonqpage
                        ${CMAKE_CURRENT_SOURCE_DIR}/settings
                        ${CMAKE_CURRENT_SOURCE_DIR}/sync
                        ${CMAKE_CURRENT_SOURCE_DIR}/urlbar
                        ${CMAKE_CURRENT_SOURCE_DIR}/tabwindow
                        ${CMAKE_CURRENT_SOURCE_DIR}/useragent
                        ${CMAKE_CURRENT_SOURCE_DIR}/webtab
                        ${CMAKE_CURRENT_SOURCE_DIR}/webwindow
                        ${CMAKE_CURRENT_BINARY_DIR}
)

# # Nepomuk optional include directories
# IF(SOPRANO_FOUND AND NepomukCore_FOUND)
#     INCLUDE_DIRECTORIES(${SOPRANO_INCLUDE_DIR}
#                         ${NEPOMUK_CORE_INCLUDE_DIR}
#     )
# 
#     FIND_PACKAGE(Soprano REQUIRED)
#     INCLUDE(SopranoAddOntology)
# ENDIF(SOPRANO_FOUND AND NepomukCore_FOUND)

# # Opera sync optional include directories
# IF(HAVE_QCA2 AND HAVE_QTOAUTH)
#     INCLUDE_DIRECTORIES(${QCA2_INCLUDE_DIR}
#                         ${QTOAUTH_INCLUDE_DIR}
#     )
# ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)


### -------------- ADDING DEFINITIONS...

#ADD_DEFINITIONS ( ${KDE4_DEFINITIONS} )


### -------------- ADDING APPLICATION ICON...

# TODO restore this...
# KDE4_ADD_APP_ICON( rekonq_KDEINIT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-rekonq.png )


### -------------- ADDING APPLICATION KCFG FILES...

KCONFIG_ADD_KCFG_FILES( rekonq_KDEINIT_SRCS rekonq.kcfgc )


### --------------- ADDING EXECUTABLE...

KF5_ADD_KDEINIT_EXECUTABLE (rekonq ${rekonq_KDEINIT_SRCS} main.cpp)


### --------------- TARGETTING LINK LIBRARIES...

TARGET_LINK_LIBRARIES (     kdeinit_rekonq
                            # ---------------------------------------
                            Qt5::Core
                            Qt5::Gui
                            Qt5::Widgets
                            Qt5::Network
                            Qt5::Script
                            Qt5::WebKitWidgets
                            Qt5::Concurrent
                            # ---------------------------------------
                            KF5::Auth
                            KF5::Bookmarks
                            KF5::Codecs
                            KF5::ConfigCore
                            KF5::ConfigWidgets
                            KF5::CoreAddons
                            KF5::DBusAddons
                            KF5::GlobalAccel
                            KF5::GuiAddons
                            KF5::I18n
                            KF5::IconThemes
                            KF5::JobWidgets
                            KF5::KCMUtils
                            KF5::Notifications
                            KF5::SonnetCore
                            KF5::TextWidgets
                            KF5::Wallet
                            KF5::WidgetsAddons
                            KF5::WindowSystem
                            KF5::XmlGui
                            # ---------------------------------------
                            KF5::Parts
                            KF5::KIOCore
                            KF5::Service
                            KF5::WebKit
)

# # Nepomuk optional target link libraries
# IF(SOPRANO_FOUND AND NepomukCore_FOUND)
#     TARGET_LINK_LIBRARIES(  kdeinit_rekonq
#                             ${NEPOMUK_CORE_LIBRARY}
#                             ${SOPRANO_LIBRARIES}
#     )
# ENDIF(SOPRANO_FOUND AND NepomukCore_FOUND)


# # Opera sync optional link libraries
# IF(HAVE_QCA2 AND HAVE_QTOAUTH)
#     TARGET_LINK_LIBRARIES(  kdeinit_rekonq
#                             ${QCA2_LIBRARY}
#                             ${QTOAUTH_LIBRARY}
#     )
# ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)


### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( TARGETS kdeinit_rekonq  ${INSTALL_TARGETS_DEFAULT_ARGS} )

INSTALL( FILES rekonq.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
INSTALL( FILES webwindow/rekonqui.rc  DESTINATION  ${DATA_INSTALL_DIR}/rekonq )

