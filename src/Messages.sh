#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.rc' -or -name '*.kcfg'` >> rc.cpp || exit 11
$XGETTEXT `find . -name '*.cpp'` -o $podir/rekonq.pot
rm -f rc.cpp

