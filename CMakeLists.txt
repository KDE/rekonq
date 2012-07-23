# Andrea Diamantini - adjam7 at gmail dot com
# rekonq project

PROJECT( rekonq )

# ==================================================================================
# Information to update before to release this package.

# rekonq info
SET(REKONQ_VERSION "1.50" )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-version.h )

SET(REKONQ_SITE "http://rekonq.kde.org")

# ==================================================================================
# minimum cmake required

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4)


SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# ==================================================================================


FIND_PACKAGE(Qt4 4.8.0 COMPONENTS QtCore QtGui QtNetwork QtWebKit REQUIRED)

## Ensure we have QtWebKit >= 2.2.0
#FIND_PACKAGE(PkgConfig REQUIRED)
#PKG_CHECK_MODULES(PC_QTWEBKIT REQUIRED QtWebKit>=4.8.0)
#IF(NOT PC_QTWEBKIT_FOUND)
#  MESSAGE(FATAL_ERROR "QtWebkit 2.2.0 is required for this version of Rekonq")
#ELSE()
#  MESSAGE(STATUS "Found QtWebKit ${PC_QTWEBKIT_VERSION}")
#ENDIF()

FIND_PACKAGE(KDE4 4.7.0 REQUIRED)


INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)

ADD_DEFINITIONS(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)


# ==================================================================================
# optional nepomuk requirements

MACRO_OPTIONAL_FIND_PACKAGE(Nepomuk)
MACRO_BOOL_TO_01(Nepomuk_FOUND HAVE_NEPOMUK)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-nepomuk.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-nepomuk.h )


# ==================================================================================
# optional QCA2 & QtOAuth requirements

MACRO_OPTIONAL_FIND_PACKAGE(QCA2)
MACRO_BOOL_TO_01(QCA2_FOUND HAVE_QCA2)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-qca2.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-qca2.h )


MACRO_OPTIONAL_FIND_PACKAGE(QtOAuth)
MACRO_BOOL_TO_01(QTOAUTH_FOUND HAVE_QTOAUTH)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-qtoauth.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-qtoauth.h )


# ==================================================================================
# Log messages

MESSAGE(STATUS "")
MESSAGE(STATUS "-----------------------------------------------------------------------")
MESSAGE(STATUS " rekonq ${REKONQ_VERSION} dependencies results  <${REKONQ_SITE}>")
MESSAGE(STATUS "")

# Require shared libraries results.

# ----------------------------------- Qt -------------------------------------
SET(QT_VERS_STR
    "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}"
   )

IF(QT4_FOUND)
    MESSAGE(STATUS " Qt library found...................... YES")
    MESSAGE(STATUS " Qt version ${QT_VERS_STR} found! ")
ELSE(QT4_FOUND)
    MESSAGE(STATUS " Qt library found...................... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " rekonq needs at least Qt ${QT_MIN_VERSION}. Please install it and try compiling again.")
    MESSAGE(STATUS " Qt website is at http://qt.nokia.com")
    MESSAGE(STATUS "")
ENDIF(QT4_FOUND)

MESSAGE(STATUS "")

# ---------------------------------- KDE -------------------------------------
SET(KDE_VERS_STR
    "${KDE_VERSION_MAJOR}.${KDE_VERSION_MINOR}.${KDE_VERSION_RELEASE}"
   )

IF(KDE4_FOUND)
    MESSAGE(STATUS " KDE Desktop Environment............... YES")
    MESSAGE(STATUS " KDE version ${KDE_VERS_STR} found! ")
ELSE(KDE4_FOUND)
    MESSAGE(STATUS " KDE Desktop Environment............... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " rekonq needs at least KDE ${KDE_MIN_VERSION}. Please install it and try compiling again.")
    MESSAGE(STATUS " KDE website is at http://www.kde.org")
    MESSAGE(STATUS "")
ENDIF(KDE4_FOUND)

##### FINAL RESULTS #####

# create suspance..
MESSAGE(STATUS "")
MESSAGE(STATUS "")

IF(QT4_FOUND AND KDE4_FOUND)
    MESSAGE(STATUS " rekonq will be compiled............... YES")
    SET(REKONQ_CAN_BE_COMPILED true)
ELSE(QT4_FOUND AND KDE4_FOUND)
    MESSAGE(FATAL_ERROR " rekonq will NOT be compiled!")
    SET(REKONQ_CAN_BE_COMPILED false)
ENDIF(QT4_FOUND AND KDE4_FOUND)

# ----------------------- Optional Deps --------------------------------------

IF(REKONQ_CAN_BE_COMPILED)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " ----- Optional Features")
    MESSAGE(STATUS "")

    # Nepomuk

    IF(HAVE_NEPOMUK)
        MESSAGE(STATUS " Nepomuk Libraries..................... YES")
        MESSAGE(STATUS " Rekonq will be compiled with support for bookmarks nepomuk tagging")
    ELSE(HAVE_NEPOMUK)
        MESSAGE(STATUS " Nepomuk Libraries..................... NO")
        MESSAGE(STATUS " Rekonq will be compiled WITHOUT support for bookmarks tagging")
    ENDIF(HAVE_NEPOMUK)

    MESSAGE(STATUS "")

    # QCA2

    IF(HAVE_QCA2)
        MESSAGE(STATUS " QCA2 Libraries........................ YES")
    ELSE(HAVE_QCA2)
        MESSAGE(STATUS " QCA2 Libraries........................ NO")
    ENDIF(HAVE_QCA2)

    # QTOAUTH

    IF(HAVE_QTOAUTH)
        MESSAGE(STATUS " QtOAuth Libraries..................... YES")
    ELSE(HAVE_QTOAUTH)
        MESSAGE(STATUS " QtOAuth Libraries..................... NO")
    ENDIF(HAVE_QTOAUTH)


    IF(HAVE_QCA2 AND HAVE_QTOAUTH)
        MESSAGE(STATUS " Rekonq will be compiled with support for opera sync handler")
    ELSE(HAVE_QCA2 AND HAVE_QTOAUTH)
        MESSAGE(STATUS " Rekonq will be compiled WITHOUT support for opera sync handler")
    ENDIF(HAVE_QCA2 AND HAVE_QTOAUTH)


    MESSAGE(STATUS "")
    MESSAGE(STATUS "-----------------------------------------------------------------------")
    MESSAGE(STATUS "")

ENDIF(REKONQ_CAN_BE_COMPILED)

# ===============================================================================

IF(REKONQ_CAN_BE_COMPILED)

    ADD_SUBDIRECTORY( src )
    ADD_SUBDIRECTORY( icons )
    ADD_SUBDIRECTORY( doc )

ENDIF(REKONQ_CAN_BE_COMPILED)

# ================================================================================

# doc & translations
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

# Do not remove or modify these.
# The release script substitutes in for these comments with appropriate doc and translation directories.
#PO_SUBDIR
#DOC_SUBDIR
