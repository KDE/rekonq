INSTALL( 
    FILES closed.png loading.mng open.png webkit-icon.png busywidget.gif
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
    FILES notfound.html home.html
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/htmls
)
