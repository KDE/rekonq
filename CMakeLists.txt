# Andrea Diamantini - adjam7 at gmail dot com
# rekonq project

# ==================================================================================
# minimum cmake required

CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12 FATAL_ERROR)


# ==================================================================================
# project name

PROJECT (rekonq)

# rekonq info
SET (REKONQ_VERSION "3.X.Y")

CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/config-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-version.h)


# ==================================================================================
# Extra CMake Modules

FIND_PACKAGE (ECM 1.0.0 REQUIRED NO_MODULE)
SET (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

INCLUDE(ECMInstallIcons)
INCLUDE(FeatureSummary)

INCLUDE(KDEInstallDirs)
INCLUDE(KDECMakeSettings)
INCLUDE(KDECompilerSettings)



# ==================================================================================
# Qt 5
    
SET (QT_MIN_VERSION "5.3.0")

FIND_PACKAGE(Qt5 ${QT_MIN_VERSION} REQUIRED
    Core
    Gui
    Widgets
    Network
    Script
    WebKitWidgets
    Concurrent
    PrintSupport
)


# ==================================================================================
# KDE Frameworks

SET (KF5_MIN_VERSION "5.0.0")

# FIXME: Min version check
FIND_PACKAGE (KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Bookmarks
    DBusAddons
    GlobalAccel
    I18n
    IconThemes
    Init
    ItemViews
    KCMUtils
    Notifications
    Sonnet
    Wallet
    WindowSystem
    XmlGui
    # -------------------------
    Parts
    KIO
    Service
    WebKit
)


# ==================================================================================
# Adding subdirectories

ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (icons)


# ==================================================================================
# Log messages

FEATURE_SUMMARY(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)


# ================================================================================
# doc & translations
SET_PROPERTY (GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

# Do not remove or modify these.
# The release script substitutes in for these comments with appropriate doc and translation directories.
#PO_SUBDIR
#DOC_SUBDIR
