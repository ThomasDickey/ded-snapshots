# $Header: /users/source/archives/ded.vcs/RCS/Makefile,v 1.3 1989/03/27 14:03:11 dickey Exp $
# Top-level makefile for unix directory-editor
#
# $Log: Makefile,v $
# Revision 1.3  1989/03/27 14:03:11  dickey
# integration/cleanup for recursive make
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
GET	= checkout

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
	cd src;		make $(MFLAGS) $@
	cd test;	make $(MFLAGS) $@

rdestroy\
destroy:	$(FIRST) clobber
	cd src;		make $(MFLAGS) destroy
	cd test;	make $(MFLAGS) destroy
	rm -f *

install:	$(ALL)
deinstall:		; rm -f $(ALL)

####### (Details of Productions) ###############################################
bin:				; mkdir $@
src/Makefile:			; cd src; $(GET) Makefile
test/Makefile:			; cd test;$(GET) Makefile
$B/$(PROG):	bin/$(PROG)	; cd bin; copy -v -d ../$@
$B/$(PROG).hlp:	bin/$(PROG).hlp	; cd bin; copy -v -d ../$@
bin/$(PROG):	all
bin/$(PROG).hlp: all
