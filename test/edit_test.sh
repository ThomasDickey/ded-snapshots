: '$Header: /users/source/archives/ded.vcs/test/RCS/edit_test.sh,v 1.1 1989/03/24 11:29:50 dickey Exp $'
# Edit a test-log so that it will be simple to do regression tests on it.
# We make all references to the current pathname (of the 'ded' directory)
# substitute to CM_TOOLS.  Also, invoke sed to change noisy (irreproducible)
# comments to an innocuous form.
#
# $Log: edit_test.sh,v $
# Revision 1.1  1989/03/24 11:29:50  dickey
# Initial revision
#
P=`cd ..;pwd`
for i in $*
do
	F=`basename $i .out`
	rm -f $F.tmp
	sed -e s~$P~CM_TOOLS~ -f edit_test.sed $i >$F.tmp
done
