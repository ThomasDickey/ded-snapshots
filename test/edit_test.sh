#!/bin/sh
# $Id: edit_test.sh,v 10.0 1991/10/18 16:25:02 ste_cm Rel $
#
# Edit a test-log so that it will be simple to do regression tests on it.
# We make all references to the current pathname (of the 'ded' directory)
# substitute to CM_TOOLS.  Also, invoke sed to change noisy (irreproducible)
# comments to an innocuous form.
#
# run from test-versions:
PATH=:`pwd`:`cd ../bin;pwd`:`cd ../../../bin;pwd`:/bin:/usr/bin:/usr/ucb
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
	sed	-e s~$P~CM_TOOLS~\
		-e s~\"$USER\"~\"LOGNAME\"~\
		-e s~\"$OWNER\"~\"LOGNAME\"~\
		-e s~\"$GROUP\"~\"GROUP\"~\
		-f edit_test.sed $i >$F.tmp
done
