##### ---------- General Settings ----------

SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_BINARY_DIR}/..
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

############################################################
