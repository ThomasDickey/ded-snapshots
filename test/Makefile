# $Id: Makefile,v 11.3 1992/10/09 13:05:32 dickey Exp $
# make-file for testing unix directory editor
#
####### (Development) ##########################################################
TOP	= ../../../..
include $(TOP)/cm_library/support/cm_library.mk

####### (Standard Lists) #######################################################
SCRIPTS	=\
	edit_test.sed\
	edit_test.sh\
	run_tests.sh\
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
run_tests:	$(SOURCES)	; $(RUN_TESTS)

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
