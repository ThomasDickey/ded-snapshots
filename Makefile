# $Id: Makefile,v 11.3 1992/10/09 13:03:29 dickey Exp $
# Top-level makefile for unix directory-editor

####### (Development) ##########################################################
top	= ../..
TOP	= $(top)/..
include $(TOP)/cm_library/support/cm_library.mk

B	= $(top)/bin

####### (Standard Lists) #######################################################
THIS	= ded
ALL	=\
	$B/$(THIS)\
	$B/$(THIS).hlp
MFILES	=\
	src/Makefile\
	test/Makefile

####### (Standard Productions) #################################################
all\
run_tests::	$L/$(CM_LIB).a bin

all\
clean\
clobber\
destroy\
sources\
run_tests\
lincnt.out\
lint.out::	$(MFILES)
	cd src;		$(MAKE) $@
	cd test;	$(MAKE) $@

clean\
clobber::			; rm -f $(CLEAN)
clobber\
destroy::			; rm -rf bin
destroy::			; $(DESTROY)

install:	all $(ALL)
deinstall:			; rm -f $(ALL)

####### (Details of Productions) ###############################################
bin:				; mkdir $@
$(MFILES):			; $(GET) $@
$B/$(THIS):	bin/$(THIS)	; $(PUT)
$B/$(THIS).hlp:	bin/$(THIS).hlp	; $(PUT)
