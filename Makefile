# $Id: Makefile,v 11.2 1992/08/07 14:36:45 dickey Exp $
# Top-level makefile for unix directory-editor

####### (Development) ##########################################################
include ../common.mk

B	= $(TOP)/bin
L	= ../$(THAT)/lib

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
run_tests::	$L/$(THAT).a bin

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
