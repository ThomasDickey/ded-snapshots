# $Id: Makefile,v 10.2 1992/02/04 10:38:54 dickey Exp $
# Top-level makefile for unix directory-editor

####### (Development) ##########################################################
B	= ../../bin
L	= ../common/lib

GET	= checkout
COPY	= cp -p
PUT	= rm -f $@; $(COPY) $? $@
MAKE	= make $(MFLAGS) -k$(MAKEFLAGS)	CFLAGS="$(CFLAGS)" COPY="$(COPY)"

####### (Standard Lists) #######################################################
PROG	= ded
ALL	=\
	$B/$(PROG)\
	$B/$(PROG).hlp
MFILES	=\
	src/Makefile\
	test/Makefile

####### (Standard Productions) #################################################
all::		bin $L/lib.a

all\
clean\
clobber\
destroy\
run_tests\
sources\
lincnt.out\
lint.out::	$(MFILES)
	cd src;		$(MAKE) $@
	cd test;	$(MAKE) $@

clobber\
destroy::
	rm -rf bin
destroy::
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done;exit 0'

install:	all $(ALL)
deinstall:		; rm -f $(ALL)

####### (Details of Productions) ###############################################
bin:				; mkdir $@
$(MFILES):			; $(GET) -x $@
$B/$(PROG):	bin/$(PROG)	; $(PUT)
$B/$(PROG).hlp:	bin/$(PROG).hlp	; $(PUT)
