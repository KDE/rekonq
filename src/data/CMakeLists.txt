# image files
INSTALL(
    FILES
    bg2.png
    tile.gif
    category.png
    button.png
    busywidget.gif
    loading.gif
    webkit-icon.png
    autoscroll.png
    background.png
    DESTINATION ${XDG_DATA_INSTALL_DIR}/rekonq/pics
)

# default bookmarks
INSTALL(
    FILES
    defaultbookmarks.xbel
    DESTINATION ${XDG_DATA_INSTALL_DIR}/rekonq
)

# .desktop file
INSTALL(
    PROGRAMS rekonq.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}/nient
)

# htmls
INSTALL(
    FILES
    rekonqinfo.html
    home.html
    jquery-1.7.2.min.js
    jquery-ui-1.8.20.custom.min.js
    DESTINATION ${XDG_DATA_INSTALL_DIR}/rekonq/htmls
)

# adblock default settings
INSTALL(
    FILES
    adblockrc
    DESTINATION ${XDG_DATA_INSTALL_DIR}/rekonq
)

