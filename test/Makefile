# $Id: Makefile,v 12.2 1994/06/26 23:00:30 tom Exp $
# make-file for testing unix directory editor
#
####### (Development) ##########################################################
TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Standard Lists) #######################################################
SCRIPTS	=\
	edittest.sed\
	edittest.sh\
	run_test.sh\
	walkback.sh
CMD_FILES=\
	test1.cmd\
	test2.cmd\
	test3.cmd\
	test4.cmd\
	test5.cmd

SOURCES	= README $(SCRIPTS) $(CMD_FILES)

####### (Standard Productions) #################################################
all:		$(SOURCES)
clean::				; -$(RM) $(CLEAN) *.tmp
clean::				; -$(RM) -r junk*
clobber:	clean
destroy:			; $(DESTROY)
sources:	$(SOURCES)
lint.out:
lincnt.out:
run_test:	$(SOURCES)	; $(RUN_TESTS)

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
