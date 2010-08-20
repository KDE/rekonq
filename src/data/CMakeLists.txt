# image files
INSTALL( 
    FILES 
    bg2.png bg.png tile.gif category.png button.png
    busywidget.gif loading.mng  
    webkit-icon.png
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/pics
)

# default bookmarks
INSTALL( 
    FILES 
    defaultbookmarks.xbel
    DESTINATION ${DATA_INSTALL_DIR}/rekonq
)

# .desktop file
INSTALL( 
    FILES rekonq.desktop 
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)

# htmls
INSTALL( 
    FILES 
    rekonqinfo.html 
    home.html
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/htmls
)

# opensearch engines
INSTALL( 
    FILES 
    google.xml
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/opensearch
)
