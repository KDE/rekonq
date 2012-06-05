#!/bin/sh
#
# apply rekonq coding style to all cpp and header files in src directory 
#
# requirements: installed astyle 
# 
# rekonq use kdelibs coding style, except for brackets, so while kdelibs coding style
# is
#
# void foo() {
# ...
# }
#
# rekonq uses
#
# void foo()
# {
#   ...
# }
#
# I like this way, for me more readable. :)
#
# Kdelibs coding style is defined in http://techbase.kde.org/Policies/Kdelibs_Coding_Style 


PWD=$(pwd)

cd $PWD
cd ..

echo "Applying astyle rules..."
astyle \
--indent=spaces=4 \
--style=allman \
--indent-labels \
--pad-oper \
--pad-header \
--unpad-paren \
--keep-one-line-statements \
--convert-tabs \
--indent-preprocessor \
`find -type f -name '*.cpp'` `find -type f -name '*.h'`

echo "Removing .orig files..."
rm *.orig */*.orig

echo "Done!"

