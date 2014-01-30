# Andrea Diamantini - adjam7 at gmail dot com
# rekonq project

# ==================================================================================
# minimum cmake required

CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12 FATAL_ERROR)

# ==================================================================================
# project name

PROJECT (rekonq)

# ==================================================================================
# Information to update before to release this package.

# rekonq info
SET (REKONQ_VERSION "3.X.Y")

CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/config-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-version.h)

SET (REKONQ_SITE "http://rekonq.kde.org")

# ==================================================================================
# Extra CMake Modules

FIND_PACKAGE (ECM 0.0.9 REQUIRED NO_MODULE)

SET (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})


INCLUDE (ECMOptionalAddSubdirectory) 
INCLUDE (ECMInstallIcons)
INCLUDE (ECMSetupVersion)
INCLUDE (ECMGenerateHeaders)
INCLUDE (GenerateExportHeader)

INCLUDE (CMakePackageConfigHelpers)
INCLUDE (FeatureSummary)
INCLUDE (WriteBasicConfigVersionFile)
INCLUDE (CheckFunctionExists)

INCLUDE (KDEInstallDirs)
INCLUDE (KDECMakeSettings)
INCLUDE (KDECompilerSettings)
INCLUDE (KDEFrameworkCompilerSettings)
>>>>>>> KF5 rekonq, first commit!!!!!!!


# ==================================================================================
# Qt 5
    
SET (QT_MIN_VERSION "5.2.0")

FIND_PACKAGE(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Gui Widgets Network Script WebKitWidgets)

# ==================================================================================
# KDE Frameworks

SET (KF5_MIN_VERSION "5.0.0")

FIND_PACKAGE (KF5 CONFIG REQUIRED
    Auth
    Bookmarks
    Config
    ConfigWidgets
    CoreAddons
    DBusAddons
    GlobalAccel
    GuiAddons
    I18n
    Init
    JobWidgets
    KCMUtils
    Solid
    TextWidgets
    Wallet
    WidgetsAddons
    WindowSystem
    XmlGui
    # -------------------------
    Parts
    KIO
    Service
    WebKit
)

SET(KF5_VERSION_STRING
    "${KF5_VERSION_MAJOR}.${KF5_VERSION_MINOR}.${KF5_VERSION_PATCH}"
   )


# ==================================================================================
# optional nepomuk requirements

# we HAVE TO move to Baloo here...
#if(WITH_NEPOMUK)
#  FIND_PACKAGE(Soprano)
#
#  FIND_PACKAGE(NepomukCore 0.1.0)
#endif()
#
#IF(SOPRANO_FOUND AND NepomukCore_FOUND)
#    ADD_DEFINITIONS(-DHAVE_NEPOMUK)
# ENDIF(SOPRANO_FOUND AND NepomukCore_FOUND)

# ==================================================================================
# optional QCA2 & QtOAuth requirements

# MACRO_OPTIONAL_FIND_PACKAGE(QCA2)
# MACRO_BOOL_TO_01(QCA2_FOUND HAVE_QCA2)
# 
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-qca2.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-qca2.h )
# 
# 
# MACRO_OPTIONAL_FIND_PACKAGE(QtOAuth)
# MACRO_BOOL_TO_01(QTOAUTH_FOUND HAVE_QTOAUTH)
# 
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-qtoauth.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-qtoauth.h )


# ==================================================================================
# Log messages

MESSAGE(STATUS "")
MESSAGE(STATUS "-----------------------------------------------------------------------")
MESSAGE(STATUS " rekonq ${REKONQ_VERSION} dependencies results  <${REKONQ_SITE}>")
MESSAGE(STATUS "")

# Require shared libraries results.

# ----------------------------------- Qt -------------------------------------

IF(Qt5Core_FOUND)
    MESSAGE(STATUS " Qt library found...................... YES")
    MESSAGE(STATUS " Qt version ${Qt5Core_VERSION_STRING} found! ")
