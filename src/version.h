/*
 * Version-identifier for DED
 * 06 Jun 1988, provided clean recover for 'R' if no files are found.
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
static	char	version[] = "@(#)DED Directory Editor (88/06/06 12:30:33)";
