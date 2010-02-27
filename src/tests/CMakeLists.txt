##### ---------- General Settings ----------

SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_BINARY_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/..
                        ${CMAKE_CURRENT_SOURCE_DIR}/../adblock
                        ${CMAKE_CURRENT_SOURCE_DIR}/../bookmarks
                        ${CMAKE_CURRENT_SOURCE_DIR}/../history
                        ${CMAKE_CURRENT_SOURCE_DIR}/../rekonqpage
                        ${CMAKE_CURRENT_SOURCE_DIR}/../settings
                        ${CMAKE_CURRENT_SOURCE_DIR}/../urlbar
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)

##### ------------- tabbar test

kde4_add_unit_test( tabbar_test tabbar_test.cpp )

target_link_libraries( tabbar_test
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

##### ------------- webpage test

kde4_add_unit_test( webpage_test webpage_test.cpp )

target_link_libraries( webpage_test
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

############################################################
