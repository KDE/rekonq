INSTALL( 
    FILES 
    bg2.png bg.png tile.gif category.png button.png
    busywidget.gif loading.mng  
    webkit-icon.png
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/pics
)

INSTALL( 
    FILES 
    defaultbookmarks.xbel
    default.css
    DESTINATION ${DATA_INSTALL_DIR}/rekonq
)

INSTALL( 
    FILES rekonq.desktop 
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)

INSTALL( 
    FILES 
    rekonqinfo.html 
    home.html
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/htmls
)
