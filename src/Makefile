# $Id: Makefile,v 10.8 1992/04/07 07:48:38 dickey Exp $
# MAKE-file for directory editor
#
####### (Environment) ##########################################################
THIS	= ded

B	= ../bin
I	= ../../common/interface
L	= ../../common/lib

GET	= checkout
COPY	= cp -p
PUT	= rm -f $@; $(COPY) $? $@
#
####### (Command-line Options) #################################################
DED_H	= $(THIS).h $I/ptypes.h $I/common.h $I/cmdch.h $I/dyn_string.h

INCLUDES= -I. -I$I
CPP_OPTS= $(DEFINES) $(INCLUDES)

LIBS	= $L/lib.a
DATE	= echo '****'`date` >> $@
LINTOPT	= $(CPP_OPTS) -ltd -lcurses -lapollo
#
####### (Standard Lists) #######################################################
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
	restat.c \
	showpath.c \
	sortset.c
HSRC	= \
	$(THIS).h \
	version.h
#
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
	$Z(restat.o) \
	$Z(showpath.o) \
	$Z(sortset.o)
#
SOURCES	= Makefile README $(CSRC) $(HSRC) $(THIS).hlp
MAIN_O	= $(THIS).o $Z $(LIBS)
#
####### (Standard Productions) #################################################
all:		sources $(ALL)
#
clean:				; rm -f *.[oa] *.out *.bak core
clobber:	clean		; rm -f $(ALL)
destroy:
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done;exit 0'
run_tests:
sources:	$(SOURCES)

####### (Details of Productions) ###############################################
$B/$(THIS):	$(MAIN_O)	; $(CC) -o $@ $(MAIN_O) -lcurses -ltermcap
$B/$(THIS).hlp:	$(THIS).hlp	; $(PUT)
#
$Z:	$(OBJS)
	ranlib $@
#
$(THIS).o:	$(DED_H) $(THIS).c version.h
$(OBJS):	$(DED_H)
$Z(ded2s.o):	$I/acl.h
$Z(dedscan.o):	$I/rcsdefs.h $I/sccsdefs.h
$Z(ftree.o):	$I/rcsdefs.h $I/sccsdefs.h
#
####### (Development) ##########################################################
$(SOURCES):			; $(GET) $@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@

CPROTO	= cproto -e -i -fo'\\n\\t\\t' -fp'\\n\\t\\t'

.PRECIOUS:	proto.h
proto.h:	$(CSRC)		; $(CPROTO) $(CPP_OPTS) $? >$@

.SUFFIXES:
.SUFFIXES: .c .i .o .a .proto
.c.o:		; $(CC) $(CPP_OPTS) $(CFLAGS) -c $<
.c.a:
	@echo compile $<
	@$(CC) $(CFLAGS) $(CPP_OPTS) -c $*.c
	@ar rv $@ $*.o
	@rm -f $*.o
.c.i:		; $(CC) $(CPP_OPTS) -E -C $< >$@
.c.proto:	; $(CPROTO) $(CPP_OPTS) $< >$@
