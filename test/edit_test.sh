#!/bin/sh
# $Id: edit_test.sh,v 9.0 1989/12/11 09:54:59 ste_cm Rel $
# Edit a test-log so that it will be simple to do regression tests on it.
# We make all references to the current pathname (of the 'ded' directory)
# substitute to CM_TOOLS.  Also, invoke sed to change noisy (irreproducible)
# comments to an innocuous form.
#
# $Log: edit_test.sh,v $
# Revision 9.0  1989/12/11 09:54:59  ste_cm
# BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
#
# Revision 8.0  89/12/11  09:54:59  ste_cm
# BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
# 
# Revision 7.0  89/12/11  09:54:59  ste_cm
# BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
# 
# Revision 6.0  89/12/11  09:54:59  ste_cm
# BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
# 
# Revision 5.1  89/12/11  09:54:59  dickey
# specify interpreter to avoid apollo sr10.1 bug
# 
# Revision 5.0  89/03/30  15:49:46  ste_cm
# BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
# 
P=`cd ..;pwd`
if test -z "$LOGNAME"
then
	LOGNAME=`set - \`ls -ld .\`;echo $3`
fi
for i in $*
do
	F=`basename $i .out`
	rm -f $F.tmp
	sed	-e s~$P~CM_TOOLS~\
		-e s~\"$LOGNAME\"~\"LOGNAME\"~\
		-f edit_test.sed $i >$F.tmp
done
