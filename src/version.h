/*
 * $Id: version.h,v 12.15 2000/10/19 09:52:50 tom Exp $
 * Version-identifier for DED
 *
 *		$Log: version.h,v $
 *		Revision 12.15  2000/10/19 09:52:50  tom
 *		19 Oct 2000, add '-p' option to print selected pathnames.
 *		08 Apr 2000, remove unneeded call for ncurses' trace().
 *		24 Jan 2000, revised directory-macros.
 *		24 Jan 2000, open .ftree in binary-mode for OS/2 EMX and Cygwin.
 *		16 Aug 1999, add cast to work with BeOS's long long ino_t.
 *		16 Aug 1999, use ttyname() for BeOS port.
 *		10 Aug 1999, change -b to a toggle, allow curses to decide if box characters are available.
 *		09 Aug 1999, allow color names to be mixed case, in any order.
 *		21 Jul 1998, show hostname prefix for pathname
 *		21 Jul 1998, change filelist header layout to allow for hostname prefix to to pathname.
 *		29 May 1998, compile with g++
 *		04 Mar 1998, rename 'y' sort to 'o'.
 *		15 Feb 1998, add home/end/ppage/npage keys.
 *		15 Feb 1998, remove special code for apollo sr10.  Correct a missing 'else' in time2s that caused future dates to be formatted as in the past.
 *		15 Feb 1998, remove special code for apollo sr10.  add home/end/ppage/npage cases.  change tag/untag to repaint faster.
 *		15 Feb 1998, corrected ifdef'ing of realpath vs chdir.
 *
 *		Revision 12.14  1998/02/16 02:18:59  tom
 *		generate all makefiles, making this portable to systems having 'make'
 *		programs w/o archive rules.
 *		remove apollo_sr10 code (because it's obsolete, and because I want to be
 *		able to re-use some of the command-codes).  This frees up o/O, to which I
 *		renamed y/Y (was lock owner).  That lets me reserve y/Y for vertical split.
 *		Started working on compiler warnings (gcc's signed/unsigned).
 *		Add home/end/ppage/npage keys where appropriate.
 *		Make the tag/untag operations faster by moving repaint out of repeat-loop.
 *		Correct a missing 'else' that caused future-dates to be formatted as if they
 *		were in the current week.
 *		Correct an ifdef for systems that have no 'realpath()'; was invoking chdir
 *		and this caused incorrect result in path_RESOLVE().
 *
 *		Revision 12.13  1997/02/11 09:36:50  tom
 *		fixes to dedscan's handling of common-prefix trimming (97/1, 97/2).
 *		restructured initialization of dedcolor to use ncurses 'use_default_colors()'.
 *		move ANSI_VARARGS tests into td_lib configure.
 *		make dedblip work before curses is initialized.
 *
 *		Revision 12.12  1996/03/16 19:36:50  tom
 *		memory leaks, mod to redoVIEW interface.
 *
 *		Revision 12.11  1996/02/10 01:32:48  tom
 *		mods to support scrolling regions
 *
 *		Revision 12.10  1996/01/13 15:11:04  tom
 *		mods for using the last (80th) column on display.  Use sysvr4 scrolling
 *		support if available.  Added -i option (temporary) for inverting color.
 *		Corrected ~ command in ftree.  Corrected infinite-loop in 'dedfind' when
 *		current file was symlink.
 *
 *		Revision 12.9  1995/11/05 23:29:47  tom
 *		mods to prevent tilde-expansion in list-entries
 *
 *		Revision 12.8  1995/11/05 22:28:43  tom
 *		mods to handle/display control characters better in shell
 *		commands. mods to make workspace pager scroll by single
 *		lines. mods to display in 80th column. mods to use 'const',
 *		and dyn_string() to cut down on data+bss sections.
 *
 *		Revision 12.7  1995/09/04 23:25:43  tom
 *		changes to support autoconf, /etc/DIR_COLOR.  Added "-b" and
 *		"-e" options (temporary!), and extended &-toggle to all
 *		dot-files.  Some bug-fixes for ring-maintainence.  Modified
 *		viewport handling to make toggling between lists more stable. 
 *		In td_lib, added module to support CmVision, and environment
 *		variables $DED_TREE, $DED_CM_LOOKUP.  Also in td_lib, mods for
 *		resizing support with ncurses, and using btree to speedup
 *		uid2s, gid2s.
 *
 *		Revision 12.6  1994/07/24 01:01:24  tom
 *		Allow '*' to have repeat count.
 *		Handle empty directories better (force '.').
 *		Mods for $DED_TREE.
 *		Revised ftree display; do left/right scroll; renamed 'A' to '&'.
 *		Support for auxiliary character set/ncurses.
 *		Added color support.
 *		Allow resizing (if curses supports it).
 *
 *		Revision 12.5  1994/07/01  00:21:51  tom
 *		S-sort. Mods to make scrolling smoother.
 *
 *		Revision 12.4  1994/07/01  00:18:43  tom
 *		HP/UX port. Linux port. Mods for autoconf. Mods for resizing (non-Sys5)
 *		curses windows.
 *
 *		Revision 12.3  1993/12/06  17:23:59  dickey
 *		added 'S' sort
 *		optimized some of the screen refreshing
 *
 *		Revision 12.2  1993/11/23  18:51:42  dickey
 *		Ifdef'd idents (to simply testing with gcc warnings)
 *		Ported to HP/UX.
 *		Simulate scrolling for up/down line commands.
 *		Made "^" command act as a toggle to top/bottom of screen.
 *		Added mouse support for xterm.
 *		Rewrote blip code, to show the counts rather than a lot of dots.
 *		Corrected an infinite loop in the filelist search commands.
 *
 *		Revision 12.1  1993/09/28  12:50:33  dickey
 *		gcc warnings
 *
 *		Revision 12.0  1992/08/28  09:50:52  ste_cm
 *		BASELINE Thu May  6 14:26:47 1993 -- split from CM_TOOLS #11
 *
 *		Revision 11.3  92/08/28  09:50:52  dickey
 *		added '-command (repeat last of given inline command)
 *		
 *		Revision 11.2  92/08/07  13:44:36  dickey
 *		added command-history to the non-inline text commands
 *		(e.g., filelist ':', '/', '?', pager '/', '?', and directory
 *		tree '/', '?', '@', '~').
 *		
 *		Revision 11.1  92/08/05  09:18:55  dickey
 *		added '/', '?', 'n', 'N' search commands to workspace pager.
 *		also added '<' and '>' commands to workspace pager.
 *		
 *		Revision 11.0  92/04/08  13:17:04  ste_cm
 *		BASELINE Thu Jul 16 09:39:01 1992 -- EBPM4 support
 *		
 *		Revision 10.5  92/04/08  13:17:04  dickey
 *		restructured so that there is not nearly as much global data.
 *		This allowed me to complete the split-screen operations,
 *		even making the two viewports have independent (nearly)
 *		filelists.
 *		
 *		Revision 10.4  92/02/28  10:49:34  dickey
 *		(except for 'dlog.c') modified so that shell commands &
 *		substitution are done on dynamic-strings, allowing them to
 *		be very long. show ellipsis if "#" substitution is longer
 *		than 256-chars.
 *		
 *		Revision 10.3  92/02/17  15:05:55  dickey
 *		make directory-renaming work properly by renaming file-lists
 *		as well.
 *		
 *		Revision 10.2  92/02/06  10:43:06  dickey
 *		modified z/Z sort to make it easier to use. Now, 'Z' sorts
 *		by difference between checkin/modification times.  Also,
 *		rather than showing only '<' or '>', I show '-' and '+' if
 *		only one second difference applies -- makes it simpler to
 *		read when looking at apollo restores.
 *		
 *		Revision 10.1  92/01/16  15:51:37  dickey
 *		corrected an error in 'dedmake()' which broke replay-logic.
 *		make 'ded' able to treat argument which is a single file.
 *		
 *		Revision 10.0  91/10/16  12:47:15  ste_cm
 *		BASELINE Fri Oct 18 16:46:43 1991 -- ANSI conversion; vcs
 *		
 *		Revision 9.9  91/10/16  12:47:15  dickey
 *		mods to support replay of 'c' commands
 *		
 *		Revision 9.8  91/09/09  08:14:51  dickey
 *		make "2T" show days.fraction relative to the current time
 *		
 *		Revision 9.7  91/08/16  13:58:07  dickey
 *		interpret "2T" to show numeric date (fractions of days)
 *		
 *		Revision 9.6  91/07/24  12:18:05  dickey
 *		added command-substitution codes u,g,v,y
 *		
 *		Revision 9.5  91/07/17  07:45:18  dickey
 *		added @-sort and D-sort
 *		
 *		Revision 9.4  91/07/12  08:21:59  dickey
 *		added CTL/G command
 *		
 *		Revision 9.3  91/07/11  11:13:51  dickey
 *		minor nits about screen refresh
 *		
 *		Revision 9.2  91/07/02  17:48:21  dickey
 *		made file-list 'S' and 'P' commands 3-way.
 *		
 *		Revision 9.1  91/06/28  07:57:26  dickey
 *		corrected code which knows about effective/real user-id
 *		(e.g., flag for executable, permission for chmod).
 *		Added P-sort (sorts extended-acl-flag).
 *		
 *		Revision 9.0  91/05/31  09:15:12  ste_cm
 *		BASELINE Mon Jun 10 10:09:56 1991 -- apollo sr10.3
 *		
 *		Revision 8.8  91/05/31  09:15:12  dickey
 *		highlight the subtree in ftree 'R' command. mods for apollo
 *		sr10.3
 *		
 *		Revision 8.7  91/04/18  09:42:38  dickey
 *		added ':' command to directory-tree
 *		
 *		Revision 8.6  91/04/18  09:00:29  dickey
 *		added "cL" command to create hard links.
 *		
 *		Revision 8.5  91/04/16  08:54:39  dickey
 *		absorb backslash in shell command only when "#" or "%"
 *		follows (briefer).
 *		
 *		Revision 8.4  91/04/16  08:16:39  dickey
 *		modified so that "-" argument causes DED to read a list of
 *		arguments from stdin (no wildcard expansion!)
 *		
 *		Revision 8.3  91/04/04  09:28:42  dickey
 *		mods to recover/proceed when 'chdir()' works but 'getwd()'
 *		does not (execute, but no read permission in path).
 *		
 *		Revision 8.2  91/04/01  12:34:48  dickey
 *		added CTL/I (tab) subcommand to 't' (type) to alter tabs.
 *		
 *		Revision 8.1  90/08/27  10:46:21  dickey
 *		added better error recovery to ".ftree" read/write
 *		
 *		Revision 8.0  90/05/23  11:30:26  ste_cm
 *		BASELINE Mon Aug 13 15:06:41 1990 -- LINCNT, ADA_TRANS
 *		
 *		Revision 7.1  90/05/23  11:30:26  dickey
 *		corrections to edit-link 'E' command (for DSEE)
 *		corrections to path-prefix stripping in 'dedscan()'
 *		make "-t" option inherit into subprocesses of 'ded'.
 *		make CTL(E) command on directories set scan-pattern.
 *		
 *		Revision 7.0  90/04/27  16:38:43  ste_cm
 *		BASELINE Mon Apr 30 09:54:01 1990 -- (CPROTO)
 *		
 *		Revision 6.2  90/04/27  16:38:43  dickey
 *		corrections/modifications to pathname resolution to try to
 *		avoid confusion in ".." sequences with symbolic links vs bugs
 *		found when invoking ded while su'ing.  To do this, reduced
 *		usage of 'abspath()' in favor of 'abshome()'.
 *		
 *		Revision 6.1  90/04/18  07:43:41  dickey
 *		correction to 'E' command (following symbolic link to file).
 *		invoke 'rcslast()' to show "RCS,v" (permit-file) version.
 *		
 *		Revision 6.0  90/03/02  08:59:28  ste_cm
 *		BASELINE Thu Mar 29 07:37:55 1990 -- maintenance release (SYNTHESIS)
 *		
 *		Revision 5.5  90/03/02  08:59:28  dickey
 *		modified 't' command so that directories are shown in a
 *		reasonable form.  modified 'q' behavior to prompt user if
 *		he has gone into other directory than original-arg.  added
 *		"-n" option to support this (so subprocesses don't prompt).
 *		
 *		Revision 5.4  90/02/07  09:48:00  dickey
 *		added 'showpath()' procedure to handle display of very-long
 *		paths.  modified '#' command to provide reset/set/all mode.
 *		
 *		Revision 5.3  90/01/30  08:50:49  dickey
 *		added '-T' (and 'T' toggle) to show long date+time.
 *		added 0/2 repeat-count for ':' and '.' commands so we can
 *		reset/set clear-screen state from '!' or '%' commands.
 *		new 'T' toggle obsoletes 'T'-command, so this is changed to "2t"
 *		
 *		Revision 5.2  89/12/08  10:27:43  dickey
 *		added ":"-prompt scrolling for sort-commands
 *		
 *		Revision 5.1  89/12/01  15:00:11  dickey
 *		added special sort-keys '?' and newline.
 *		
 *		Revision 5.0  89/10/12  16:13:59  ste_cm
 *		BASELINE Fri Oct 27 12:27:25 1989 -- apollo SR10.1 mods + ADA_PITS 4.0
 *		
 *		Revision 4.3  89/10/12  16:13:59  dickey
 *		tuned apollo sr10.1 code (type-uid display).
 *		make "2G" and "2I" commands show double-columns of info.
 *		enhanced treatment of 'cmdcol[]' for better alignment.
 *		protect against chmod of extended-acls
 *		refined "st" command to handle "." and DSEE files.
 *		corrected RCS/SCCS display to show version/locker always.
 *		
 *		Revision 4.2  89/10/04  17:02:38  dickey
 *		added code for apollo/sr10.1: show extended acls, added
 *		options '-a' and '-O', with corresponding commands '&' and
 *		'O', sorts 'o' and 'O'.
 *		
 *		Revision 4.1  89/08/25  09:30:42  dickey
 *		revised window-repainting (to reduce number of malloc/free
 *		calls, which causes my apollo vt100 to barf).  Finished the
 *		'E' enhancement by making ded scroll to link-target.
 *		
 *		Revision 4.0  89/08/22  16:36:45  ste_cm
 *		BASELINE Thu Aug 24 10:20:06 EDT 1989 -- support:navi_011(rel2)
 *		
 *		Revision 3.2  89/08/22  16:36:45  dickey
 *		augmented 'E' (enter-directory) when applied to symbolic
 *		link to a file.
 *		
 *		Revision 3.1  89/08/11  14:30:33  dickey
 *		added a bit of error-recovery to 'read_char()'.
 *		enhanced '<' command by showing substitution for group.
 *		
 *		Revision 3.0  89/06/13  08:00:37  ste_cm
 *		BASELINE Mon Jun 19 14:21:57 EDT 1989
 *		
 *		Revision 2.4  89/06/13  08:00:37  dickey
 *		corrected pointer-bug which made CTL/R command fail on sun
 *		
 *		Revision 2.3  89/06/06  08:39:37  dickey
 *		corrected change to 'dedscan()' which broke '@' command.
 *		modified blip-call in Z-toggle to show successful sccs/RCS lookup.
 *		
 *		Revision 2.2  89/06/05  15:52:26  dickey
 *		simplified logic in dedscan, ftree which sets logical links in file-tree
 *		
 *		Revision 2.1  89/05/26  13:46:44  dickey
 *		added ctl/R command to provide per-directory read-selection expression
 *		
 *		Revision 2.0  89/04/03  09:55:39  ste_cm
 *		BASELINE Thu Apr  6 13:14:13 EDT 1989
 *		
 *		Revision 1.46  89/04/03  09:55:39  dickey
 *		patched 'restat_W()' function
 *		
 *		Revision 1.45  89/03/24  09:17:39  dickey
 *		added "-c" option to process command files.
 *		converted version-date to RCS-format.
 *		
 *		Revision 1.44  89/03/23  15:12:26  dickey
 *		store RCS date in 'version[]' rather than SCCS date
 *		
 *
 * 15 Mar 1989, make logging work with subprocesses.
 * 14 Mar 1989, added '<' command and "-l" (logfile) option.
 * 07 Mar 1989, corrected ftree interaction between A/@ commands.
 * 28 Feb 1989, plugged a few holes in ft_insert/ft_linkto calls.
 * 23 Jan 1989, added 'N' sort, '-t' option, 'A'-ftree toggle.  Expand '%F' and
 *		'%B' in '>' command.  Added '~'-ftree command.
 * 18 Jan 1989, added '#' command
 * 12 Sep 1988, added 'c' command, changed '@'-toggle interaction with stat's.
 * 09 Sep 1988, misc bug fixes
 * 02 Sep 1988, added '>' link-edit command.
 * 01 Sep 1988, edit 'argv[]' for repeats & common pathname.
 * 16 Aug 1988, added repeat-count to 'W' and 'l' file-list commands.
 * 12 Aug 1988, added "d" sort; lint/compile on apollo sys5 environment.
 * 03 Aug 1988, added 'X' command (splits current window), signals to long
 *		commands.  Added workspace-marker column-scale.
 * 25 Jul 1988, added repeat (level) count to ftree's R-command.
 * 11 Jul 1988, added tagsort ("+" between r/s and key in sort).
 * 08 Jul 1988, corrections to @,Y toggles, testing ftree-rename.
 * 16 Jun 1988, added '@'-toggle.
 * 07 Jun 1988, added CTL(K) command.
 * 06 Jun 1988, provided clean recover for 'R' if no files are found.
 *		use 'gethome()' for ".ftree" location (process, not login).
 * 01 Jun 1988, added 'Y'-toggle.
 * 26 May 1988, minor fixes to refresh, allocation.
 * 23 May 1988, added '.'-sort, extended RCS/SCCS interface for versions.
 * 18 May 1988, extended '%'-substitution.  Added RCS date-scanning.
 * 16 May 1988, added I,U toggles to ftree.  RCS mods.  Fixed 'resizewin()'.
 * 11 May 1988, first cut at renaming directories.
 * 10 May 1988, use 'txtalloc()' to reduce the amount of malloc/realloc.
 * 09 May 1988, ported from Apollo to Sun (gould).
 * 06 May 1988, more fixes, added W,Q commands to ftree.
 * 03 May 1988, more fixes, added P,s,t subcommands to 'p'.
 * 02 May 1988, miscellaneous enhancements/fixes for first user release.
 * 28 Apr 1988, integration with ftree complete, added "=" and '"' commands.
 * 27 Apr 1988, cleanup of 'rawgets()' interface to fix bugs in wraparound, etc.
 * 21 Apr 1988, corrections/enhancements to screen-refresh logic.
 * 11 Apr 1988, integrated 'U' command so we can issue either Aegis or Unix
 *		commands.
 * 25 Mar 1988, implemented ':'.  Provided 'D', 'e' commands interfacing to
 *		'ftree'.  Corrected screen-refresh (no 'touchwin!).
 * 14 Dec 1987, added CTL/E, CTL/V (apollo pad-edit/view)
 * 01 Dec 1987, added '*', '^' commands, made '%' refresh screen
 * 25 Nov 1987, added sccs-support (V,z,Z toggles, V,z,Z sorts)
 */
static
#ifndef __hpux	/* scanf is broken on HP/UX 9 */
const
#endif
char	version[] = "$Date: 2000/10/19 09:52:50 $";
