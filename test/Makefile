# $Id: Makefile,v 11.4 1993/04/27 10:37:09 dickey Exp $
# make-file for testing unix directory editor
#
####### (Development) ##########################################################
TOP	= ../../../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Standard Lists) #######################################################
SCRIPTS	=\
	edit_test.sed\
	edit_test.sh\
	run_test.sh\
	traceback.sh
CMD_FILES=\
	test1.cmd\
	test2.cmd\
	test3.cmd\
	test4.cmd\
	test5.cmd

SOURCES	= README $(SCRIPTS) $(CMD_FILES)

####### (Standard Productions) #################################################
all:		$(SOURCES)
clean::				; rm -f $(CLEAN) *.tmp
clean::				; rm -rf junk*
clobber:	clean
destroy:			; $(DESTROY)
sources:	$(SOURCES)
lint.out:
lincnt.out:
run_test:	$(SOURCES)	; $(RUN_TESTS)

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
