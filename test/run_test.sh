: '$Header: /users/source/archives/ded.vcs/test/RCS/run_test.sh,v 1.2 1989/03/24 12:31:13 dickey Exp $'
# Perform regression tests for unix directory editor.  If we find any problems,
# show it in the log.
#
# $Log: run_test.sh,v $
# Revision 1.2  1989/03/24 12:31:13  dickey
# piping ded output on Apollo breaks tests
#
# Revision 1.1  89/03/24  12:22:59  dickey
# Initial revision
# 
L=run_tests.out
date >>$L
#
for i in *.cmd
do
	F=`basename $i .cmd`
	rm -f $F.out
	../bin/ded -c$F.cmd -l$F.out
	edit_test.sh $F.out
	diff $F.cmd $F.tmp >$F.dif
	if [ -s $F.dif ]
	then
		echo '?? test-difference for '$i | tee -a $L
		cat $F.dif | tee -a $L
	fi
	rm -f $F.tmp $F.out $F.dif
done
