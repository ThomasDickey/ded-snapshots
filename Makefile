# $Header: /users/source/archives/ded.vcs/RCS/Makefile,v 1.2 1989/03/24 13:02:14 dickey Exp $
# Top-level makefile for unix directory-editor
#
# $Log: Makefile,v $
# Revision 1.2  1989/03/24 13:02:14  dickey
# added help-file to install-list
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
	src/Makefile

####### (Standard Productions) #################################################
all:		$L/lib.a
all\
clean\
clobber\
destroy\
sources\
lint.out:	$(FIRST)
	cd src;		make $(MFLAGS) $@

run_tests:	all
	cd test;	make $(MFLAGS) $@

install:	$(ALL)
deinstall:		; rm -f $(ALL)

####### (Details of Productions) ###############################################
bin:				; mkdir $@
src/Makefile:			; cd src; $(GET) Makefile
$B/$(PROG):	bin/$(PROG)	; cd bin; copy -v -d ../$@
$B/$(PROG).hlp:	bin/$(PROG).hlp	; cd bin; copy -v -d ../$@
bin/$(PROG):	all
bin/$(PROG).hlp: all
