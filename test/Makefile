# $Header: /users/source/archives/ded.vcs/test/RCS/Makefile,v 1.2 1989/03/24 12:30:36 dickey Exp $
# make-file for testing unix directory editor
#
# $Log: Makefile,v $
# Revision 1.2  1989/03/24 12:30:36  dickey
# piping ded output on Apollo breaks tests
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

SOURCES	= $(SCRIPTS) $(CMD_FILES)

####### (Standard Productions) #################################################
all:
clean:				; rm -f *.out
clobber:	clean
destroy:			; rm -f *
sources:	$(SOURCES)
run_tests:	$(SOURCES)	; $@.sh

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
