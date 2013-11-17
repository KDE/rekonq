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
    PROGRAMS rekonq.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)

# htmls
INSTALL(
    FILES
    rekonqinfo.html
    home.html
    jquery-1.7.2.min.js
    jquery-ui-1.8.20.custom.min.js
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/htmls
)

# adblock default settings
INSTALL(
    FILES
    adblockrc
    DESTINATION ${DATA_INSTALL_DIR}/rekonq
)

# opensearch engines
INSTALL(
    FILES
    beolingus.xml
    bing.xml
    yahoo.xml
    de2en.xml
    dictfr.xml
    facebook.xml
    google.xml
    kde_techbase.xml
    kde_userbase.xml
    youtube.xml
    voila.xml
    wikia.xml
    wikipedia.xml
    wiktionary.xml
    wr_english.xml
    db_opensearch.json
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/opensearch
)
