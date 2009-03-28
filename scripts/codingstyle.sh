#!/bin/sh
#
# apply kdelibs coding style to all c, cpp and header files in src directory 
# requirements: installed astyle 
# 
# The coding style is defined in http://techbase.kde.org/Policies/Kdelibs_Coding_Style 
#
# In rekonq we use some little different rules..

{
PWD=$(pwd)

cd $PWD
cd ..
cd src

astyle \
--indent=spaces=4 \
--brackets=break \
--indent-labels \
--pad=oper \
--unpad=paren \
--one-line=keep-statements \
--convert-tabs \
--indent-preprocessor \
`find -type f -name '*.cpp'` `find -type f -name '*.h'`
}
