#!/bin/sh
# $Id: run_test.sh,v 10.0 1991/10/22 12:21:11 ste_cm Rel $
#
# Perform regression tests for unix directory editor.  If we find any problems,
# show it in the log.
#
TTY=/dev/tty
if ( date >>$TTY )
then	echo >/dev/null
else	echo '?? cannot run this script in batch-mode'
	exit 1
fi
#
# run from test-versions:
PATH=:`pwd`:`cd ../bin;pwd`:`cd ../../../bin;pwd`:/bin:/usr/bin:/usr/ucb
export PATH
#
umask 022
rm -f ../src/*.out
chmod 755 ../bin/ded
#
trap "rm -f /tmp/.ftree" 0
#
for i in *.cmd
do
	F=`basename $i .cmd`
	echo '** running '$F
	rm -f $F.tmp $F.out $F.dif
	rm -f /tmp/.ftree
	ded -t/tmp -c$F.cmd -l$F.out >>$TTY
	edit_test.sh $F.out
	diff $F.cmd $F.tmp >$F.dif
	if [ -s $F.dif ]
	then
		echo '?? test-difference for '$i
		cat $F.dif
	fi
	rm -f $F.tmp $F.out $F.dif
done
