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
    default_rekonq.css
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
    DESTINATION ${DATA_INSTALL_DIR}/rekonq/htmls
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
