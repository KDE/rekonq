#! /bin/sh
$EXTRACTRC *.ui */*ui *.rc >> rc.cpp || exit 1
$XGETTEXT *.cpp */*.cpp -o $podir/rekonq.pot
rm -f rc.cpp

