# $Id: Makefile,v 12.1 1994/05/30 17:40:53 tom Exp $

TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Standard Lists) #######################################################
SOURCES	=\
	README\
	config.txt\
	release.txt

####### (Standard Productions) #################################################
all:		$(SOURCES) $(ALL)
clean:				; -$(RM) $(CLEAN)
clobber:	clean		; -$(RM) $(ALL)
destroy:			; $(DESTROY)
sources:	$(SOURCES)
lint.out\
lintlib\
run_test\
install\
deinstall:

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
