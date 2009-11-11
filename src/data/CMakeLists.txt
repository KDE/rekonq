INSTALL( 
    FILES bg2.png bg.png bot.gif busywidget.gif closed.png loading.mng open.png tile.gif top.png webkit-icon.png category.png button.png
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
