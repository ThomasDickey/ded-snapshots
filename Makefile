# $Header: /users/source/archives/ded.vcs/RCS/Makefile,v 3.0 1989/03/28 14:50:24 ste_cm Rel $
# Top-level makefile for unix directory-editor
#
# $Log: Makefile,v $
# Revision 3.0  1989/03/28 14:50:24  ste_cm
# BASELINE Mon Jun 19 14:20:36 EDT 1989
#
#	Revision 2.0  89/03/28  14:50:24  ste_cm
#	BASELINE Thu Apr  6 13:12:06 EDT 1989
#	
#	Revision 1.5  89/03/28  14:50:24  dickey
#	added 'all' dependency to 'install'
#	
#	Revision 1.4  89/03/28  10:15:39  dickey
#	use MAKE-variable to encapsulate recursive-build info.
#	
#	Revision 1.3  89/03/27  14:03:11  dickey
#	integration/cleanup for recursive make
#	
#	Revision 1.2  89/03/24  13:02:14  dickey
#	added help-file to install-list
#	
#	Revision 1.1  89/03/23  13:37:28  dickey
#	Initial revision
#	
####### (Development) ##########################################################
B	= ../../bin
L	= ../common/lib
PUT	= copy -v -d ../$@
GET	= checkout
MAKE	= make $(MFLAGS) -k$(MAKEFLAGS)	GET=$(GET)

####### (Standard Lists) #######################################################
PROG	= ded
ALL	=\
	$B/$(PROG)\
	$B/$(PROG).hlp
FIRST	=\
	bin\
	src/Makefile\
	test/Makefile

####### (Standard Productions) #################################################
all:		$L/lib.a

all\
clean\
clobber\
run_tests\
sources\
lincnt.out\
lint.out:	$(FIRST)
	cd src;		$(MAKE) $@
	cd test;	$(MAKE) $@

rdestroy\
destroy:	$(FIRST) clobber
	cd src;		$(MAKE) destroy
	cd test;	$(MAKE) destroy
	rmdir bin
	rm -f *

install:	all $(ALL)
deinstall:		; rm -f $(ALL)

####### (Details of Productions) ###############################################
bin:				; mkdir $@
src/Makefile:			; cd src;	$(GET) Makefile
test/Makefile:			; cd test;	$(GET) Makefile
$B/$(PROG):	bin/$(PROG)	; cd bin;	$(PUT)
$B/$(PROG).hlp:	bin/$(PROG).hlp	; cd bin;	$(PUT)
