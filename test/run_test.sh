#!/bin/sh
# $Id: run_test.sh,v 9.0 1990/05/11 11:21:04 ste_cm Rel $
# Perform regression tests for unix directory editor.  If we find any problems,
# show it in the log.
#
# $Log: run_test.sh,v $
# Revision 9.0  1990/05/11 11:21:04  ste_cm
# BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
#
# Revision 8.0  90/05/11  11:21:04  ste_cm
# BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
# 
# Revision 7.1  90/05/11  11:21:04  dickey
# set umask to make test repeatable
# 
# Revision 7.0  89/12/11  09:52:18  ste_cm
# BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
# 
# Revision 6.0  89/12/11  09:52:18  ste_cm
# BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
# 
# Revision 5.1  89/12/11  09:52:18  dickey
# specify interpreter to avoid apollo sr10.1 bug
# 
# Revision 5.0  89/03/30  16:07:02  ste_cm
# BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
# 
L=run_tests.out
date >>$L
#
umask 022
for i in *.cmd
do
	F=`basename $i .cmd`
	rm -f $F.tmp $F.out $F.dif
	rm -f /tmp/.ftree
	../bin/ded -t/tmp -c$F.cmd -l$F.out >>/dev/tty
	edit_test.sh $F.out
	diff $F.cmd $F.tmp >$F.dif
	if [ -s $F.dif ]
	then
		echo '?? test-difference for '$i | tee -a $L
		cat $F.dif | tee -a $L
	fi
	rm -f $F.tmp $F.out $F.dif
done
rm -f /tmp/.ftree
