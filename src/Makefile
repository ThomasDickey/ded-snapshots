# $Id: Makefile,v 9.0 1990/08/27 08:12:09 ste_cm Rel $
# MAKE-file for directory editor
#
# $Log: Makefile,v $
# Revision 9.0  1990/08/27 08:12:09  ste_cm
# BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
#
#        Revision 8.1  90/08/27  08:12:09  dickey
#        really need to lint with apollo stuff, tho not portable
#        
#        Revision 8.0  90/04/18  07:42:02  ste_cm
#        BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
#        
#        Revision 7.0  90/04/18  07:42:02  ste_cm
#        BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
#        
#        Revision 6.2  90/04/18  07:42:02  dickey
#        updated include-dependency for 'dedscan.c'
#        
#        Revision 6.1  90/04/17  09:12:31  dickey
#        suppress .c.a rule to cover up apollo sr10.2 make-bug (ded.c -> ded.a)
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
LINTOPT	= $(DEFINES) $(INCLUDES) -ltd -lcurses -lapollo
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
dedscan:	$I/rcsdefs.h
#
####### (Development) ##########################################################
$(SOURCES):			; $(GET) $@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@

.SUFFIXES:
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<
