# Andrea Diamantini - adjam7 at gmail dot com
# rekonq project

PROJECT( rekonq )

# ===============================================================================================
# Informations to update before to release this package.

# rekonq info
SET(REKONQ_MAJOR_VERSION "0")
SET(REKONQ_MINOR_VERSION "1")
SET(REKONQ_PATCH_VERSION "9")

SET(REKONQ_VERSION_STR
    "${REKONQ_MAJOR_VERSION}.${REKONQ_MINOR_VERSION}.${REKONQ_PATCH_VERSION}"
   )

SET(REKONQ_SITE "http://rekonq.sourceforge.net")

# =================================================================================================
# minimum cmake required

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

# =================================================================================================


SET(QT_MIN_VERSION 4.5.0)
FIND_PACKAGE(Qt4 REQUIRED)
SET(KDE_MIN_VERSION 4.2.0)
FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(MacroOptionalFindPackage)
INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)

# =================================================================================================
# KDE Version

if(${KDE_VERSION_MINOR} GREATER "2")
    SET(KDE_OLD_VERSION false)
else(${KDE_VERSION_MINOR} GREATER "2")
    if(${KDE_VERSION_RELEASE} GREATER "69")
        SET(KDE_OLD_VERSION false)
    else(${KDE_VERSION_RELEASE} GREATER "69")
        SET(KDE_OLD_VERSION true)
    endif(${KDE_VERSION_RELEASE} GREATER "69")
endif(${KDE_VERSION_MINOR} GREATER "2")
# =================================================================================================

# --------------- KNetworkAccessManager
if(KDE_OLD_VERSION)

    find_package(KdeNetwork REQUIRED)

    MESSAGE(STATUS "----------------------------------------------------------------------------------")
    MESSAGE(STATUS "")
    MESSAGE(STATUS " As KDE VERSION is minor than 4.2.70, rekonq will use KNetworkAccessManager ")
    MESSAGE(STATUS " classes, instead of newly KIO::AccessManager. ")
    MESSAGE(STATUS "")

endif(KDE_OLD_VERSION)



# ==================================================================================================
# Log messages

MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS " rekonq ${REKONQ_VERSION_STR} dependencies results  <${REKONQ_SITE}>")
MESSAGE(STATUS "")

# Require shared libraries results.

# ----------------- Qt
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

MESSAGE(STATUS "") # -----------------------------------------------------------

# -------------- KDE
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


# --------------- KNetworkAccessManager
if(KDE_OLD_VERSION)

    MESSAGE(STATUS "")
    IF(KDENETWORK_FOUND)
        MESSAGE(STATUS " KDE Network library found............. YES")
    ELSE(KDENETWORK_FOUND)
        MESSAGE(STATUS " KDE Network library found............. NO")
        MESSAGE(STATUS "")
        MESSAGE(SEND_ERROR " rekonq needs, from version 0.2, KDENETWORK if compiled with a KDE version minor than 4.2.69 ")
        MESSAGE(STATUS " You can actually retrieve it from playground/libs/webkitkde/kdenetwork KDE svn")
        MESSAGE(STATUS "")
    ENDIF(KDENETWORK_FOUND)

endif(KDE_OLD_VERSION)

MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "")


##### FINAL RESULTS #####

if(KDE_OLD_VERSION)

    IF(QT_FOUND AND KDE4_FOUND AND KDENETWORK_FOUND)
        MESSAGE(STATUS " rekonq will be compiled............... YES")
        SET(REKONQ_CAN_BE_COMPILED true)
    ELSE(QT_FOUND AND KDE4_FOUND AND KDENETWORK_FOUND)
        MESSAGE(FATAL_ERROR " rekonq will NOT be compiled!")
        SET(REKONQ_CAN_BE_COMPILED false)
    ENDIF(QT_FOUND AND KDE4_FOUND AND KDENETWORK_FOUND)

else(KDE_OLD_VERSION)

    IF(QT_FOUND AND KDE4_FOUND)
        MESSAGE(STATUS " rekonq will be compiled............... YES")
        SET(REKONQ_CAN_BE_COMPILED true)
    ELSE(QT_FOUND AND KDE4_FOUND)
        MESSAGE(FATAL_ERROR " rekonq will NOT be compiled!")
        SET(REKONQ_CAN_BE_COMPILED false)
    ENDIF(QT_FOUND AND KDE4_FOUND)

endif(KDE_OLD_VERSION)


MESSAGE(STATUS "")
MESSAGE(STATUS "----------------------------------------------------------------------------------")
MESSAGE(STATUS "")

# ==================================================================================================

IF(REKONQ_CAN_BE_COMPILED)

    ADD_SUBDIRECTORY( src )
    ADD_SUBDIRECTORY( icons )
    ADD_SUBDIRECTORY( data )
#    ADD_SUBDIRECTORY( i18n )
#    ADD_SUBDIRECTORY( doc )

ENDIF(REKONQ_CAN_BE_COMPILED)

# =====================================================================================================
