#!/bin/sh
# $Id: run_test.sh,v 12.2 1994/07/12 01:04:24 tom Exp $
#
# Perform regression tests for unix directory editor.  If we find any problems,
# show it in the log.
#
if test $# = 0
then
	$0 *.cmd
	exit
fi
#
TTY=/dev/tty
if ( date >>$TTY )
then	echo >/dev/null
else	echo '?? cannot run this script in batch-mode'
	exit 1
fi
#
# Apollo has /com/vt100
if test -f /com/vt100
then	TB=/com/tb
else	rm -f core
	TB=./walkback.sh
fi
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
umask 022
rm -f ../src/*.out
chmod 755 ../bin/ded
#
for i in $*
do
	F=`basename $i .cmd`
	R="/tmp/.ftree $F.tmp $F.out"
	trap "rm -f $R" 0 1 2 5 15
	echo '** running '$F
	rm -f $R
	if test -f $F.cmd
	then	OPTS="-c$F.cmd -l$F.out"
	else	OPTS="-l$F.out"
	fi
	if ( ded -t/tmp $OPTS >>$TTY )
	then	echo '** normal completion'
	else	eval $TB
	fi
	./edittest.sh $F.out
	if test -f $F.cmd
	then
		if ( cmp -s $F.cmd $F.tmp )
		then
			echo '** ok:   '$i
		else
			echo '?? diff: '$i
			diff $F.cmd $F.tmp
		fi
	else
		echo '** save: '$F.cmd
		mv $F.tmp $F.cmd
	fi
	rm -f $F.tmp $F.out
#?	rm -f $F.out
	trap "rm -f /tmp/.ftree" 0 1 2 5 15
done
