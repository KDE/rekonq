########### install files ###############
#
cmake_minimum_required(VERSION 2.6)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR rekonq)
