# $Id: Makefile,v 12.0 1993/05/06 13:39:04 ste_cm Rel $

TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Standard Lists) #######################################################
SOURCES	=\
	README\
	config.txt\
	release.txt

####### (Standard Productions) #################################################
all:		$(SOURCES) $(ALL)
clean:				; rm -f $(CLEAN)
clobber:	clean		; rm -f $(ALL)
destroy:			; $(DESTROY)
sources:	$(SOURCES)
lint.out\
lintlib\
run_test\
install\
deinstall:

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
