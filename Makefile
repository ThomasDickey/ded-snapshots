# $Id: Makefile,v 8.0 1989/08/22 09:14:53 ste_cm Rel $
# Top-level makefile for unix directory-editor
#
# $Log: Makefile,v $
# Revision 8.0  1989/08/22 09:14:53  ste_cm
# BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
#
#	Revision 7.0  89/08/22  09:14:53  ste_cm
#	BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
#	
#	Revision 6.0  89/08/22  09:14:53  ste_cm
#	BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
#	
#	Revision 5.0  89/08/22  09:14:53  ste_cm
#	BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
#	
#	Revision 4.0  89/08/22  09:14:53  ste_cm
#	BASELINE Thu Aug 24 10:18:59 EDT 1989 -- support:navi_011(rel2)
#	
#	Revision 3.1  89/08/22  09:14:53  dickey
#	corrected 'destroy' rule
#	
#	Revision 3.0  89/03/28  14:50:24  ste_cm
#	BASELINE Mon Jun 19 14:20:36 EDT 1989
#	
####### (Development) ##########################################################
B	= ../../bin
L	= ../common/lib
PUT	= copy -v -d ../$@
GET	= checkout
MAKE	= make $(MFLAGS) -k$(MAKEFLAGS)	GET=$(GET)

####### (Standard Lists) #######################################################
PROG	= ded
ALL	=\
	$B/$(PROG)\
	$B/$(PROG).hlp
FIRST	=\
	bin\
	src/Makefile\
	test/Makefile

####### (Standard Productions) #################################################
all:		$L/lib.a

all\
clean\
clobber\
run_tests\
sources\
lincnt.out\
lint.out:	$(FIRST)
	cd src;		$(MAKE) $@
	cd test;	$(MAKE) $@

rdestroy\
destroy:	$(FIRST)
	cd src;		$(MAKE) destroy
	cd test;	$(MAKE) destroy
	rm -rf bin
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done;exit 0'

install:	all $(ALL)
deinstall:		; rm -f $(ALL)

####### (Details of Productions) ###############################################
bin:				; mkdir $@
src/Makefile:			; cd src;	$(GET) Makefile
test/Makefile:			; cd test;	$(GET) Makefile
$B/$(PROG):	bin/$(PROG)	; cd bin;	$(PUT)
$B/$(PROG).hlp:	bin/$(PROG).hlp	; cd bin;	$(PUT)
