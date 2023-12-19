#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/eprager/Projects/General/evk_test/build
  make -f /Users/eprager/Projects/General/evk_test/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/eprager/Projects/General/evk_test/build
  make -f /Users/eprager/Projects/General/evk_test/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/eprager/Projects/General/evk_test/build
  make -f /Users/eprager/Projects/General/evk_test/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/eprager/Projects/General/evk_test/build
  make -f /Users/eprager/Projects/General/evk_test/build/CMakeScripts/ReRunCMake.make
fi

