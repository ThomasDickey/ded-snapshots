# $Header: /users/source/archives/ded.vcs/src/RCS/Makefile,v 3.0 1989/05/26 13:00:44 ste_cm Rel $
# MAKE-file for directory editor
#
# $Log: Makefile,v $
# Revision 3.0  1989/05/26 13:00:44  ste_cm
# BASELINE Mon Jun 19 14:21:57 EDT 1989
#
#        Revision 2.1  89/05/26  13:00:44  dickey
#        added 'dedread.c' module
#        
#        Revision 2.0  89/04/03  08:13:33  ste_cm
#        BASELINE Thu Apr  6 13:14:13 EDT 1989
#        

####### (Environment) ##########################################################
B	= ../bin
I	= ../../common/interface
L	= ../../common/lib
GET	= checkout
#
####### (Command-line Options) #################################################
DED_H	= ded.h $I/ptypes.h $I/cmdch.h
INCLUDES= -I. -I$I
CFLAGS	= $(DEFINES) $(INCLUDES) -O
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
	ftree.c
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
	ftree.o
#
SRCS	= README $(CSRC) $(HSRC) ded.hlp
MAIN_O	= ded.o $A $(LIBS)
#
####### (Standard Productions) #################################################
all:		$(ALL)
#
clean:				; rm -f *.[oa] *.out
clobber:	clean		; rm -f $(ALL)
destroy:	clobber		; rm -f *
run_tests:
sources:	$(SRCS)

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
#
####### (Development) ##########################################################
$(SRCS):			; $(GET) $@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@
