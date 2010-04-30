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
cd src

echo "cppcheck(ing)..."
cppcheck \
--enable=all \
--force \
--verbose \
. 2>/tmp/cppcheck.out

echo "DONE. read output in /tmp/cppcheck.out"
