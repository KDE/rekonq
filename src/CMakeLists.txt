### ------- sub dirs -------

ADD_SUBDIRECTORY( data )


### ------- SETTING REKONQ FILES..

set(rekonq_KDEINIT_SRCS
    #----------------------------------------
    application.cpp
    autosaver.cpp
    searchengine.cpp
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
    history/historymanager.cpp
    history/historymodels.cpp
    #----------------------------------------
    session/sessionmanager.cpp
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
    tabwindow/tabbar.cpp
    tabwindow/tabhighlighteffect.cpp
    tabwindow/tabpreviewpopup.cpp
    tabwindow/tabwindow.cpp
    #----------------------------------------
    webwindow/networkaccessmanager.cpp
    webwindow/webpage.cpp
    webwindow/webwindow.cpp
    #----------------------------------------
)


KDE4_ADD_UI_FILES( rekonq_KDEINIT_SRCS
    #----------------------------------------
    settings/settings_general.ui
    settings/settings_tabs.ui
    settings/settings_appearance.ui
    settings/settings_webkit.ui
    settings/settings_advanced.ui
    settings/settings_privacy.ui
    settings/password_exceptions.ui
 )


### ------------- INCLUDING DIRECTORIES...

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/adblock
                        ${CMAKE_CURRENT_SOURCE_DIR}/history
                        ${CMAKE_CURRENT_SOURCE_DIR}/session
                        ${CMAKE_CURRENT_SOURCE_DIR}/settings
                        ${CMAKE_CURRENT_SOURCE_DIR}/tabwindow
                        ${CMAKE_CURRENT_SOURCE_DIR}/webwindow
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)

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


### ------------ INSTALL FILES...

INSTALL( TARGETS rekonq ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( TARGETS kdeinit_rekonq  ${INSTALL_TARGETS_DEFAULT_ARGS} )
