# $Id: Makefile,v 10.3 1992/02/28 14:46:43 dickey Exp $
# MAKE-file for directory editor
#
####### (Environment) ##########################################################
B	= ../bin
I	= ../../common/interface
L	= ../../common/lib

GET	= checkout
COPY	= cp -p
PUT	= rm -f $@; $(COPY) $? $@
#
####### (Command-line Options) #################################################
DED_H	= ded.h $I/ptypes.h $I/common.h $I/cmdch.h $I/dyn_string.h

INCLUDES= -I. -I$I
CPP_OPTS= $(DEFINES) $(INCLUDES)

LIBS	= $L/lib.a
DATE	= echo '****'`date` >> $@
LINTOPT	= $(CPP_OPTS) -ltd -lcurses -lapollo
#
####### (Standard Lists) #######################################################
ALL	= $B/ded $B/ded.hlp
A	= ded.a
CSRC	=\
	ded.c \
	deddoit.c \
	deddump.c \
	dedfind.c \
	dedfree.c \
	dedline.c \
	dedmake.c \
	dedname.c \
	dedread.c \
	dedring.c \
	dedscan.c \
	dedshow.c \
	dedsigs.c \
	dedsort.c \
	dedtype.c \
	deduniq.c \
	dedwait.c \
	ded2s.c \
	dlog.c \
	ftree.c \
	showpath.c \
	sortset.c
HSRC	= \
	ded.h \
	version.h
#
OBJS	= \
	deddoit.o \
	deddump.o \
	dedfind.o \
	dedfree.o \
	dedline.o \
	dedmake.o \
	dedname.o \
	dedread.o \
	dedring.o \
	dedscan.o \
	dedshow.o \
	dedsigs.o \
	dedsort.o \
	dedtype.o \
	deduniq.o \
	dedwait.o \
	ded2s.o \
	dlog.o \
	ftree.o \
	showpath.o \
	sortset.o
#
SOURCES	= Makefile README $(CSRC) $(HSRC) ded.hlp
MAIN_O	= ded.o $A $(LIBS)
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
$B/ded:	$(MAIN_O)		; $(CC) -o $@ $(MAIN_O) -lcurses -ltermcap
$B/ded.hlp:	ded.hlp		; $(PUT)
#
$A:	$(OBJS)
	ar rv $@ $?
	ranlib $@
#
archive: $(OBJS)
	rm -f $A
	ar rv $A ($OBJS)
#
ded.o:		$(DED_H) ded.c version.h
$(OBJS):	$(DED_H)
ded2s.o:	$I/acl.h
dedscan:	$I/rcsdefs.h $I/sccsdefs.h
ftree.o:	$I/rcsdefs.h $I/sccsdefs.h
#
####### (Development) ##########################################################
$(SOURCES):			; $(GET) $@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@

CPROTO	= cproto -e -i -fo'\\n\\t\\t' -fp'\\n\\t\\t'

.PRECIOUS:	proto.h
proto.h:	$(CSRC)		; $(CPROTO) $(CPP_OPTS) $? >$@

.SUFFIXES:
.SUFFIXES: .c .i .o .proto
.c.o:		; $(CC) $(CPP_OPTS) $(CFLAGS) -c $<
.c.i:		; $(CC) $(CPP_OPTS) -E -C $< >$@
.c.proto:	; $(CPROTO) $(CPP_OPTS) $< >$@
