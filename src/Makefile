# $Id: Makefile,v 12.5 1994/07/01 23:28:19 tom Exp $
# MAKE-file for directory editor

THIS	= ded
TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Standard Lists) #######################################################
LINTLIBS= -ltd -lcurses -lapollo

DED_H	= $(THIS).h $(CURSES_H) $I/dyn_str.h

ALL	= $B/$(THIS) $B/$(THIS).hlp

CSRC	=\
	$(THIS).c \
	ded2s.c \
	dedblip.c \
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
	dedsize.c \
	dedsort.c \
	dedtags.c \
	dedtype.c \
	deduniq.c \
	dedview.c \
	dedwait.c \
	dlog.c \
	ftree.c \
	history.c \
	inline.c \
	restat.c \
	showpath.c \
	sortset.c
HSRC	= \
	$(THIS).h \
	version.h

Z	= $(THIS).a
OBJS	= \
	$Z(ded2s.o) \
	$Z(dedblip.o) \
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
	$Z(dedsize.o) \
	$Z(dedsort.o) \
	$Z(dedtags.o) \
	$Z(dedtype.o) \
	$Z(deduniq.o) \
	$Z(dedview.o) \
	$Z(dedwait.o) \
	$Z(dlog.o) \
	$Z(ftree.o) \
	$Z(history.o) \
	$Z(inline.o) \
	$Z(restat.o) \
	$Z(showpath.o) \
	$Z(sortset.o)

SOURCES	= Makefile README $(CSRC) $(HSRC) $(THIS).hlp $(THIS)rc
MAIN_O	= $(THIS).o $Z $(LIBS)

####### (Standard Productions) #################################################
all\
install:	$B $(SOURCES) $(ALL)

clean:				; rm -f $(CLEAN) $Z
clobber:	clean		; rm -f $(ALL)
destroy:			; $(DESTROY)
run_test:
sources:	$(SOURCES)

####### (Details of Productions) ###############################################
$B:				; mkdir $@
$B/$(THIS):	$(MAIN_O)	; $(LINK) -o $@ $(MAIN_O) -lcurses -ltermcap
$B/$(THIS).hlp:	$(THIS).hlp	; $(PUT)

$Z:	$(OBJS)
	ranlib $@

$(THIS).o:	$(DED_H) $(THIS).c version.h
$(OBJS):	$(DED_H)
$Z(dedsort.o):	$(QSORT_H)
$Z(ded2s.o):	$I/acl.h
$Z(dedscan.o):	$(RCSDEFS_H) $(SCCSDEFS_H)
$Z(ftree.o):	$(RCSDEFS_H) $(SCCSDEFS_H)

####### (Development) ##########################################################
$(SOURCES):			; $(GET) $@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPTS) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@

.PRECIOUS:	proto.h
proto.h:	$(CSRC)		; $(CPROTO) $(CPP_OPTS) $? >$@
tags:		$(CSRC)		; dotags $(CSRC)
