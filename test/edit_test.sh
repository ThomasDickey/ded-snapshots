: '$Header: /users/source/archives/ded.vcs/test/RCS/edit_test.sh,v 5.0 1989/03/30 15:49:46 ste_cm Rel $'
# Edit a test-log so that it will be simple to do regression tests on it.
# We make all references to the current pathname (of the 'ded' directory)
# substitute to CM_TOOLS.  Also, invoke sed to change noisy (irreproducible)
# comments to an innocuous form.
#
# $Log: edit_test.sh,v $
# Revision 5.0  1989/03/30 15:49:46  ste_cm
# BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
#
# Revision 4.0  89/03/30  15:49:46  ste_cm
# BASELINE Thu Aug 24 10:31:56 EDT 1989 -- support:navi_011(rel2)
# 
# Revision 3.0  89/03/30  15:49:46  ste_cm
# BASELINE Mon Jun 19 14:39:05 EDT 1989
# 
# Revision 2.0  89/03/30  15:49:46  ste_cm
# BASELINE Thu Apr  6 13:28:24 EDT 1989
# 
# Revision 1.2  89/03/30  15:49:46  dickey
# edit user-name (assumed to be file-owner!) to LOGNAME string.
# 
# Revision 1.1  89/03/24  11:29:50  dickey
# Initial revision
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
