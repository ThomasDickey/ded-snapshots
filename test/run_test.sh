#!/bin/sh
# $Id: run_test.sh,v 9.2 1991/10/18 16:12:36 dickey Exp $
#
# Perform regression tests for unix directory editor.  If we find any problems,
# show it in the log.
#
date
#
# run from test-versions:
PATH=:`pwd`:`cd ../bin;pwd`:`cd ../../../bin;pwd`:/bin:/usr/bin:/usr/ucb
export PATH
#
umask 022
rm -f ../src/*.out
chmod 755 ../bin/ded
#
for i in *.cmd
do
	F=`basename $i .cmd`
	echo '** running '$F
	rm -f $F.tmp $F.out $F.dif
	rm -f /tmp/.ftree
	ded -t/tmp -c$F.cmd -l$F.out >>/dev/tty
	edit_test.sh $F.out
	diff $F.cmd $F.tmp >$F.dif
	if [ -s $F.dif ]
	then
		echo '?? test-difference for '$i
		cat $F.dif
	fi
	rm -f $F.tmp $F.out $F.dif
done
rm -f /tmp/.ftree
