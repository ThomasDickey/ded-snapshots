#!/bin/sh
# $Id: run_test.sh,v 10.1 1992/02/28 10:57:53 dickey Exp $
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
if test -f /com/vt100
then	TB=/com/tb
else	rm -f core
T	TB=./traceback.sh
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
	if ( ded -t/tmp -c$F.cmd -l$F.out >>$TTY )
	then	echo '** normal completion'
	else	eval $TB
	fi
	edit_test.sh $F.out
	diff $F.cmd $F.tmp >$F.dif
	if [ -s $F.dif ]
	then
		echo '?? test-difference for '$i
		cat $F.dif
	fi
	rm -f $F.tmp $F.out $F.dif
done
