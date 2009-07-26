INSTALL( 
    FILES loading.mng
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/pics
)

INSTALL( 
    FILES defaultbookmarks.xbel
    DESTINATION ${DATA_INSTALL_DIR}/rekonq
)

INSTALL( 
    FILES rekonq.desktop 
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)

INSTALL( 
    FILES notfound.html
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/htmls
)

INSTALL( 
    FILES webkit-icon.png
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/pics
)
