: '$Header: /users/source/archives/ded.vcs/test/RCS/run_test.sh,v 5.0 1989/03/30 16:07:02 ste_cm Rel $'
# Perform regression tests for unix directory editor.  If we find any problems,
# show it in the log.
#
# $Log: run_test.sh,v $
# Revision 5.0  1989/03/30 16:07:02  ste_cm
# BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
#
# Revision 4.0  89/03/30  16:07:02  ste_cm
# BASELINE Thu Aug 24 10:31:56 EDT 1989 -- support:navi_011(rel2)
# 
# Revision 3.0  89/03/30  16:07:02  ste_cm
# BASELINE Mon Jun 19 14:39:05 EDT 1989
# 
# Revision 2.0  89/03/30  16:07:02  ste_cm
# BASELINE Thu Apr  6 13:28:24 EDT 1989
# 
# Revision 1.4  89/03/30  16:07:02  dickey
# use -t option of ded so we can ensure we have empty (known-state)
# directory-tree.
# 
# Revision 1.3  89/03/28  11:30:34  dickey
# pipe ded-stdout to /dev/tty to avoid problems if the stdout of this script
# is piped somewhere.
# 
# Revision 1.2  89/03/24  12:31:13  dickey
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
