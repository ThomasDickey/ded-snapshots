# $Id: Makefile,v 11.3 1992/08/07 07:41:52 dickey Exp $
# MAKE-file for directory editor

####### (Environment) ##########################################################
THIS	= ded
include ../../common.mk

####### (Standard Lists) #######################################################
DED_H	= $(THIS).h $(PTYPES_H) $I/cmdch.h $I/dyn_string.h

ALL	= $B/$(THIS) $B/$(THIS).hlp

CSRC	=\
	$(THIS).c \
	ded2s.c \
	deddoit.c \
	deddump.c \
	dedfind.c \
	dedfree.c \
	dedline.c \
	dedmake.c \
	dedmsgs.c \
	dedname.c \
	dedread.c \
	dedring.c \
	dedscan.c \
	dedshow.c \
	dedsigs.c \
	dedsort.c \
	dedtags.c \
	dedtype.c \
	deduniq.c \
	dedview.c \
	dedwait.c \
	dlog.c \
	ftree.c \
	history.c \
	restat.c \
	showpath.c \
	sortset.c
HSRC	= \
	$(THIS).h \
	version.h

Z	= $(THIS).a
OBJS	= \
	$Z(ded2s.o) \
	$Z(deddoit.o) \
	$Z(deddump.o) \
	$Z(dedfind.o) \
	$Z(dedfree.o) \
	$Z(dedline.o) \
	$Z(dedmake.o) \
	$Z(dedmsgs.o) \
	$Z(dedname.o) \
	$Z(dedread.o) \
	$Z(dedring.o) \
	$Z(dedscan.o) \
	$Z(dedshow.o) \
	$Z(dedsigs.o) \
	$Z(dedsort.o) \
	$Z(dedtags.o) \
	$Z(dedtype.o) \
	$Z(deduniq.o) \
	$Z(dedview.o) \
	$Z(dedwait.o) \
	$Z(dlog.o) \
	$Z(ftree.o) \
	$Z(history.o) \
	$Z(restat.o) \
	$Z(showpath.o) \
	$Z(sortset.o)

SOURCES	= Makefile README $(CSRC) $(HSRC) $(THIS).hlp
MAIN_O	= $(THIS).o $Z $(LIBS)

####### (Standard Productions) #################################################
all\
install:	$(SOURCES) $(ALL)

clean:				; rm -f $(CLEAN)
clobber:	clean		; rm -f $(ALL)
destroy:			; $(DESTROY)
run_tests:
sources:	$(SOURCES)

####### (Details of Productions) ###############################################
$B/$(THIS):	$(MAIN_O)	; $(CC) -o $@ $(MAIN_O) -lcurses -ltermcap
$B/$(THIS).hlp:	$(THIS).hlp	; $(PUT)

$Z:	$(OBJS)
	ranlib $@

$(THIS).o:	$(DED_H) $(THIS).c version.h
$(OBJS):	$(DED_H)
$Z(ded2s.o):	$I/acl.h
$Z(dedscan.o):	$(RCSDEFS_H) $(SCCSDEFS_H)
$Z(ftree.o):	$(RCSDEFS_H) $(SCCSDEFS_H)

####### (Development) ##########################################################
$(SOURCES):			; $(GET) $@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@

.PRECIOUS:	proto.h
proto.h:	$(CSRC)		; $(CPROTO) $(CPP_OPTS) $? >$@