ELSE(Qt5Core_FOUND)
    MESSAGE(STATUS " Qt library found...................... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " rekonq ${REKONQ_VERSION} needs at least Qt ${QT_MIN_VERSION}. Please install it and try compiling again.")
    MESSAGE(STATUS " Qt website is at http://qt-project.org")
    MESSAGE(STATUS "")
ENDIF(Qt5Core_FOUND)

MESSAGE(STATUS "")

# ---------------------------------- KDE -------------------------------------

IF(KF5_FOUND)
    MESSAGE(STATUS " KDE Desktop Environment............... YES")
    MESSAGE(STATUS " KDE version ${KF5_VERSION_STRING} found! ")
ELSE(KF5_FOUND)
    MESSAGE(STATUS " KDE Desktop Environment............... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " rekonq ${REKONQ_VERSION} needs at least KDE ${KF5_MIN_VERSION}. Please install it and try compiling again.")
    MESSAGE(STATUS " KDE website is at http://www.kde.org")
    MESSAGE(STATUS "")
ENDIF(KF5_FOUND)

##### FINAL RESULTS #####

# create suspance..
MESSAGE(STATUS "")
MESSAGE(STATUS "")

IF(Qt5Core_FOUND AND KF5_FOUND)
    MESSAGE(STATUS " rekonq will be compiled............... YES")
    SET(REKONQ_CAN_BE_COMPILED true)
ELSE(Qt5Core_FOUND AND KF5_FOUND)
    MESSAGE(FATAL_ERROR " rekonq will NOT be compiled!")
    SET(REKONQ_CAN_BE_COMPILED false)
ENDIF(Qt5Core_FOUND AND KF5_FOUND)

# ----------------------- Optional Deps --------------------------------------

IF(REKONQ_CAN_BE_COMPILED)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " ----- Optional Features")
    MESSAGE(STATUS "")

    # Nepomuk

#    IF(SOPRANO_FOUND AND NepomukCore_FOUND)
#        MESSAGE(STATUS " Nepomuk Libraries..................... YES")
#        MESSAGE(STATUS " Rekonq will be compiled with support for bookmarks nepomuk tagging")
#    ELSE(SOPRANO_FOUND AND NepomukCore_FOUND)
#        MESSAGE(STATUS " Nepomuk Libraries..................... NO")
#        MESSAGE(STATUS " Rekonq will be compiled WITHOUT support for bookmarks tagging")
#    ENDIF(SOPRANO_FOUND AND NepomukCore_FOUND)

    MESSAGE(STATUS "")

#     # QCA2
# 
#     IF(HAVE_QCA2)
#         MESSAGE(STATUS " QCA2 Libraries........................ YES")
#     ELSE(HAVE_QCA2)
#         MESSAGE(STATUS " QCA2 Libraries........................ NO")
#     ENDIF(HAVE_QCA2)
# 
#     # QTOAUTH
# 
#     IF(HAVE_QTOAUTH)
#         MESSAGE(STATUS " QtOAuth Libraries..................... YES")
#     ELSE(HAVE_QTOAUTH)
#         MESSAGE(STATUS " QtOAuth Libraries..................... NO")
#     ENDIF(HAVE_QTOAUTH)
# 
# 
#     IF(HAVE_QCA2 AND HAVE_QTOAUTH)
#         MESSAGE(STATUS " Rekonq will be compiled with support for opera sync handler")
#     ELSE(HAVE_QCA2 AND HAVE_QTOAUTH)
#         MESSAGE(STATUS " Rekonq will be compiled WITHOUT support for opera sync handler")
#     ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)

    MESSAGE(STATUS "")
    MESSAGE(STATUS "-----------------------------------------------------------------------")
    MESSAGE(STATUS "")

ENDIF(REKONQ_CAN_BE_COMPILED)

# ===============================================================================

IF(REKONQ_CAN_BE_COMPILED)

    ADD_SUBDIRECTORY (src)
    ADD_SUBDIRECTORY (icons)

ENDIF(REKONQ_CAN_BE_COMPILED)

# ================================================================================

# doc & translations
SET_PROPERTY (GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

# Do not remove or modify these.
# The release script substitutes in for these comments with appropriate doc and translation directories.
#PO_SUBDIR
#DOC_SUBDIR
