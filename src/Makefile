# @(#)Makefile	1.19 88/08/03 15:48:24
# MAKE-file for directory editor
#
####### (Environment) ##########################################################
B	= ../../bin
I	= ../lib
GET	= getdelta
#
####### (Command-line Options) #################################################
DED_H	= ded.h $I/cmdch.h
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
	deddump.c \
	dedfind.c \
	dedfree.c \
	deddoit.c \
	dedline.c \
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
	deddump.o \
	dedfind.o \
	dedfree.o \
	deddoit.o \
	dedline.o \
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
clean:				; rm -f *.o *.out
clobber:	clean		; rm -f $(ALL)
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
deddoit.o:	$(DED_H) deddoit.c
deddump.o:	$(DED_H) deddump.c
dedfind.o:	$(DED_H) dedfind.c
dedfree.o:	$(DED_H) dedfree.c
dedline.o:	$(DED_H) dedline.c
dedname.o:	$(DED_H) dedname.c
dedring.o:	$(DED_H) dedring.c
dedscan.o:	$(DED_H) dedscan.c
dedshow.o:	$(DED_H) dedshow.c
dedsigs.o:		 dedsigs.c
dedsort.o:	$(DED_H) dedsort.c
dedtype.o:	$(DED_H) dedtype.c
dedwait.o:	$(DED_H) dedwait.c
ded2s.o:	$(DED_H) ded2s.c
ftree.o:	$(DED_H) ftree.c
#
$B/ded:		ded		; rm -f $@;cp $? $@
$B/ded.hlp:	ded.hlp		; rm -f $@;cp $? $@
#
####### (Development) ##########################################################
$(SRCS):			; $(GET) -s sccs/s.$@
lint.out:	$(CSRC)		; $(DATE); tdlint $(LINTOPT) $? >>$@
lincnt.out:	$(CSRC)		; $(DATE); lincnt $(CSRC) >>$@
