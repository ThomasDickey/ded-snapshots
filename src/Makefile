# @(#)Makefile	1.16 88/08/01 13:40:46
# MAKE-file for directory editor
B	= ../../bin
GET	= getdelta
#
DED_H   = ded.h ../lib/cmdch.h
INCLUDES= -I. -I../lib
CFLAGS  = $(DEFINES) $(INCLUDES) -O
LIBS	=\
	../lib/lib.a
#
SRCS	=\
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
	dedsort.c \
	dedtype.c \
	dedwait.c \
	ded2s.c \
	ftree.c
#
all:	ded
#
ded:	ded.o ded.a $(LIBS)
	$(CC) -o $@ ded.o ded.a $(LIBS) -lcurses -ltermcap
	chmod 755 $@
#
ftree:	ftree.c $(LIBS)
	mv ftree.o ftree.o-
	$(CC) -o $@ -DTEST ftree.c $(LIBS) -lcurses -ltermcap
	ls -l ftree.*
	mv ftree.o- ftree.o
#
ded.a:	\
        deddump.o\
        dedfind.o\
        dedfree.o\
        deddoit.o\
        dedline.o\
        dedname.o\
        dedring.o\
        dedscan.o\
	dedshow.o\
	dedsort.o\
	dedtype.o\
	dedwait.o\
	ded2s.o\
	ftree.o
	ar rv $@ $?
	ranlib $@
#
ded.o:	        $(DED_H) ded.c version.h
deddoit.o:      $(DED_H) deddoit.c
deddump.o:      $(DED_H) deddump.c
dedfind.o:      $(DED_H) dedfind.c
dedfree.o:      $(DED_H) dedfree.c
dedline.o:      $(DED_H) dedline.c
dedname.o:      $(DED_H) dedname.c
dedring.o:      $(DED_H) dedring.c
dedscan.o:      $(DED_H) dedscan.c
dedshow.o:      $(DED_H) dedshow.c
dedsort.o:      $(DED_H) dedsort.c
dedtype.o:      $(DED_H) dedtype.c
dedwait.o:      $(DED_H) dedwait.c
ded2s.o:        $(DED_H) ded2s.c
ftree.o:	$(DED_H) ftree.c
#
clean:
	rm -f *.o
clobber: clean
install:	$B/ded\
		$B/ded.hlp
#
$B/ded:		ded	; rm -f $@;cp $? $@
$B/ded.hlp:	ded.hlp	; rm -f $@;cp $? $@
#
ded.h\
ded.hlp\
$(SRCS):
	$(GET) -s sccs/s.$@
#
lint.out:
	tdlint -lcurses -ltd\
		-DNO_SCCS_ID $(DEFINES) $(INCLUDES) *.c >lint.out
