# Andrea Diamantini - adjam7 at gmail dot com
# rekonq project

PROJECT( rekonq )

# ===============================================================================================
# Informations to update before to release this package.

# rekonq version
SET(REKONQ_MAJOR_VERSION "0")
SET(REKONQ_MINOR_VERSION "0")
SET(REKONQ_PATCH_VERSION "1")

SET(REKONQ_VERSION_STRING
    "${REKONQ_MAJOR_VERSION}.${REKONQ_MINOR_VERSION}.${REKONQ_PATCH_VERSION}"
   )

# =================================================================================================
# minimum cmake required

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

# =================================================================================================


SET( QT_MIN_VERSION 4.4.0)
FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)

# ==================================================================================================
# Log messages

MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS " rekonq ${REKONQ_VERSION_STRING} dependencies results  <http://rekonq.adjam.org>")
MESSAGE(STATUS "")

# Require shared libraries results.

IF(QT_FOUND)
    MESSAGE(STATUS " qt library found............... YES")
ELSE(QT_FOUND)
    MESSAGE(STATUS " qt library found............... NO")
    MESSAGE(STATUS "")
    MESSAGE(SEND_ERROR " rekonq needs at least Qt 4.4.0. Please install it and try compiling again.")
    MESSAGE(STATUS " Qt website is at http://www.trolltech.com")
    MESSAGE(STATUS "")
ENDIF(QT_FOUND)

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "")

# Optional plugins results.

IF(QT_FOUND)
    MESSAGE(STATUS " rekonq will be compiled....... YES")
    SET(REKONQ_CAN_BE_COMPILED true)
ELSE(QT_FOUND)
    MESSAGE(FATAL_ERROR " rekonq will not be compiled!")
    SET(REKONQ_CAN_BE_COMPILED false)
ENDIF(QT_FOUND)

MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "")

# ==================================================================================================

IF(REKONQ_CAN_BE_COMPILED)

    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KDE4_INCLUDES}
        ${QT4_INCLUDES}
        ${QT_QTNETWORK_INCLUDE_DIR}
        ${QT_QTWEBKIT_INCLUDE_DIR}
    )

    ADD_SUBDIRECTORY( src )
    ADD_SUBDIRECTORY( icons )
    ADD_SUBDIRECTORY( data )
    ADD_SUBDIRECTORY( htmls )

ENDIF(REKONQ_CAN_BE_COMPILED)

# =====================================================================================================