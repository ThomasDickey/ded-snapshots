# $Header: /users/source/archives/ded.vcs/RCS/Makefile,v 1.1 1989/03/23 13:37:28 dickey Exp $
# Top-level makefile for unix directory-editor
#
# $Log: Makefile,v $
# Revision 1.1  1989/03/23 13:37:28  dickey
# Initial revision
#
####### (Development) ##########################################################
B	= ../../bin
L	= ../common/lib
GET	= checkout

####### (Standard Lists) #######################################################
PROG	= ded
ALL	=\
	$B/$(PROG)
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
bin/$(PROG):	all
