#!/bin/sh
# $Id: traceback.sh,v 12.0 1991/11/12 07:59:12 ste_cm Rel $
if test -f core
then
	adb $1 <<EOF/
\$c
\$q
EOF/
fi
