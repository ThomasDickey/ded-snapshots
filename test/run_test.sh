#!/bin/sh
# $Id: run_test.sh,v 11.0 1992/07/23 12:03:05 ste_cm Rel $
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
if test -f /com/vt100
then	TB=/com/tb
else	rm -f core
	TB=./traceback.sh
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
for i in $*
do
	F=`basename $i .cmd`
	trap "rm -f /tmp/.ftree $F.tmp $F.out" 0 1 2 5 15
	echo '** running '$F
	rm -f $F.tmp $F.out
	rm -f /tmp/.ftree
	if test -f $F.cmd
	then	OPTS="-c$F.cmd -l$F.out"
	else	OPTS="-l$F.out"
	fi
	if ( ded -t/tmp $OPTS >>$TTY )
	then	echo '** normal completion'
	else	eval $TB
	fi
	edit_test.sh $F.out
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
done
