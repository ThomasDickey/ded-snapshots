# $Id: Makefile,v 12.0 1993/05/06 13:23:22 ste_cm Rel $
# make-file for testing unix directory editor
#
####### (Development) ##########################################################
TOP	= ../..
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
