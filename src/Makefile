# $Id: Makefile,v 6.0 1990/03/05 11:13:33 ste_cm Rel $
# MAKE-file for directory editor
#
# $Log: Makefile,v $
# Revision 6.0  1990/03/05 11:13:33  ste_cm
# BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
#
#        Revision 5.4  90/03/05  11:13:33  dickey
#        cleanup
#        
#        Revision 5.3  90/03/02  11:55:00  dickey
#        separated CFLAGS, INCLUDES, DEFINES defs
#        
#        Revision 5.2  90/02/01  12:53:32  dickey
#        added module 'showpath.c'
#        
#        Revision 5.1  89/12/01  13:39:32  dickey
#        broke 'sortset.c' out of 'ded.c'
#        
#        Revision 5.0  89/10/04  11:16:19  ste_cm
#        BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
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

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<
