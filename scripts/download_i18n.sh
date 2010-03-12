#!/bin/sh
# use this stupid script to just prepare rekonq
# dir with translations.
#
# 1. Update the lists of the ready (about 80%) translations
# check the situation here: http://l10n.kde.org/stats/gui/trunk-kde4/po/rekonq.po/
LIST="pt_BR en_GB ca zh_CN cs da nl fr gl de hu it nds pl pt ru sr es sv tr uk"

# 2. run this script. It will create an i18n dir in rekonq sources ($RK_SRCS variable, set it to your source path) 
# dir with all the listed translations (eg: italian translation = rekonq_it.po file) 
# plus the CMakeLists.txt file needed to compile them.
RK_SRCS=/DATI/KDE/SRC/rekonq

# 3. Uncomment the "ADD_SUBDIRECTORY( i18n )" line in main CMakeLists.txt file.

# 4. test a package creation (to see the translations installed)

# THAT's ALL!!

########################################################################################################

# exit on most errors
set -e

# current dir
CWD=$(pwd)

# create the i18n dir
cd $RK_SRCS
mkdir -p i18n
cd i18n

# download the po files
for lang in $LIST
do
  wget http://websvn.kde.org/*checkout*/trunk/l10n-kde4/$lang/messages/playground-network/rekonq.po;
  mv rekonq.po rekonq_$lang.po;
done

# create the CMakeLists.txt file for the translations


echo '

FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
 
IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
        MESSAGE(
"------
                 NOTE: msgfmt not found. Translations will *not* be installed
------")
ELSE(NOT GETTEXT_MSGFMT_EXECUTABLE)
 
        SET(catalogname rekonq)
 
        ADD_CUSTOM_TARGET(translations ALL)
 
        FILE(GLOB PO_FILES ${catalogname}*.po)
 
        FOREACH(_poFile ${PO_FILES})
                GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
                STRING(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName} )
                STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )
 
                IF( _langCode )
                        GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
                        SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
 
                        ADD_CUSTOM_COMMAND(TARGET translations
                                COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                                DEPENDS ${_poFile})
                        INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)
                ENDIF( _langCode )
 
        ENDFOREACH(_poFile ${PO_FILES})
 
ENDIF(NOT GETTEXT_MSGFMT_EXECUTABLE)

' > CMakeLists.txt

# done :)
cd $CWD
echo "Done. Yuppy!"
