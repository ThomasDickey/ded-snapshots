# $Id: Makefile,v 12.0 1993/05/06 13:52:44 ste_cm Rel $
# Top-level make-file for DED

THIS	= ded
TOP	= ..
include $(TOP)/td_lib/support/td_lib.mk

####### (Standard Lists) #######################################################
SOURCES	=\
	Makefile\
	COPYING\
	README

MFILES	=\
	certify/Makefile\
	src/Makefile\
	test/Makefile\
	user/Makefile

DIRS	=\
	bin

####### (Standard Productions) #################################################
all::		$(DIRS) $(SOURCES)
sources::	$(SOURCES)

all\
clean\
clobber\
destroy\
run_test\
sources::	$(MFILES)
	cd certify;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd test;	$(MAKE) $@
	cd user;	$(MAKE) $@

lint.out\
lincnt.out:	$(MFILES)
	cd src;		$(MAKE) $@

clean\
clobber::			; rm -f $(CLEAN)

clobber\
destroy::			; rm -rf $(DIRS)

destroy::			; $(DESTROY)

IT	= $(INSTALL_BIN)/$(THIS)

install::	all
install::	$(IT)
deinstall::			; rm -f $(IT)

install\
deinstall::
	cd user;	$(MAKE) $@ INSTALL_MAN=`cd ..;cd $(INSTALL_MAN);pwd`

####### (Details of Productions) ###############################################
$(SOURCES)\
$(MFILES):			; $(GET) $@

$(DIRS):			; mkdir $@
bin/$(THIS):	all

$(INSTALL_BIN)/$(THIS):		bin/$(THIS)	; $(COPY) bin/$(THIS) $@
