/*
 * Version-identifier for DED
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
static	char	version[] = "@(#)DED Directory Editor (89/03/14 14:00:59)";
