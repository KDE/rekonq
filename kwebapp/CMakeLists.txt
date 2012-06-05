set( kwebapp_SRCS
    rekonqview.cpp
    searchengine.cpp
    walletbar.cpp
    webview.cpp
    webpage.cpp
    websnap.cpp
    kwebmain.cpp
 )

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT4_INCLUDES}
    ${KDE4_INCLUDES}
)


kde4_add_executable(kwebapp ${kwebapp_SRCS})

target_link_libraries(kwebapp
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
)


install(TARGETS kwebapp ${INSTALL_TARGETS_DEFAULT_ARGS} )

