#!/bin/sh
# $Id: edittest.sh,v 12.2 1994/07/12 01:04:14 tom Exp $
#
# Edit a test-log so that it will be simple to do regression tests on it.
# We make all references to the current pathname (of the 'ded' directory)
# substitute to CM_TOOLS.  Also, invoke sed to change noisy (irreproducible)
# comments to an innocuous form.
#
# run from test-versions:
PATH=/bin:/usr/bin:/usr/ucb
for p in ../../../bin ../../bin ../bin .
do
	if test -d $p
	then
		PATH="`cd $p;pwd`:$PATH"
	fi
done
export PATH
#
P=`cd ..;pwd`
USER=`whoami`
OWNER=`set - \`ls -lgd .\`;echo $3`
GROUP=`set - \`ls -lgd .\`;echo $4`
#
for i in $*
do
	F=`basename $i .out`
	rm -f $F.tmp
	sed	-e s~$P~CM_TOOLS~g\
		-e s~\"$USER\"~\"LOGNAME\"~\
		-e s~\"$OWNER\"~\"LOGNAME\"~\
		-e s~\"$GROUP\"~\"GROUP\"~\
		-f edittest.sed $i >$F.tmp
done
