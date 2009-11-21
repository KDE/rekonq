
ADD_DEFINITIONS ( ${KDE4_DEFINITIONS} )

SET(kcm_webkitadblock_SRCS
    kcmwebkitadblock.cpp
)

kde4_add_ui_files(kcm_webkitadblock_SRCS
    webkitadblock.ui
)

kde4_add_plugin(kcm_webkitadblock ${kcm_webkitadblock_SRCS})

target_link_libraries(kcm_webkitadblock 
                        ${KDE4_KDEUI_LIBS}
                        ${KDE4_KIO_LIBS}
)

install(TARGETS kcm_webkitadblock  DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES webkitAdblock.desktop DESTINATION  ${SERVICES_INSTALL_DIR} )
