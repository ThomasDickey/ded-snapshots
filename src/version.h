/*
 * $Id: version.h,v 8.5 1991/04/16 08:54:39 dickey Exp $
 * Version-identifier for DED
 * $Log: version.h,v $
 * Revision 8.5  1991/04/16 08:54:39  dickey
 * absorb backslash in shell command only when "#" or "%"
 * follows (briefer).
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
static	char	version[] = "$Date: 1991/04/16 08:54:39 $";
