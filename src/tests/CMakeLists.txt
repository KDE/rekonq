##### ---------- General Settings ----------

SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_BINARY_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/..
                        ${CMAKE_CURRENT_SOURCE_DIR}/../adblock
                        ${CMAKE_CURRENT_SOURCE_DIR}/../bookmarks
                        ${CMAKE_CURRENT_SOURCE_DIR}/../history
                        ${CMAKE_CURRENT_SOURCE_DIR}/../settings
                        ${CMAKE_CURRENT_SOURCE_DIR}/../urlbar
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)

##### ------------- findbar test

kde4_add_unit_test( findbar_test findbar_test.cpp )

target_link_libraries( findbar_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- mainwindow test

kde4_add_unit_test( mainwindow_test mainwindow_test.cpp )
 
target_link_libraries( mainwindow_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- mainview test

kde4_add_unit_test( mainview_test mainview_test.cpp )
 
target_link_libraries( mainview_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- networkaccessmanager test

kde4_add_unit_test( networkaccessmanager_test networkaccessmanager_test.cpp )
 
target_link_libraries( networkaccessmanager_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- protocolhandler test

kde4_add_unit_test( protocolhandler_test protocolhandler_test.cpp )
 
target_link_libraries( protocolhandler_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- sessionmanager test

kde4_add_unit_test( sessionmanager_test sessionmanager_test.cpp )
 
target_link_libraries( sessionmanager_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- tabbar test

kde4_add_unit_test( tabbar_test tabbar_test.cpp )

target_link_libraries( tabbar_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- walletbar test

kde4_add_unit_test( walletbar_test walletbar_test.cpp )

target_link_libraries( walletbar_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- webpage test

kde4_add_unit_test( webpage_test webpage_test.cpp )

target_link_libraries( webpage_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- websnap test

kde4_add_unit_test( websnap_test websnap_test.cpp )

target_link_libraries( websnap_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- webtab test

kde4_add_unit_test( webtab_test webtab_test.cpp )

target_link_libraries( webtab_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- webview test

kde4_add_unit_test( webview_test webview_test.cpp )

target_link_libraries( webview_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTTEST_LIBRARY}
)

##### ------------- urlbar test

kde4_add_unit_test( urlbar_test urlbar_test.cpp )

target_link_libraries( urlbar_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTTEST_LIBRARY}
)

############################################################
