dnl $Id: aclocal.m4,v 12.3 1997/09/13 14:48:52 tom Exp $
dnl Macros for DED configure script.
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2 (default: on)],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string 
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE($1,[$2],[test "$enableval" != ifelse($5,no,yes,no) && enableval=ifelse($5,no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse($3,,[    :]dnl
,[    $3]) ifelse($4,,,[
  else
    $4])
  fi],[enableval=$5 ifelse($4,,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname). 
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f $srcdir/config.guess ; then
	AC_CANONICAL_HOST
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name")
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT("Configuring for $cf_cv_system_name")

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT("Cached system name does not agree with actual")
	AC_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          test: display \"compiling\" commands],
	[
    ECHO_LD='@echo linking [$]@;'
    RULE_CC='	@echo compiling [$]<'
    SHOW_CC='	@echo compiling [$]@'
    ECHO_CC='@'
],[
    ECHO_LD=''
    RULE_CC='# compiling'
    SHOW_CC='# compiling'
    ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl Look for a non-standard library, given parameters for AC_TRY_LINK.  We
dnl prefer a standard location, and use -L options only if we do not find the
dnl library in the standard library location(s).
dnl	$1 = library name
dnl	$2 = includes
dnl	$3 = code fragment to compile/link
dnl	$4 = corresponding function-name
dnl
dnl Sets the variable "$cf_libdir" as a side-effect, so we can see if we had
dnl to use a -L option.
AC_DEFUN([CF_FIND_LIBRARY],
[
	cf_cv_have_lib_$1=no
	cf_libdir=""
	AC_CHECK_FUNC($4,cf_cv_have_lib_$1=yes,[
		cf_save_LIBS="$LIBS"
		AC_MSG_CHECKING(for $4 in -l$1)
		LIBS="-l$1 $LIBS"
		AC_TRY_LINK([$2],[$3],
			[AC_MSG_RESULT(yes)
			 cf_cv_have_lib_$1=yes
			],
			[AC_MSG_RESULT(no)
			CF_LIBRARY_PATH(cf_search,$1)
			for cf_libdir in $cf_search
			do
				AC_MSG_CHECKING(for -l$1 in $cf_libdir)
				LIBS="-L$cf_libdir -l$1 $cf_save_LIBS"
				AC_TRY_LINK([$2],[$3],
					[AC_MSG_RESULT(yes)
			 		 cf_cv_have_lib_$1=yes
					 break],
					[AC_MSG_RESULT(no)
					 LIBS="$cf_save_LIBS"])
			done
			])
		])
if test $cf_cv_have_lib_$1 = no ; then
	AC_ERROR(Cannot link $1 library)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Locate TD_LIB, which is either installed, with headers, library and
dnl include-file for make, or in a directory side-by-side with the current
dnl program's configure script. Use the side-by-side version in preference
dnl to the installed version, to facilitate development.
dnl
dnl Sets:
dnl	TD_LIB_rules - actual path of td_lib.mk
dnl
AC_DEFUN([CF_FIND_TDLIB],
[
AC_MSG_CHECKING(for td_lib in side-by-side directory)
AC_CACHE_VAL(cf_cv_tdlib_devel,[
	cf_cv_tdlib_devel=no

	test -d ../td_lib &&
	test -d ../td_lib/include &&
	test -f ../td_lib/include/td_config.h &&
	test -d ../td_lib/lib &&
	test -f ../td_lib/lib/libtd.a &&
	cf_cv_tdlib_devel=yes

	if test "$cf_cv_tdlib_devel" = yes ; then
		cf_save_CFLAGS="$CFLAGS"
		cf_save_LIBS="$LIBS"
		CFLAGS="-I`cd ../td_lib/include;pwd` $CFLAGS"
		LIBS="-L`cd ../td_lib/lib;pwd` $LIBS"
		TD_LIB_rules=`cd ../td_lib/support;pwd`
	fi
])
AC_MSG_RESULT($cf_cv_tdlib_devel)

if test "$cf_cv_tdlib_devel" = no ; then
    CF_HEADER_PATH(cf_search,td)
    # get all matches, since we're including <ptypes.h> and <td/ptypes.h>
    for cf_incdir in $cf_search
    do
	test -f $cf_incdir/td/td_config.h && CFLAGS="-I$cf_incdir $CFLAGS"
    done
    CF_FIND_LIBRARY(td,[
#define TESTING_CONFIG_H
#include <td/ptypes.h>],[
        char *p = doalloc(0,1)],
        doalloc)
	if test -z "$cf_libdir" ; then
		CF_LIBRARY_PATH(cf_search,td)
		cf_libdir=/usr/local/lib
		cf_td_lib_rules=no
		for cf_libdir in $cf_search
		do
			if test -f $cf_libdir/td_lib.mk ; then
				cf_td_lib_rules=yes
				break
			fi
		done
		test $cf_td_lib_rules = no && AC_ERROR(Cannot find td_lib.mk)
	fi
	TD_LIB_rules=$cf_libdir
fi

AC_SUBST(TD_LIB_rules)
])dnl
dnl ---------------------------------------------------------------------------
dnl Construct a search-list for a nonstandard header-file
AC_DEFUN([CF_HEADER_PATH],
[$1=""
if test -d "$includedir"  ; then
test "$includedir" != NONE       && $1="[$]$1 $includedir $includedir/$2"
fi
if test -d "$oldincludedir"  ; then
test "$oldincludedir" != NONE    && $1="[$]$1 $oldincludedir $oldincludedir/$2"
fi
if test -d "$prefix"; then
test "$prefix" != NONE           && $1="[$]$1 $prefix/include $prefix/include/$2"
fi
test "$prefix" != /usr/local     && $1="[$]$1 /usr/local/include /usr/local/include/$2"
test "$prefix" != /usr           && $1="[$]$1 /usr/include /usr/include/$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl Construct a search-list for a nonstandard library-file
AC_DEFUN([CF_LIBRARY_PATH],
[$1=""
if test -d "$libdir"  ; then
test "$libdir" != NONE           && $1="[$]$1 $libdir $libdir/$2"
fi
if test -d "$exec_prefix"; then
test "$exec_prefix" != NONE      && $1="[$]$1 $exec_prefix/lib $exec_prefix/lib/$2"
fi
if test -d "$prefix"; then
test "$prefix" != NONE           && \
test "$prefix" != "$exec_prefix" && $1="[$]$1 $prefix/lib $prefix/lib/$2"
fi
test "$prefix" != /usr/local     && $1="[$]$1 /usr/local/lib /usr/local/lib/$2"
test "$prefix" != /usr           && $1="[$]$1 /usr/lib /usr/lib/$2"
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if the 'make' program knows how to interpret archive rules.  Though
dnl this is common practice since the mid-80's, there are some holdouts (1997).
AC_DEFUN([CF_MAKE_AR_RULES],
[
AC_MSG_CHECKING(if ${MAKE-make} program knows about archives)
AC_CACHE_VAL(cf_cv_ar_rules,[
cf_dir=subd$$
cf_cv_ar_rules=unknown
mkdir $cf_dir
cat >$cf_dir/makefile <<CF_EOF
AR = ar crv
CC = $CC

.SUFFIXES:
.SUFFIXES: .c .o .a

all:  conf.a

.c.a:
	\$(CC) -c $<
	\$(AR) \$[]@ \$[]*.o
conf.a : conf.a(conftest.o)
CF_EOF
touch $cf_dir/conftest.c
if ( cd $cf_dir && ${MAKE-make} 2>&AC_FD_CC >&AC_FD_CC && test -f conf.a )
then
	cf_cv_ar_rules=yes
else
echo ... did not find archive >&AC_FD_CC
rm -f $cf_dir/conftest.o
cat >$cf_dir/makefile <<CF_EOF
AR = ar crv
CC = $CC

.SUFFIXES:
.SUFFIXES: .c .o

all:  conf.a

.c.o:
	\$(CC) -c $<

conf.a : conftest.o
	\$(AR) \$[]@ \$?
CF_EOF
if ( cd $cf_dir && ${MAKE-make} 2>&AC_FD_CC >&AC_FD_CC && test -f conf.a )
then
	cf_cv_ar_rules=no
else
	AC_ERROR(I do not know how to construct a library)
fi
fi
rm -rf $cf_dir
])
AC_MSG_RESULT($cf_cv_ar_rules)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for the use of 'include' in 'make' (BSDI is a special case)
dnl The symbol $ac_make is set in AC_MAKE_SET, as a side-effect.
AC_DEFUN([CF_MAKE_INCLUDE],
[
AC_MSG_CHECKING(for style of include in makefiles)

make_include_left=""
make_include_right=""
make_include_quote="unknown"

cf_inc=head$$
cf_dir=subd$$
echo 'RESULT=OK' >$cf_inc
mkdir $cf_dir

for cf_include in "include" ".include" "!include"
do
	for cf_quote in '' '"'
	do
		cat >$cf_dir/makefile <<CF_EOF
SHELL=/bin/sh
${cf_include} ${cf_quote}../$cf_inc${cf_quote}
all :
	@echo 'cf_make_include=\$(RESULT)'
CF_EOF
	cf_make_include=""
	eval `cd $cf_dir && ${MAKE-make} 2>&AC_FD_CC | grep cf_make_include=OK`
	if test -n "$cf_make_include"; then
		make_include_left="$cf_include"
		make_include_quote="$cf_quote"
		break
	else
		echo Tried 1>&AC_FD_CC
		cat $cf_dir/makefile 1>&AC_FD_CC
	fi
	done
	test -n "$cf_make_include" && break
done

rm -rf $cf_inc $cf_dir

if test -z "$make_include_left" ; then
	AC_ERROR(Your $ac_make program does not support includes)
fi
if test ".$make_include_quote" != .unknown ; then
	make_include_left="$make_include_left $make_include_quote"
	make_include_right="$make_include_quote"
fi

AC_MSG_RESULT(${make_include_left}file${make_include_right})

AC_SUBST(make_include_left)
AC_SUBST(make_include_right)
])dnl
dnl ---------------------------------------------------------------------------
dnl Tests for one or more programs given by name along the user's path, and
dnl sets a variable to the program's full-path if found.
AC_DEFUN([CF_PROGRAM_FULLPATH],
[
AC_MSG_CHECKING(full path of $1)
AC_CACHE_VAL(cf_cv_$1,[
	cf_cv_$1="[$]$1"
	if test -z "[$]cf_cv_$1"; then
		set -- $2;
		while test [$]# != 0; do
			cf_word=[$]1
			case [$]1 in
			-*)
				;;
			*)
				if test -f "$cf_word" && test ! -f "./$cf_word" && test -x "$cf_word"; then
					cf_cv_$1="$cf_word"
				else
					IFS="${IFS= 	}"; cf_save_ifs="$IFS"; IFS="${IFS}:"
					for cf_dir in $PATH; do
						test -z "$cf_dir" && cf_dir=.
						if test "$cf_dir" != "." && test -f $cf_dir/$cf_word && test -x $cf_dir/$cf_word; then
							cf_cv_$1="$cf_dir/$cf_word"
							break
						fi
					done
					IFS="$cf_save_ifs"
				fi
				if test -n "[$]cf_cv_$1"; then
					shift
					break
				fi
				;;
			esac
			shift
		done
	fi
	# append options, if any
	if test -n "[$]cf_cv_$1"; then
		while test [$]# != 0; do
			case [$]1 in
			-[*]) cf_cv_$1="[$]cf_cv_$1 [$]1";;
			[*])  set -- end;;
			esac
			shift
		done
	fi
])
if test -n "[$]cf_cv_$1"; then
	AC_DEFINE_UNQUOTED($1,"[$]cf_cv_$1")
  AC_MSG_RESULT("[$]cf_cv_$1")
else
  AC_MSG_RESULT((not found))
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Append predefined lists to $2/makefile, given a path to a directory that
dnl has a 'modules' file in $1.
dnl
dnl The library is named $Z, to avoid problems with parentheses.
AC_DEFUN([CF_SRC_MAKEFILE],
[
cf_mod=$1/$2/modules
cf_out=$2/makefile
if test -f $cf_mod ; then
${AWK-awk} <$cf_mod >>$cf_out '
BEGIN	{
		found = 0;
	}
	{
		if ( found == 0 )
		{
			printf "\nCSRC="
			found = 1;
		}
		printf " \\\n\t%s.c", [$]1
	}
END	{
		print ""
	}
'
	if test $cf_cv_ar_rules = yes ; then
${AWK-awk} <$cf_mod >>$cf_out '
BEGIN	{
		found = 0;
	}
	{
		if ( found == 0 )
		{
			printf "\nOBJS="
			found = 1;
		}
		printf " \\\n\t$Z(%s.o)", [$]1
	}
END	{
		print ""
	}
'
	else
${AWK-awk} <$cf_mod >>$cf_out '
BEGIN	{
		found = 0;
	}
	{
		if ( found == 0 )
		{
			printf "\nOBJS="
			found = 1;
		}
		printf " \\\n\t%s.o", [$]1
	}
END	{
		print ""
	}
'
	fi
ifelse($3,,,[
	cat >>$cf_out <<CF_EOF


${make_include_left}$3${make_include_right}
CF_EOF
])
test -f $1/$2/makefile.2nd && \
    cat $1/$2/makefile.2nd >>$2/makefile

	if test $cf_cv_ar_rules = yes ; then
cat >>$cf_out <<CF_EOF

\$Z:	\$(OBJS)
	\$(RANLIB) \$Z
CF_EOF
	else
cat >>$cf_out <<CF_EOF

\$Z:	\$(OBJS)
	\$(AR) \$Z \$(OBJS)
	\$(RANLIB) \$Z
CF_EOF
	fi
fi
test -f $1/$2/makefile.end && \
    cat $1/$2/makefile.end >>$2/makefile
])dnl
dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
changequote(,)dnl
$1=`echo $2 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
])dnl
