# $Id: Makefile,v 12.1 1994/05/30 17:41:10 tom Exp $

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
clean:					; -$(RM) $(CLEAN)
clobber:	clean			; -$(RM) $(ALL) *.cat
destroy:	clobber			; $(DESTROY)
run_test:
sources:	$(SOURCES)
install:	$(IT)
deinstall:				; -$(RM) $(IT)

####### (Details of Productions) ###############################################
$(PREFIX)$(THIS).$(SECTION):	$(THIS).$(USE)	; $(PUT)

$(SOURCES):				; $(GET) $@
