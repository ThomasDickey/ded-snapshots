# $Id: Makefile,v 6.1 1990/04/17 08:33:04 dickey Exp $
# MAKE-file for directory editor
#
# $Log: Makefile,v $
# Revision 6.1  1990/04/17 08:33:04  dickey
# suppress .c.a rule to cover up apollo sr10.2 make-bug (ded.c -> ded.a)
#
#        Revision 6.0  90/03/05  11:13:33  ste_cm
#        BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
#        
####### (Environment) ##########################################################
B	= ../bin
I	= ../../common/interface
L	= ../../common/lib
GET	= checkout
#
####### (Command-line Options) #################################################
DED_H	= ded.h $I/ptypes.h $I/common.h $I/cmdch.h
INCLUDES= -I. -I$I
CFLAGS	=
LIBS	= $L/lib.a
DATE	= echo '****'`date` >> $@
LINTOPT	= $(DEFINES) $(INCLUDES) -ltd -lcurses
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
clean:				; rm -f *.[oa] *.out
clobber:	clean		; rm -f $(ALL)
destroy:
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done;exit 0'
run_tests:
sources:	$(SOURCES)

####### (Details of Productions) ###############################################
$B/ded:	$(MAIN_O)		; $(CC) -o $@ $(MAIN_O) -lcurses -ltermcap
$B/ded.hlp:	ded.hlp		; copy -d $@
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
#
####### (Development) ##########################################################
$(SOURCES):			; $(GET) $@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@

.SUFFIXES:
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<
