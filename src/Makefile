# @(#)Makefile	1.2 87/11/24 13:04:48
# MAKE-file for directory editor
DED_H   = ded.h
CFLAGS  = $(DEFINES) $(INCLUDES) -O
OBJS	=\
	ded.o\
        dedfind.o\
        deddoit.o\
        dedscan.o\
	dedsort.o\
	dedtype.o\
	ded2s.o\
	doalloc.o
LIBS	=\
	../lib/lib.a
#
ded:	$(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(LIBS) -lcurses -ltermcap
	chmod 755 $@
#
ded.o:	        $(DED_H) ded.c
deddoit.o:      $(DED_H) deddoit.c
dedfind.o:      $(DED_H) dedfind.c
dedscan.o:      $(DED_H) dedscan.c
dedsort.o:      $(DED_H) dedsort.c
dedtype.o:      $(DED_H) dedtype.c
ded2s.o:        $(DED_H) ded2s.c
#
clean:
	rm -f *.o
clobber: clean
install:
	rm -f ~/bin/ded ~/bin/ded.hlp
	cp ded ded.hlp ~/bin
#
lint.out:
	tdlint -lcurses -ltd\
		-DNO_SCCS_ID $(DEFINES) $(INCLUDES) *.c >lint.out
