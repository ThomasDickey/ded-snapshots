# $Id: Makefile,v 11.0 1992/07/23 12:05:48 ste_cm Rel $
# make-file for testing unix directory editor
#
####### (Development) ##########################################################
GET	= checkout

####### (Standard Lists) #######################################################
SCRIPTS	=\
	edit_test.sed\
	edit_test.sh\
	run_tests.sh\
	traceback.sh
CMD_FILES=\
	test1.cmd\
	test2.cmd\
	test3.cmd\
	test4.cmd

SOURCES	= README $(SCRIPTS) $(CMD_FILES)

####### (Standard Productions) #################################################
all:		sources
clean:				; rm -f *.out *.bak core
clobber:	clean
destroy:
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done;exit 0'
sources:	$(SOURCES)
lint.out:
lincnt.out:
run_tests:	$(SOURCES)	; sh -c '$@.sh 2>&1 | tee -a $@.out'

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
