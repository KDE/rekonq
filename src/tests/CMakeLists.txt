### ---------- General Settings -------

SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES (   ${CMAKE_CURRENT_BINARY_DIR}/..
                        ${KDE4_INCLUDES}
                        ${QT4_INCLUDES}
)

##### -------------tabbar test

kde4_add_unit_test( tabbar_test TEST tabbar_test.cpp )
 
target_link_libraries( tabbar_test
    kdeinit_rekonq
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTTEST_LIBRARY}
)

# --------------
