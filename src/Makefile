# @(#)Makefile	1.22 88/09/12 11:21:07
# MAKE-file for directory editor
#
####### (Environment) ##########################################################
B	= ../../bin
I	= ../lib
GET	= getdelta
#
####### (Command-line Options) #################################################
DED_H	= ded.h $I/ptypes.h $I/cmdch.h
INCLUDES= -I. -I$I
CFLAGS	= $(DEFINES) $(INCLUDES) -O
LIBS	= ../lib/lib.a
DATE	= echo '****'`date` >> $@
LINTOPT	= $(DEFINES) $(INCLUDES) -ltd -lcurses
#
####### (Standard Lists) #######################################################
ALL	= ded ded.hlp
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
	dedring.c \
	dedscan.c \
	dedshow.c \
	dedsigs.c \
	dedsort.c \
	dedtype.c \
	dedwait.c \
	ded2s.c \
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
	dedring.o \
	dedscan.o \
	dedshow.o \
	dedsigs.o \
	dedsort.o \
	dedtype.o \
	dedwait.o \
	ded2s.o \
	ftree.o
#
SRCS	= $(CSRC) $(HSRC) ded.hlp
MAIN_O	= ded.o $A $(LIBS)
#
####### (Standard Productions) #################################################
all:		$(ALL)
#
clean:				; rm -f *.[oa] *.out
clobber:	clean		; rm -f ded
destroy:			; rm -f *
sources:	$(SRCS)

install:	$B/ded\
		$B/ded.hlp
#
####### (Details of Productions) ###############################################
ded:	$(MAIN_O)		; $(CC) -o $@ $(MAIN_O) -lcurses -ltermcap
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
$B/ded:		ded		; copy -d $@
$B/ded.hlp:	ded.hlp		; copy -d $@
#
####### (Development) ##########################################################
$(SRCS):			; $(GET) -s sccs/s.$@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $(CSRC) >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@
