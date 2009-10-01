# Andrea Diamantini - adjam7 at gmail dot com
# rekonq project

PROJECT( rekonq )

# ==================================================================================
# Informations to update before to release this package.

# rekonq info
SET(REKONQ_MAJOR_VERSION "0")
SET(REKONQ_MINOR_VERSION "2")
SET(REKONQ_PATCH_VERSION "62")

SET(REKONQ_VERSION_STR
    "${REKONQ_MAJOR_VERSION}.${REKONQ_MINOR_VERSION}.${REKONQ_PATCH_VERSION}"
   )

SET(REKONQ_SITE "http://rekonq.sourceforge.net")

# ==================================================================================
# minimum cmake required

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

# ==================================================================================


SET(QT_MIN_VERSION 4.5.0)
FIND_PACKAGE(Qt4 REQUIRED)
SET(KDE_MIN_VERSION 4.3.1)
FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)

# ==================================================================================
# Log messages

MESSAGE(STATUS "")
MESSAGE(STATUS "-----------------------------------------------------------------------")
MESSAGE(STATUS " rekonq ${REKONQ_VERSION_STR} dependencies results  <${REKONQ_SITE}>")
MESSAGE(STATUS "")

# Require shared libraries results.

# ----------------------------------- Qt -------------------------------------
SET(QT_VERS_STR
    "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}"
   )

IF(QT_FOUND)
    MESSAGE(STATUS " Qt library found...................... YES")
    MESSAGE(STATUS " Qt version ${QT_VERS_STR} found! ")
ELSE(QT_FOUND)
    MESSAGE(STATUS " Qt library found...................... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " rekonq needs at least Qt ${QT_MIN_VERSION}. Please install it and try compiling again.")
    MESSAGE(STATUS " Qt website is at http://www.qtsoftware.com")
    MESSAGE(STATUS "")
ENDIF(QT_FOUND)

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

IF(QT_FOUND AND KDE4_FOUND)
    MESSAGE(STATUS " rekonq will be compiled............... YES")
    SET(REKONQ_CAN_BE_COMPILED true)
ELSE(QT_FOUND AND KDE4_FOUND)
    MESSAGE(FATAL_ERROR " rekonq will NOT be compiled!")
    SET(REKONQ_CAN_BE_COMPILED false)
ENDIF(QT_FOUND AND KDE4_FOUND)

MESSAGE(STATUS "")
MESSAGE(STATUS "-----------------------------------------------------------------------")
MESSAGE(STATUS "")

# ===============================================================================

IF(REKONQ_CAN_BE_COMPILED)

    ADD_SUBDIRECTORY( src )
    ADD_SUBDIRECTORY( icons )
    ADD_SUBDIRECTORY( data )

ENDIF(REKONQ_CAN_BE_COMPILED)

# ================================================================================
