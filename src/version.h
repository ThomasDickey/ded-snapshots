/*
 * Version-identifier for DED
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
static	char	version[] = "@(#)DED Directory Editor (88/04/27 10:42:01)";
