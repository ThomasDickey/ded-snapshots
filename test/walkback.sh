#!/bin/sh
# $Id: walkback.sh,v 12.1 1991/11/12 07:59:12 tom Exp $
if test -f core
then
	adb $1 <<EOF/
\$c
\$q
EOF/
fi
