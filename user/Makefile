# $Id: Makefile,v 12.0 1993/05/06 14:07:06 ste_cm Rel $

THIS	= ded
TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Development) ##########################################################
USE	= man
SECTION	= 1
PREFIX	= $(INSTALL_MAN)/$(USE)$(SECTION)/

####### (Standard Lists) #######################################################
IT	=\
	$(PREFIX)$(THIS).$(SECTION)

SOURCES	=\
	$(THIS).man

ALL	=

####### (Standard Productions) #################################################
all:		$(SOURCES) $(ALL)
clean:					; rm -f $(CLEAN)
clobber:	clean			; rm -f $(ALL) *.cat
destroy:	clobber			; $(DESTROY)
run_test:
sources:	$(SOURCES)
install:	$(IT)
deinstall:				; rm -f $(IT)

####### (Details of Productions) ###############################################
$(PREFIX)$(THIS).$(SECTION):	$(THIS).$(USE)	; $(PUT)

$(SOURCES):				; $(GET) $@
