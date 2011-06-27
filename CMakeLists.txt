# Andrea Diamantini - adjam7 at gmail dot com
# rekonq project

PROJECT( rekonq )

# ==================================================================================
# Informations to update before to release this package.

# rekonq info
SET(REKONQ_VERSION "0.7.55" )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h )

SET(REKONQ_SITE "http://rekonq.kde.org")

# ==================================================================================
# minimum cmake required

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4)

# ==================================================================================


FIND_PACKAGE(Qt4 4.7.0 COMPONENTS QtCore QtGui QtNetwork QtWebKit REQUIRED)
FIND_PACKAGE(KDE4 4.6.90 REQUIRED)

INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
ADD_DEFINITIONS(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)


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

MESSAGE(STATUS "")
MESSAGE(STATUS "-----------------------------------------------------------------------")
MESSAGE(STATUS "")

# ===============================================================================

IF(REKONQ_CAN_BE_COMPILED)

    ADD_SUBDIRECTORY( src )
    ADD_SUBDIRECTORY( icons )
    ADD_SUBDIRECTORY( docs )
#    ADD_SUBDIRECTORY( i18n )

ENDIF(REKONQ_CAN_BE_COMPILED)

# ================================================================================
