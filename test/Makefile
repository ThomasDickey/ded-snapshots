# $Header: /users/source/archives/ded.vcs/test/RCS/Makefile,v 3.0 1989/04/03 08:24:45 ste_cm Rel $
# make-file for testing unix directory editor
#
# $Log: Makefile,v $
# Revision 3.0  1989/04/03 08:24:45  ste_cm
# BASELINE Mon Jun 19 14:39:05 EDT 1989
#
#	Revision 2.0  89/04/03  08:24:45  ste_cm
#	BASELINE Thu Apr  6 13:28:24 EDT 1989
#	
#	Revision 1.3  89/04/03  08:24:45  dickey
#	added lint & lincnt rules, as well as README file
#	
#	Revision 1.2  89/03/24  12:30:36  dickey
#	piping ded output on Apollo breaks tests
#	
#	Revision 1.1  89/03/24  12:22:28  dickey
#	Initial revision
#	

####### (Development) ##########################################################
GET	= checkout

####### (Standard Lists) #######################################################
SCRIPTS	=\
	edit_test.sed\
	edit_test.sh\
	run_tests.sh
CMD_FILES=\
	test1.cmd\
	test2.cmd\
	test3.cmd

SOURCES	= README $(SCRIPTS) $(CMD_FILES)

####### (Standard Productions) #################################################
all:
clean:				; rm -f *.out
clobber:	clean
destroy:			; rm -f *
sources:	$(SOURCES)
lint.out:
lincnt.out:
run_tests:	$(SOURCES)	; $@.sh

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
