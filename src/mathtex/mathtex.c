/****************************************************************************
 *
 * Copyright(c) 2007-2012, John Forkosh Associates, Inc. All rights reserved.
 *           http://www.forkosh.com   mailto: john@forkosh.com
 * --------------------------------------------------------------------------
 * This file is part of mathTeX, which is free software. You may redistribute
 * and/or modify it under the terms of the GNU General Public License,
 * version 3 or later, as published by the Free Software Foundation.
 *      MathTeX is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, not even the implied warranty of MERCHANTABILITY.
 * See the GNU General Public License for specific details.
 *      By using mathTeX, you warrant that you have read, understood and
 * agreed to these terms and conditions, and that you possess the legal
 * right and ability to enter into this agreement and to use mathTeX
 * in accordance with it.
 *      Your mathtex.zip distribution should contain the file COPYING,
 * an ascii text copy of the GNU General Public License, version 3.
 * If not, point your browser to  http://www.gnu.org/licenses/
 * or write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330,  Boston, MA 02111-1307 USA.
 * --------------------------------------------------------------------------
 *
 * Purpose:   o	MathTeX, licensed under the gpl, lets you easily embed
 *		LaTeX math in your html pages or blogs, wikis, bb's, etc.
 *		It submits a LaTeX math expression to latex, and
 *		immediately emits the corresponding gif image, rather than
 *		the usual TeX dvi.
 *		For example,
 *		 <img src="/cgi-bin/mathtex.cgi?\int_{-\infty}^xe^{-t^2}dt"
 *		  alt="" border=0 align=middle>
 *		immediately generates the corresponding gif image,
 *		displaying the rendered expression wherever you put
 *		that <img> tag.
 *		But there's no inherent need to repeatedly write
 *		cumbersome <img> tags as illustrated above.  You can write
 *		your own custom tags, or write a wrapper script around
 *		mathTeX to simplify the notation.
 *		See http://www.forkosh.com/mathtex.html for more information.
 *
 * Functions:	=============================================================
 *		main(argc,argv)     latex's math expression and emits gif/png
 *		mathtex(expression,filename)  create image of math expression
 *		setpaths(method)     set paths for latex,dvipng,dvips,convert
 *		isnotfound(filename)  check "... 2>.err" file for "not found"
 *		validate(expression) remove illegal \commands from expression
 *		advertisement(expression,mode)  wrap expression in ad message
 *		mathlog(expression,filename)          write entry in log file
 *		makepath(oath,name,extension)   construct path/name.extension
 *		isfexists(filename)      check whether or not filename exists
 *		isdexists(dirname)      check whether or not directory exists
 *		whichpath(program,nlocate)         determines path to command
 *		locatepath(program,nlocate) tries locate if whichpath() fails
 *		rrmdir(path)                                       rm -r path
 *		rewritecache(cachefile,maxage) write cachefile with imageinfo
 *		emitcache(cachefile,maxage,isbuffer) dump cachefile to stdout
 *		readcachefile(cachefile,buffer)    read cachefile into buffer
 *		crc16(s)                               16-bit crc of string s
 *		md5str(instr)                      md5 hash library functions
 *		unescape_url(url)             xlate all %xx's in url to ascii
 *		x2c(what)    xlate a single hex "xx" to equivalent ascii char
 *		timelimit(command,killtime)   throttle command after killtime
 *		getdirective(string,directive,iscase,isvalid,nargs,args) \dir
 *		mathprep(expression)           preprocessor for mathTeX input
 *		strwstr(string,substr,white,sublen)     find substr in string
 *		strreplace(string,from,to,iscase,nreplace)  change from to to
 *		strchange(nfirst,from,to)   change nfirst chars of from to to
 *		isstrstr(string,snippets,iscase)    is any snippet in string?
 *		nomath(s)          removes/replaces any LaTeX math chars in s
 *		strwrap(s,linelen,tablen)insert \n's and spaces to wrap lines
 *		strpspn(s,reject,segment) non-{[()]} chars of s not in reject
 *		strqspn(s,q,isunescape) find matching " or ' in quoted string
 *		isnumeric(s)                     determine if s is an integer
 *		evalterm(store,term)     evaluate numeric value of expression
 *		getstore(store,identifier)return value corresponding to ident
 *		timestamp(tzdelta,ifmt)       returns current date:time stamp
 *		tzadjust(tzdelta,year,month,day,hour)   adjust time for tzone
 *		daynumber(year,month,day)     #calendar days from Jan 1, 1973
 *		calendar(year,month,day)    formats one-month calendar string
 *		emitembedded(imagenum,isquery)  emit embedded image to stdout
 *		embeddedimages(imagenum,nbytes,imgtype)   embedded gif or png
 *
 * Source:	mathtex.c
 *
 * --------------------------------------------------------------------------
 * Notes      o	See the comment block above each function
 *		for more information about it.
 *	      o	MathTeX runs only under Unix-like operating systems.
 *		To compile mathTeX
 *		   cc mathtex.c -DLATEX=\"$(which latex)\" \
 *		   -DDVIPNG=\"$(which dvipng)\" \
 *		   -DDVIPS=\"$(which dvips)\" \
 *		   -DCONVERT=\"$(which convert)\" \
 *		   -o mathtex.cgi
 *		And see discussion of optional -D switches below.
 *		To install mathTeX
 *		   (a) mv mathtex.cgi to your cgi-bin/ directory
 *		       and chmod its permissions as necessary
 *		   (b) mkdir cgi-bin/mathtex  which is the cache directory
 *		       and chmod its permissions so mathtex.cgi can rw it.
 *		See http://www.forkosh.com/mathtex.html for more information.
 *	      o	The timelimit() code is adapted from
 *		   http://devel.ringlet.net/sysutils/timelimit/
 *		Compile with -DTIMELIMIT=\"$(which timelimit)\" to use an
 *		installed copy of that program rather than the built-in code.
 *	      o	Some program parameters adjustable by optional -D switches on
 *		mathTeX's compile line are illustrated with default values...
 *		-DLATEX=\"/usr/share/texmf/bin/latex\"  path to LaTeX program
 *		-DPDFLATEX=\"/usr/share/texmf/bin/pdflatex\" path to pdflatex
 *		-DDVIPNG=\"/usr/share/texmf/bin/dvipng\"       path to dvipng
 *		-DDVIPS=\"/usr/share/texmf/bin/dvips\"          path to dvips
 *		-DPS2EPSI=\"/usr/bin/ps2epsi\"                path to ps2epsi
 *		-DCONVERT=\"/usr/bin/convert\"                path to convert
 *		-DCACHE=\"mathtex/\"     relative path to mathTeX's cache dir
 *		-DTIMELIMIT=\"/usr/local/bin/timelimit\"    path to timelimit
 *		-WARNTIME=10   #secs latex can run using standalone timelimit
 *		-KILLTIME=10   #secs latex can run using built-in timelimit()
 *		-DGIF                                         emit gif images
 *		-DPNG                                         emit png images
 *		-DDISPLAYSTYLE                            \[ \displaystyle \]
 *		-DTEXTSTYLE                                    $ \textstyle $
 *		-DPARSTYLE       paragraph mode, supply your own $ $ or \[ \]
 *		-DFONTSIZE=5           1=\tiny,...,5=\normalsize,...,10=\Huge
 *		-DUSEPACKAGE=\"filename\"       file containing \usepackage's
 *		-DNEWCOMMAND=\"filename\"       file containing \newcommand's
 *		-DDPI=\"120\"        dvipng -D DPI  parameter (as \"string\")
 *		-DGAMMA=\"2.5\"   dvipng --gamma GAMMA  param (as \"string\")
 *		-DNOQUIET     -halt-on-error (default reply q(uiet) to error)
 *		-DTEXTAREANAME=\"formdata\"   <textarea name=...> in a <form>
 *		-DREFERER=\"\"         comma-separated list of valid referers
 *		-DMAXINVALID=0     max length expression from invalid referer
 *		-DADFREQUENCY=0  one request out of n displayed along with ad
 *		-DADVERTISEMENT=\"filename\"      file containing ad template
 *		See http://www.forkosh.com/mathtex.html for more information.
 *
 * --------------------------------------------------------------------------
 * Revision History:
 * See http://www.forkosh.com/mathtexchangelog.html for more information.
 * 10/11/07	J.Forkosh	Installation Version 1.00.
 * 02/17/08	J.Forkosh	Version 1.01
 * 03/06/09	J.Forkosh	Version 1.02
 * 08/14/09	J.Forkosh	Version 1.03
 * 04/06/11	J.Forkosh	Version 1.04
 * 11/15/11	J.Forkosh	Version 1.05
 * 12/13/11	J.Forkosh	Most recent change (see REVISIONDATE below)
 *
 ****************************************************************************/

/* -------------------------------------------------------------------------
Program ID
-------------------------------------------------------------------------- */
#define	VERSION "1.05"			/* mathTeX version number */
#define	REVISIONDATE "13 December 2011"	/* date of most recent revision */
#define	COPYRIGHTDATE "2007-2012"	/* copyright date */

/* -------------------------------------------------------------------------
Standard header files
-------------------------------------------------------------------------- */
/* ---
 * standard headers
 * ---------------- */
#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
char	*strcasestr();			/* non-standard extension */
#include <ctype.h>
#include <time.h>
extern	char **environ;			/* for \environment directive */

/* -------------------------------------------------------------------------
Information adjustable by -D switches on compile line
-------------------------------------------------------------------------- */
/* ---
 * executable paths (e.g., path/latex including filename of executable image)
 * ----------------------------------------------------------------------- */
/* --- determine which switches have been explicitly specified --- */
#if defined(LATEX)
  #define ISLATEXSWITCH 1		/* have -DLATEX=\"path/latex\" */
#else
  #define ISLATEXSWITCH 0		/* no -DLATEX switch */
  #define LATEX "/usr/share/texmf/bin/latex" /* default path to latex */
#endif
#if defined(PDFLATEX)
  #define ISPDFLATEXSWITCH 1		/* have -PDFDLATEX=\"path/latex\" */
#else
  #define ISPDFLATEXSWITCH 0		/* no -PDFDLATEX switch */
  #define PDFLATEX "/usr/share/texmf/bin/pdflatex" /* default pdflatex path*/
#endif
#if defined(DVIPNG)
  #define ISDVIPNGSWITCH 1		/* have -DDVIPNG=\"path/dvipng\" */
#else
  #define ISDVIPNGSWITCH 0		/* no -DDVIPNG switch */
  #define DVIPNG "/usr/share/texmf/bin/dvipng"/* default path to dvipng */
#endif
#if defined(DVIPS)
  #define ISDVIPSSWITCH 1		/* have -DDVIPS=\"path/dvips\" */
#else
  #define ISDVIPSSWITCH 0		/* no -DDVIPS switch */
  #define DVIPS "/usr/share/texmf/bin/dvips" /* default path to dvips */
#endif
#if defined(PS2EPSI)
  #define ISPS2EPSISWITCH 1		/* have -DPS2EPSI=\"path/ps2epsi\" */
#else
  #define ISPS2EPSISWITCH 0		/* no -DPS2EPSI switch */
  #define PS2EPSI "/usr/bin/ps2epsi"	/* default path to ps2epsi */
#endif
#if defined(CONVERT)
  #define ISCONVERTSWITCH 1		/* have -DCONVERT=\"path/convert\" */
#else
  #define ISCONVERTSWITCH 0		/* no -DCONVERT switch */
  #define CONVERT "/usr/bin/convert"	/* default path to convert */
#endif
#if defined(TIMELIMIT)
  #define ISTIMELIMITSWITCH 1	    /* have -DTIMELIMIT=\"path/timelimit\" */
#else
  #define ISTIMELIMITSWITCH 0		/* no -DTIMELIMIT switch */
  #define TIMELIMIT "/usr/local/bin/timelimit" /* default path to timelimit*/
#endif
/* --- paths, as specified by -D switches, else from whichpath() --- */
static	char latexpath[256] = LATEX,    pdflatexpath[256] = PDFLATEX,
	dvipngpath[256] = DVIPNG,       dvipspath[256] = DVIPS,
	ps2epsipath[256] = PS2EPSI,     convertpath[256] = CONVERT,
	timelimitpath[256] = TIMELIMIT;
/* --- source of path info: 0=default, 1=switch, 2=which, 3=locate --- */
static	int  islatexpath=ISLATEXSWITCH, ispdflatexpath=ISPDFLATEXSWITCH,
	isdvipngpath=ISDVIPNGSWITCH,    isdvipspath=ISDVIPSSWITCH,
	isps2epsipath=ISPS2EPSISWITCH,  isconvertpath=ISCONVERTSWITCH,
	istimelimitpath=ISTIMELIMITSWITCH;
/* ---
 * cache path -DCACHE=\"path/\" specifies directory
 * ------------------------------------------------ */
#if !defined(CACHE)
  #define CACHE "mathtex/"		/* relative to mathtex.cgi */
#endif
#if !defined(CACHELOG)
  #define CACHELOG "mathtex.log"	/* default cache log file */
#endif
#define	MAXAGE 7200			/* maxage in cache is 7200 secs */
static	int  iscaching = 1;		/* true if caching images */
static	char cachepath[256] = CACHE;	/* path to cached image files */
/* ---
 * latex method info specifying latex,pdflatex
 * ------------------------------------------- */
#if    ISLATEXSWITCH==0 && ISPDFLATEXSWITCH==1
  #define LATEXMETHOD 2
#elif !defined(LATEXMETHOD)
  #define LATEXMETHOD 1
#endif
static	int  latexmethod = LATEXMETHOD;	/* 1=latex, 2=pdflatex */
static	int  ispicture = 0;		/* true for picture environment */
/* ---
 * image method info specifying dvipng or dvips/convert
 * use dvipng if -DDVIPNG supplied (or -DDVIPNGMETHOD specified),
 * else use dvips/convert if -DDVIPS supplied (or -DDVIPSMETHOD specified)
 * ----------------------------------------------------------------------- */
#if    defined(DVIPNGMETHOD) || ISDVIPNGSWITCH==1
  #define IMAGEMETHOD 1
#elif  defined(DVIPSMETHOD)  || (ISDVIPSSWITCH==1 && ISDVIPNGSWITCH==0)
  #define IMAGEMETHOD 2
#elif !defined(IMAGEMETHOD)
  #define IMAGEMETHOD 1
#endif
static	int  imagemethod = IMAGEMETHOD;	/* 1=dvipng, 2=dvips/convert */
/* ---
 * image type info specifying gif, png
 * ----------------------------------- */
#if defined(GIF)
  #define IMAGETYPE 1
#endif
#if defined(PNG)
  #define IMAGETYPE 2
#endif
#if !defined(IMAGETYPE)
  #define IMAGETYPE 1
#endif
static	int  imagetype = IMAGETYPE;	/* 1=gif, 2=png */
static	char *extensions[] = { NULL,	/* image type file .extensions */
  "gif", "png", NULL };
/* ---
 * \[ \displaystyle \]  or  $ \textstyle $  or  \parstyle
 * ------------------------------------------------------ */
#if defined(DISPLAYSTYLE)
  #define MATHMODE 0
#endif
#if defined(TEXTSTYLE)
  #define MATHMODE 1
#endif
#if defined(PARSTYLE)
  #define MATHMODE 2
#endif
#if !defined(MATHMODE)
  #define MATHMODE 0
#endif
static	int  mathmode = MATHMODE; /* 0=display 1=text 2=paragraph */
/* ---
 * font size info 1=\tiny ... 10=\Huge
 * ----------------------------------- */
#if !defined(FONTSIZE)
  #define FONTSIZE 5
#endif
static	int  fontsize = FONTSIZE;	/* 1=tiny ... 10=Huge */
static	char *sizedirectives[] = { NULL, /* fontsize directives */
  "\\tiny", "\\scriptsize", "\\footnotesize", "\\small", "\\normalsize",
  "\\large", "\\Large", "\\LARGE", "\\huge", "\\Huge", NULL };
/* ---
 * dpi/density info for dvipng/convert
 * ----------------------------------- */
#if !defined(DPI)
  #define DPI "120"
#endif
static	char density[256] = DPI;	/*-D/-density arg for dvipng/convert*/
/* ---
 * default -gamma for convert is 0.5, or --gamma for dvipng is 2.5
 * --------------------------------------------------------------- */
#define	DVIPNGGAMMA "2.5"		/* default gamma for dvipng */
#define	CONVERTGAMMA "0.5"		/* default gamma for convert */
#if !defined(GAMMA)
  #define ISGAMMA 0			/* no -DGAMMA=\"gamma\" switch */
  #if IMAGEMETHOD == 1			/* for dvipng... */
    #define GAMMA DVIPNGGAMMA		/* ...default gamma is 2.5 */
  #elif IMAGEMETHOD == 2		/* for convert... */
    #define GAMMA CONVERTGAMMA		/* ...default gamma is 0.5 */
  #else					/* otherwise... */
    #define GAMMA "1.0"			/* ...default gamma is 1.0 */
  #endif
#else
  #define ISGAMMA 1			/* -DGAMMA=\"gamma\" supplied */
#endif
static	char gamma[256] = GAMMA;	/* -gamma arg for convert() */
/* ---
 * latex -halt-on-error or quiet
 * ----------------------------- */
#if defined(QUIET)
  #define ISQUIET 99999			/* reply q */
#elif  defined(NOQUIET)
  #define ISQUIET 0			/* reply x (-halt-on-error) */
#elif  defined(NQUIET)
  #define ISQUIET NQUIET		/* reply NQUIET <Enter>'s then x */
#else
  #define ISQUIET 3			/* default reply 3 <Enter>'s then x*/
#endif
static	int  isquiet = ISQUIET;		/* >99=quiet, 0=-halt-on-error */
/* ---
 * emit depth below baseline (for vertical centering)
 * -------------------------------------------------- */
#if defined(DEPTH)
  #define ISDEPTH 1
#else
  #define ISDEPTH 0
#endif
static	int  isdepth = ISDEPTH;		/* true to emit depth */
/* ---
 * timelimit -tWARNTIME -TKILLTIME
 * ------------------------------- */
#if !defined(KILLTIME)			/* no -DKILLTIME given... */
  #define NOKILLTIMESWITCH		/* ...remember that fact below */
  #if ISTIMELIMITSWITCH == 0		/* standalone timelimit disabled */
    #if defined(WARNTIME)		/* have WARNTIME but not KILLTIME */
      #define KILLTIME (WARNTIME)	/* so use WARNTIME for KILLTIME */
    #else				/* neither WARNTIME nor KILLTIME */
      #define KILLTIME (10)		/* default for built-in timelimit()*/
      /*#define KILLTIME (0)*/		/* disable until debugged */
    #endif
  #else					/* using standalone timelimit */
    #define KILLTIME (1)		/* always default -T1 killtime */
  #endif
#endif
#if !defined(WARNTIME)			/*no -DWARNTIME given for timelimit*/
  #if ISTIMELIMITSWITCH == 0		/* and no -DTIMELIMIT path either */
    #define WARNTIME (-1)		/* so standalone timelimit disabled*/
  #else					/*have path to standalone timelimit*/
    #if !defined(NOKILLTIMESWITCH)	/* have KILLTIME but not WARNTIME */
      #define WARNTIME (KILLTIME)	/* so use KILLTIME for WARNTIME */
      #undef  KILLTIME			/* but not for standalone killtime */
      #define KILLTIME (1)		/* default -T1 killtime instead */
    #else				/* neither KILLTIME nor WARNTIME */
      #define WARNTIME (10)		/* so default -t10 warntime */
    #endif
  #endif
#endif
static	int  warntime=WARNTIME, killtime=KILLTIME; /* -twarn -Tkill values */
/* ---
 * compile (or not) built-in timelimit() code
 * ------------------------------------------ */
#if ISTIMELIMITSWITCH==0 && KILLTIME>0	/*no -DTIMELIMIT, but have KILLTIME*/
  #define ISCOMPILETIMELIMIT 1		/* so we need built-in code */
#else
  #define ISCOMPILETIMELIMIT 0		/* else we don't need built-in code*/
#endif
static	int  iscompiletimelimit=ISCOMPILETIMELIMIT; /* 1=use timelimit() */
#if ISCOMPILETIMELIMIT			/*header files, etc for timelimit()*/
  /* --- header files for timelimit() --- */
  #include <sys/signal.h>
  #include <sys/wait.h>
  #include <unistd.h>
  #include <signal.h>
  #include <sysexits.h>
  /*#define EX_OSERR	71*/		/* system error (e.g., can't fork) */
  #define HAVE_SIGACTION
  /* --- global variables for timelimit() --- */
  volatile int	fdone, falarm, fsig, sigcaught;
#endif
/* ---
 * additional latex \usepackage{}'s
 * -------------------------------- */
static	int  npackages = 0;		/* number of additional packages */
static	char packages[9][128];		/* additional package names */
static	char packargs[9][128];		/* optional arg for package */
/* ---
 * <textarea name=TEXTAREANAME ...> in a <form>
 * -------------------------------------------- */
#if !defined(TEXTAREANAME)
  #define TEXTAREANAME "formdata"
#endif
/* ---
 * comma-separated list of HTTP_REFERER's allowed/denied to use mathTeX
 * -------------------------------------------------------------------- */
#if !defined(REFERER)
  #define REFERER "\000"
#endif
#if !defined(DENYREFERER)
  #define DENYREFERER "\000"
#endif
#if !defined(MAXINVALID)		/* longest length expression */
  #define MAXINVALID 0			/* from an invalid referer */
#endif					/* that will be rendered w/o error */
/* ---
 * time zone delta t (in hours)
 * ---------------------------- */
#if !defined(TZDELTA)
  #define TZDELTA 0
#endif
/* ---
 * default uses locatepath() if whichpath() fails
 * ---------------------------------------------- */
#if !defined(NOWHICH)
  #define ISWHICH 1
#else
  #define ISWHICH 0
#endif
#if !defined(NOLOCATE)
  #define ISLOCATE 1
#else
  #define ISLOCATE 0
#endif
/* ---
 * one image in every ADFREQUENCY will be wrapped inside "advertisement"
 * --------------------------------------------------------------------- */
#if !defined(ADFREQUENCY)
  #define ADFREQUENCY 0			/* never show advertisement if 0 */
#endif
static	int  adfrequency = ADFREQUENCY;	/* advertisement frequency */
#if !defined(HOST_SHOWAD)
  #define HOST_SHOWAD "\000"		/* show ads on all hosts */
#endif
/* ---
 * debugging and error reporting
 * ----------------------------- */
#if !defined(MSGLEVEL)
  #define MSGLEVEL 1
#endif
#if !defined(MAXMSGLEVEL)
  #define MAXMSGLEVEL 999999
#endif
static	int  msglevel = MSGLEVEL;	/* message level for verbose/debug */
static	FILE *msgfp = NULL;		/* output in command-line mode */
char	*strwrap();			/* help format debugging messages */
#define	showmsg(showlevel,label,data)	/* default message format */ \
	if ( msgfp!=NULL && msglevel>=(showlevel) ) { \
	  fprintf(msgfp,(strlen(label)+strlen(data)<64? \
	  "\nmathTeX> %s: %s\n" : "\nmathTeX> %s:\n         %s\n"), \
	  (label),strwrap((data),64,-9)); fflush(msgfp); } else
static	int  msgnumber = 0;		/* embeddedimages() in query mode */
#define	MAXEMBEDDED 15			/* 1...#embedded images available */
#define	TESTMESSAGE 1			/* msg# for mathTeX test message */
#define	UNKNOWNERROR 2			/* msg# for non-specific error */
#define	CACHEFAILED 3			/* msg# if mkdir cache failed */
#define	MKDIRFAILED 4			/* msg# if mkdir tempdir failed */
#define	CHDIRFAILED 5			/* msg# if chdir tempdir failed */
#define	FOPENFAILED 6			/* msg# if fopen(latex.tex) failed */
#define	SYLTXFAILED 7			/* msg# if system(latex) failed */
#define	LATEXFAILED 8			/* msg# if latex failed */
#define	SYPNGFAILED 9			/* msg# if system(dvipng) failed */
#define	DVIPNGFAILED 10			/* msg# if dvipng failed */
#define	SYPSFAILED 11			/* msg# if system(dvips) failed */
#define	DVIPSFAILED 12			/* msg# if dvips failed */
#define	SYCVTFAILED 13			/* msg# if system(convert) failed */
#define	CONVERTFAILED 14		/* msg# if convert failed */
#define	EMITFAILED 15			/* msg# if emitcache() failed */
static	char *embeddedtext[] = { NULL,	/* text of embedded image messages */
  "(1) mathTeX test message:\\"		/* msg#1 mathTeX "okay" test */
  "cgi program running okay.",
  "(2) mathTeX failed: probably due to\\" /* msg#2 general error message */
  "bad paths, permissions, or installation.",
  "(3) Can't mkdir cgi-bin/mathtex/\\"	/* msg#3 can't create cache dir */
  "cache directory: check permissions.",
  "(4) Can't mkdir cgi-bin/tempnam/\\"	/* msg#4 */
  "work directory: check permissions.",
  "(5) Can't cd cgi-bin/tempnam/\\"	/* msg#5 */
  "work directory: check permissions.",
  "(6) Can't fopen(\"latex.tex\") file:\\" /* msg#6 */
  "check permissions.",
  "(7) Can't run latex program:\\"	/* msg#7 */
  "check -DLATEX=\"path\", etc.",
  "(8) latex ran but failed:\\"		/* msg#8 */
  "check your input expression.",
  "(9) Can't run dvipng program:\\"	/* msg#9 */
  "check -DDVIPNG=\"path\", etc.",
  "(10) dvipng ran but failed:",	/* msg#10 */
  "(11) Can't run dvips program:\\"	/* msg#11 */
  "check -DDVIPS=\"path\", etc.",
  "(12) dvips ran but failed:",		/* msg#12 */
  "(13) Can't run convert program:\\"	/* msg#13 */
  "check -DCONVERT=\"path\", etc.",
  "(14) convert ran but failed:",	/* msg#14 */
  "(15) Can't emit cached image:\\"	/* msg#15 */
  "check permissions.",
  NULL } ; /* --- end-of-*embeddedtext[] --- */
/* ---
 * output file (from shell mode)
 * ----------------------------- */
static	char outfile[256] = "\000";	/* output file, or empty for default*/
/* ---
 * temporary work directory
 * ------------------------ */
static	char tempdir[256] = "\000";	/* temporary work directory */
/* ---
 * internal buffer sizes
 * --------------------- */
#if !defined(MAXEXPRSZ)
  #define MAXEXPRSZ (32767)		/*max #bytes in input tex expression*/
#endif
#if !defined(MAXGIFSZ)
  #define MAXGIFSZ (131072)		/* max #bytes in output GIF image */
#endif

/* ---
 * latex wrapper document template (default, isdepth=0, without depth)
 * ------------------------------------------------------------------- */
static	char latexdefaultwrapper[MAXEXPRSZ+16384] =
	"\\documentclass[10pt]{article}\n" /*[fleqn] omitted*/
	"\\usepackage[latin1]{inputenc}\n"
	"\\usepackage{amsmath}\n"
	"\\usepackage{amsfonts}\n"
	"\\usepackage{amssymb}\n"
	/*"\\usepackage{bm}\n"*/	/* bold math */
	#if defined(USEPACKAGE)		/* cc -DUSEPACKAGE=\"filename\" */
	  #include USEPACKAGE		/* filename with \usepackage{}'s */
	#endif				/* or anything for the preamble */
	"%%usepackage%%\n"
      #if 0
        "\\def\\stackboxes#1{\\vbox{\\def\\\\{\\egroup\\hbox\\bgroup}"
        "\\hbox\\bgroup#1\\egroup}}\n"
        "\\def\\fparbox#1{\\fbox{\\stackboxes{#1}}}\n"
      #endif
	"%%%\\pagestyle{empty}\n"
	"%%pagestyle%%\n"
	"%%previewenviron%%\n"
	"\\begin{document}\n"
	/*"\\setlength{\\mathindent}{0pt}\n"*/ /* only with [fleqn] option */
	"\\setlength{\\parindent}{0pt}\n"
      #if 0
	"%%%\\renewcommand{\\input}[1]"	/* don't let users \input{} */
	"{\\mbox{[[$\\backslash$input\\{#1\\} illegal]]}}\n"
      #endif
	#if defined(NEWCOMMAND)		/* cc -DNEWCOMMAND=\"filename\" */
	  #include NEWCOMMAND		/* filename with \newcommand{}'s */
	#endif				/* or anything for the document */
	"\\newcommand{\\amsatop}[2]{{\\genfrac{}{}{0pt}{1}{#1}{#2}}}\n"
	"\\newcommand{\\twolines}[2]{{\\amsatop{\\mbox{#1}}{\\mbox{#2}}}}\n"
	"\\newcommand{\\fs}{{\\eval{fs}}}\n" /* \eval{} test */
	"%%fontsize%%\n"
	"%%setlength%%\n"
	"%%beginmath%% "
	"%%expression%% \n"		/* \n in case expression contains %*/
	" %%endmath%%\n"
	"\\end{document}\n";

/* ---
 * latex wrapper document template (optional, isdepth=1, with depth)
 * see http://www.mactextoolbox.sourceforge.net/articles/baseline.html
 * for discussion of this procedure
 * ------------------------------------------------------------------- */
static	char latexdepthwrapper[MAXEXPRSZ+16384] =
	"\\documentclass[10pt]{article}\n" /*[fleqn] omitted*/
	"\\usepackage[latin1]{inputenc}\n"
	"\\usepackage{amsmath}\n"
	"\\usepackage{amsfonts}\n"
	"\\usepackage{amssymb}\n"
	"%%%\\usepackage{calc}\n"
	#if defined(USEPACKAGE)		/* cc -DUSEPACKAGE=\"filename\" */
	  #include USEPACKAGE		/* filename with \usepackage{}'s */
	#endif				/* or anything for the preamble */
	"%%usepackage%%\n"
	#if defined(NEWCOMMAND)		/* cc -DNEWCOMMAND=\"filename\" */
	  #include NEWCOMMAND		/* filename with \newcommand{}'s */
	#endif				/* or anything for the document */
	"\\newcommand{\\amsatop}[2]{{\\genfrac{}{}{0pt}{1}{#1}{#2}}}\n"
	"\\newcommand{\\twolines}[2]{{\\amsatop{\\mbox{#1}}{\\mbox{#2}}}}\n"
	"\\newcommand{\\fs}{{\\eval{fs}}}\n" /* \eval{} test */
	"%%pagestyle%%\n"
	"%%previewenviron%%\n"
	"\\newsavebox{\\mybox}\n"
	"\n"
	"\\newlength{\\mywidth}\n"
	"\\newlength{\\myheight}\n"
	"\\newlength{\\mydepth}\n"
	"\n"
	/*"\\setlength{\\mathindent}{0pt}\n"*/ /* only with [fleqn] option */
	"\\setlength{\\parindent}{0pt}\n"
	"%%fontsize%%\n"
	"%%setlength%%\n"
	"\n"
	"\\begin{lrbox}{\\mybox}\n"
	"%%beginmath%% "
	"%%expression%% \n"		/* \n in case expression contains %*/
	" %%endmath%%\n"
	"\\end{lrbox}\n"
	"\n"
	"\\settowidth {\\mywidth}  {\\usebox{\\mybox}}\n"
	"\\settoheight{\\myheight} {\\usebox{\\mybox}}\n"
	"\\settodepth {\\mydepth}  {\\usebox{\\mybox}}\n"
	"\n"
	"\\newwrite\\foo\n"
	"\\immediate\\openout\\foo=\\jobname.info\n"
	"    \\immediate\\write\\foo{depth = \\the\\mydepth}\n"
	"    \\immediate\\write\\foo{height = \\the\\myheight}\n"
	"    \\addtolength{\\myheight} {\\mydepth}\n"
	"    \\immediate\\write\\foo{totalheight = \\the\\myheight}\n"
	"    \\immediate\\write\\foo{width = \\the\\mywidth}\n"
	"\\closeout\\foo\n"
	"\n"
	"\\begin{document}\n"
	"\\usebox{\\mybox}\n"
	"\\end{document}\n";
/* ---
 * latex wrapper used
 * ------------------ */
static	char *latexwrapper =		/* with or without depth */
	( ISDEPTH? latexdepthwrapper : latexdefaultwrapper );
/* ---
 * elements from \jobname.info to be prepended to graphics file
 * ------------------------------------------------------------ */
#define	IMAGEINFO struct imageinfo_struct /*"typedef" for imageinfo struct*/
#define	MAXIMAGEINFO 32			/* max 32 image info elements */
IMAGEINFO {
  char	*identifier;			/* identifier in \jobname.info */
  char	*format;			/* format to write in graphics file*/
  double value;				/* value of identifier */
  char	units[32];			/* units of value, e.g., "pt" */
  int	algorithm;			/* value conversion before writing */
  } ; /* --- end-of-store_struct --- */
static IMAGEINFO imageinfo[MAXIMAGEINFO] = {
    { "depth", "Vertical-Align:%dpx\n", -9999., "", 1 }, /* below baseline */
    { NULL, NULL, -9999., "", -9999 }	/* end-of-imageinfo */
  } ; /* --- end-of-imageinfo[] --- */

/* -------------------------------------------------------------------------
Unix or Windows header files
-------------------------------------------------------------------------- */
/* ---
 * compiling under Windows if -DWINDOWS explicitly supplied or...
 * -------------------------------------------------------------- */
#if !defined(WINDOWS)		/* -DWINDOWS not explicitly given by user */
  #if defined(_WINDOWS) || defined(_WIN32) || defined(WIN32) \
  ||  defined(DJGPP)		/* try to recognize windows compilers */ \
  ||  defined(_USRDLL)		/* must be WINDOWS if compiling for DLL */
    #define WINDOWS		/* signal windows */
  #endif
#endif
/* ---
 * unix headers
 * ------------ */
#if !defined(WINDOWS)		/* if not compiling under Windows... */
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
#endif
/* ---
 * windows-specific header info
 * ---------------------------- */
#if defined(WINDOWS)		/* Windows opens stdout in char mode, and */
  #include <fcntl.h>		/* precedes every 0x0A with spurious 0x0D.*/
  #include <io.h>		/* So emitcache() issues a Win _setmode() */
				/* call to put stdout in binary mode. */
  #if defined(_O_BINARY) && !defined(O_BINARY)  /* only have _O_BINARY */
    #define O_BINARY _O_BINARY	/* make O_BINARY available, etc... */
    #define setmode  _setmode
    #define fileno   _fileno
  #endif
  #if defined(_O_BINARY) || defined(O_BINARY)  /* setmode() now available */
    #define HAVE_SETMODE	/* so we'll use setmode() */
  #endif
#endif
static	int iswindows =		/* 1 if running under Windows, or 0 if not */
  #ifdef WINDOWS
    1;				/* 1 when program running under Windows */
  #else
    0;				/* 0 when not running under Windows */
  #endif

/* -------------------------------------------------------------------------
application macros
-------------------------------------------------------------------------- */
/* --- check if a string is empty --- */
#define	isempty(s)  ((s)==NULL?1:(*(s)=='\000'?1:0))
/* --- last char of a string --- */
#define	lastchar(s) (isempty(s)?'\000':*((s)+(strlen(s)-1)))
/* --- check for thischar in accept --- */
#define	isthischar(thischar,accept) \
	( (thischar)!='\000' && !isempty(accept) \
	&& strchr((accept),(thischar))!=(char *)NULL )
/* --- skip/find whitespace --- */
#define	WHITESPACE  " \t\n\r\f\v"	/* skipped whitespace chars */
#define	skipwhite(thisstr)  if ( (thisstr) != NULL ) \
	thisstr += strspn(thisstr,WHITESPACE)
#define	findwhite(thisstr)  while ( !isempty(thisstr) ) \
	if ( isthischar(*(thisstr),WHITESPACE) ) break; else (thisstr)++
	/* thisstr += strcspn(thisstr,WHITESPACE) */
/* --- skip \command (i.e., find char past last char of \command) --- */
#define	skipcommand(thisstr)  while ( !isempty(thisstr) ) \
	if ( !isalpha(*(thisstr)) ) break; else (thisstr)++
/* --- strncpy() n bytes and make sure it's null-terminated --- */
#define	strninit(target,source,n) if( (target)!=NULL && (n)>=0 ) { \
	  char *thissource = (source); \
	  (target)[0] = '\000'; \
	  if ( (n)>0 && thissource!=NULL ) { \
	    strncpy((target),thissource,(n)); \
	    (target)[(n)] = '\000'; } }
/* --- strip leading and trailing whitespace --- */
#define	trimwhite(thisstr) if ( (thisstr) != NULL ) { \
	int thislen = strlen(thisstr); \
	while ( --thislen >= 0 ) \
	  if ( isthischar((thisstr)[thislen],WHITESPACE) ) \
	    (thisstr)[thislen] = '\000'; \
	  else break; \
	if ( (thislen = strspn((thisstr),WHITESPACE)) > 0 ) \
	  {strsqueeze((thisstr),thislen);} } else
/* --- strcpy(s,s+n) using memmove() (also works for negative n) --- */
#define	strsqueeze(s,n) if((n)!=0) { if(!isempty((s))) { \
	int thislen3=strlen(s); \
	if ((n) >= thislen3) *(s) = '\000'; \
	else memmove((s),(s)+(n),1+thislen3-(n)); }} else /*user supplies ;*/
/* --- strsqueeze(s,t) with two pointers --- */
#define	strsqueezep(s,t) if(!isempty((s))&&!isempty((t))) { \
	int sqlen=strlen((s))-strlen((t)); \
	if (sqlen>0 && sqlen<=999) {strsqueeze((s),sqlen);} } else
/* --- min and max of args --- */
#define	max2(x,y)  ((x)>(y)? (x):(y))	/* larger of 2 arguments */
#define	min2(x,y)  ((x)<(y)? (x):(y))	/* smaller of 2 arguments */

/* -------------------------------------------------------------------------
other application global data
-------------------------------------------------------------------------- */
/* --- getdirective() global data --- */
static	int  argformat   = 0;		/* 111... if arg not {}-enclosed */
static	int  optionalpos = 0;		/* # {args} before optional [args] */
static	int  noptional   = 0;		/* # optional [args] found */
static	char optionalargs[8][512] =	/* buffer for optional args */
  { "\000", "\000", "\000", "\000", "\000", "\000", "\000", "\000" };

/* -------------------------------------------------------------------------
store for evalterm() [n.b., these are stripped-down funcs from nutshell]
-------------------------------------------------------------------------- */
#define	STORE struct store_struct	/* "typedef" for store struct */
#define	MAXSTORE 100			/* max 100 identifiers */
STORE {
  char	*identifier;			/* identifier */
  int	*value;				/* address of corresponding value */
  } ; /* --- end-of-store_struct --- */
static STORE mathtexstore[MAXSTORE] = {
    { "fontsize", &fontsize },	{ "fs", &fontsize },	/* font size */
    /*{ "mytestvar", &mytestvar },*/
    { NULL, NULL }					/* end-of-store */
  } ; /* --- end-of-mimestore[] --- */


/* ==========================================================================
 * Function:	main ( argc, argv )
 * Purpose:	driver for mathtex.c
 *		emits, usually to stdout, a gif or png image
 *		of a LaTeX math expression entered either as
 *		    (1)	html query string from a browser (most typical), or
 *		    (2)	a query string from an html <form method="get">
 *			whose <textarea name=TEXTAREANAME>
 *			(usually for demo), or
 *		    (3)	command-line arguments (usually just to test).
 *		If no input supplied, expression defaults to "f(x)=x^2",
 *		treated as test (input method 3).
 * --------------------------------------------------------------------------
 * Command-Line Arguments:
 *		When running mathTeX from the command-line, rather than
 *		from a browser (usually just for testing), syntax is
 *		     ./mathtex	[-m msglevel]	verbosity of debugging output
 *				[-c cachepath ]	name of cache directory
 *				[expression	expression, e.g., x^2+y^2,
 *		-m   0-99, controls verbosity level for debugging output
 *		     >=9 retains all directories and files created
 *		-c   cachepath specifies relative path to cache directory
 * --------------------------------------------------------------------------
 * Exits:	0=success (always exits 0, regardless of success/failure)
 * --------------------------------------------------------------------------
 * Notes:     o For an executable that emits gif images
 *		     cc mathtex.c -o mathtex.cgi
 *		or, alternatively, for an executable that emits png images
 *		     cc -DPNG mathtex.c -o mathtex.cgi
 *		See Notes at top of file for other compile-line -D options.
 *	      o	Move executable to your cgi-bin directory and either
 *		point your browser to it directly in the form
 *		     http://www.yourdomain.com/cgi-bin/mathtex.cgi?f(x)=x^2
 *		or put a tag in your html document of the form
 *		     <img src="/cgi-bin/mathtex.cgi?f(x)=x^2"
 *		       border=0 align=middle>
 *		where f(x)=x^2 (or any other expression) will be displayed
 *		either as a gif or png image (as per -DIMAGETYPE flag).
 * ======================================================================= */
/* --- entry point --- */
int	main ( int argc, char *argv[] )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- expression to be emitted --- */
static	char exprbuffer[MAXEXPRSZ+1] = "\000"; /* input TeX expression */
char	hashexpr[MAXEXPRSZ+1] = "\000";	/* usually use md5 of original expr*/
char	*expression = exprbuffer;	/* ptr to expression */
char	*query = getenv("QUERY_STRING"); /* getenv("QUERY_STRING") result */
int	isquery = 0;			/* true if input from QUERY_STRING */
/* --- preprocess expression for special mathTeX directives, etc --- */
char	*mathprep();			/* preprocess expression */
int	unescape_url();			/* convert %20 to blank space, etc */
int	strreplace(), irep=0;		/* look for keywords in expression */
char	*strchange();			/* edit expression */
char	*getdirective(), argstring[256],/*look for \density,\usepackage,etc*/
	*pdirective = NULL;		/* ptr to char after \directive */
int	validate();			/* remove \input, etc */
int	advertisement(), crc16();	/*wrap expression in advertisement*/
int	evalterm();			/* preprocess \eval{expression} */
char	*adtemplate = NULL;		/* usually use default message */
char	*host_showad = HOST_SHOWAD;	/* show ads only on this host */
char	*nomath();			/* remove math chars from string */
/* --- referer initialization variables --- */
char	*http_referer = getenv("HTTP_REFERER"), /* referer using mathTeX */
	*mathtex_host = getenv("HTTP_HOST"),	/* http host for mathTeX */
	*server_name  = getenv("SERVER_NAME"),	/* server hosting mathTeX */
	*http_host = (!isempty(mathtex_host)?mathtex_host: /*match host*/
	  (!isempty(server_name)?server_name:(NULL))); /* or server_name */
int	isvalidreferer = 1;		/* can this referer use mathTeX? */
struct	{ char *deny;			/* http_referer can't contain this */
	int msgnumber; }		/* emit invalid_referer_msg[msg#] */
	htaccess[] = {			/* ".htaccess" table for deny */
	#ifdef HTACCESS			/* eg, -DHTACCESS=\"htaccess.txt\" */
	  #include HTACCESS		/* eg,  {"",1},  for no referer */
	#endif
	{ NULL, -999 } };		/* trailer */
/* --- other initialization variables --- */
char	*whichpath();			/* look for latex,dvipng,etc */
int	setpaths();			/* set paths to latex,dvipng,etc */
int	isstrstr();			/* find any snippet in string */
int	isdexists();			/* check whether cache dir exists */
int	perm_all = (S_IRWXU|S_IRWXG|S_IRWXO); /* 777 permissions */
int	readcachefile(), nbytes=0;	/*read expr for -f command-line arg*/
char	*timestamp(),			/*for \time (in addition to \today)*/
	*calendar();			/* for \calendar{yyyy,mm,dd} string*/
int	emitembedded();			/* in case latex can't run */
int	timelimit();			/* just to check stub or built-in */
int	iscolorpackage = 0,		/* true if \usepackage{color} found*/
	iseepicpackage = 0,		/* true if \usepackage{eepic} found*/
	ispict2epackage = 0,		/* true if \usepackage{pict2e}found*/
	ispreviewpackage = 0;		/* true if \usepackage{preview} */
/* --- image rendering --- */
int	mathtex();			/* generate new image using LaTeX */
/* --- image caching --- */
int	rewritecache();			/* rewrite cachefile with imageinfo*/
int	emitcache();			/* emit cached image if it exists */
int	maxage = MAXAGE;		/* max-age is typically two hours */
char	*makepath();			/* construct full path/filename.ext*/
char	*md5str(), *md5hash=NULL;	/* md5 has of expression */
int	mathlog();			/* log requests */
/* --- messages --- */
char	*copyright1 =			/* copyright, gnu/gpl notice */
 "+-----------------------------------------------------------------------+\n"
 "|mathTeX vers " VERSION ", Copyright(c) " COPYRIGHTDATE
 ", John Forkosh Associates, Inc|\n"
 "+-----------------------------------------------------------------------+\n"
 "| mathTeX is free software, licensed to you under terms of the GNU/GPL  |\n"
 "|           and comes with absolutely no warranty whatsoever.           |\n",
 *copyright2 =				/* second part of copyright */
 "|     See http://www.forkosh.com/mathtex.html for complete details.     |\n"
 "+-----------------------------------------------------------------------+";
char	*usage1 =			/* usage instructions */
 "Command-line Usage:\n"
 "  ./mathtex.cgi \"expression\"     expression, e.g., \"x^2+y^2\"\n"
 "            | -f input_file      or read expression from input_file\n"
 "            [ -o output_file ]   write image to ouput_file\n"
 "            [ -m msglevel ]      verbosity / message level\n"
 "            [ -c cache ]         path to cache directory\n",
 *usage2 =				/* second part of usage */
 "Example:\n"
 "  ./mathtex.cgi \"x^2+y^2\" -o equation1\n"
 "creates equation1.gif containing image of x^2+y^2\n";
char	*versiontemplate =		/* mathTeX version "adtemplate" */
 "\\begin{center}\n"
 "\\fbox{\\footnotesize $\\twolines{mathTeX version " VERSION
 ",\\ \\ revised " REVISIONDATE "}{Copyright (c) " COPYRIGHTDATE
 ", John Forkosh Associates, Inc.}$}\\\\ \\vspace*{-.2in}"
 "%%beginmath%% %%expression%% %%endmath%%\n"
 "\\end{center}\n";
char	whichtemplate[512] =		/* mathTeX which "adtemplate" */
 "\\begin{center}\n"
 "\\fbox{\\footnotesize %%whichpath%%}\\\\ \\vspace*{-.2in}"
 "%%beginmath%% %%expression%% %%endmath%%\n"
 "\\end{center}\n";
int	maxinvalidmsg = 2;		/* max invalid_referer_msg[] index */
char	*invalid_referer_msg[] = {	/* invalid referer messages */
 /* --- message#0 (default invalid_referer message) --- */
 "\\parstyle\\usepackage{color}\\small\\color{red}\\noindent "
 "\\fbox{$\\twolines{\\textbf{%%referer%%}}"
 "{\\textbf{is not authorized to use mathTeX on this server}}$}",
 /* --- message#1 ("provide http_referer" message) --- */
 "\\parstyle\\usepackage{color}\\small\\color{red}\\noindent "
 "\\fbox{$\\twolines{\\textbf{To use the public math\\TeX{} server,}}"
 "{\\textbf{please provide your HTTP\\_REFERER}}$}",
 /* --- message#2 ("production use" message) --- */
 "\\parstyle\\usepackage{color}\\small\\color{red}\\noindent "
 "\\fbox{$\\twolines{\\textbf{For production use, please install mathtex.cgi on}}"
 "{\\textbf{your own server.  See www.forkosh.com/mathtex.html}}$}",
 /* --- end-of-table trailer --- */
 NULL } ; /* --- end-of-invalid_referer_msg[] --- */
/* -------------------------------------------------------------------------
Initialization
-------------------------------------------------------------------------- */
/* --- set global variables --- */
msgfp = NULL;				/* for query mode output */
msgnumber = 0;				/* no errors to report yet */
if ( imagetype < 1 || imagetype > 2 ) imagetype = 1;   /* keep in bounds */
if ( imagemethod<1 || imagemethod>2 ) imagemethod = 1; /* keep in bounds */
/* ---
 * check QUERY_STRING query for expression
 * --------------------------------------- */
if ( query != NULL )			/* check query string from environ */
  if ( strlen(query) >= 1 )		/* caller gave us a query string */
    { strninit(expression,query,MAXEXPRSZ); /* so use it as expression */
      isquery = 1; }			/* and set isquery flag */
if ( !isquery )				/* empty query string */
  { char *addr = getenv("SERVER_ADDR");	/* additional getenv("") results */
    if ( !isempty(http_host) || addr!=NULL ) /* assume http query */
      {	isquery = 1;			/* set flag to signal query */
	strcpy(expression,"\\fbox{\\rm No expression supplied}"); }
  } /* --- end-of-if(!isquery) --- */
/* ---
 * process command-line args (only if not a query)
 * ----------------------------------------------- */
if ( !isquery				/* don't have an html query string */
/*&&   argc>1*/ ) {			/* and have command-line args */
  int	argnum = 0;			/*argv[] index for command-line args*/
  int	isendofswitches = 0;		/* -- arg signals no more -switches */
  msglevel = 3;				/*adjust minimum shell message level*/
  msgfp = stdout;			/* for comamnd-line mode output */
  while ( argc > ++argnum ) {		/*check for switches and expression*/
    if ( *argv[argnum] == '-'		/* have some -switch */
    &&   !isendofswitches ) {		/* and haven't seen a -- arg yet*/
      char *field = argv[argnum] + 1;	/* ptr to char(s) following - */
      char flag = tolower((int)(*field)); /* single char following - */
      field++;				/* bump past flag */
      trimwhite(field);			/*remove leading/trailing whitespace*/
      if ( isempty(field) ) {		/* no chars after -flag char, so */
        if ( flag == '-' )		/* have -- arg, so */
          isendofswitches = 1;		/* signal no more -switches */
        else				/* have a single char flag, so */
          field = argv[++argnum]; }	/* arg following flag is its value */
      switch ( flag ) {			/* see what user wants to tell us */
	/* --- unrecognized -switch flag --- */
	default: /*argnum--;*/  break;
	/* --- interpret recognized flags --- */
	case 'c': strcpy(cachepath,field);
		if ( strlen(cachepath) < 1  /* path is an empty string */
		||   strcmp(cachepath,"none") == 0 )  /* or keyword "none" */
		  iscaching = 0;	/* so disable caching */
		break;
	case 'm': msglevel = atoi(field);   break;
	case 'f': nbytes = readcachefile(field,(unsigned char *)exprbuffer);
	        exprbuffer[nbytes] = '\000'; /* null-terminate expression */
		if ( !isempty(outfile) ) break; /* default to same outfile */
	case 'o': strcpy(outfile,field); /* output file for image */
	        trimwhite(outfile);	/*remove leading/trailing whitespace*/
	        break;
	} /* --- end-of-switch(flag) --- */
      } /* --- end-of-if(*argv[argnum]=='-') --- */
    else				/* expression if arg not a -flag */
      strcpy(exprbuffer,argv[argnum]);	/* take last unswitched arg */
    } /* --- end-of-while(argc>++argnum) --- */
  if ( isempty(expression) ) {		/*no expression, show usage and quit*/
    if ( msgfp!=NULL && msglevel>=3 )	/* user needs usage instructions */
      fprintf(msgfp,"%s%s\n%s%s\n",copyright1,copyright2, /*show copyright*/
      usage1,usage2);			/* and show usage */
    goto end_of_job; }			/* quit */
  } /* --- end-of-if(!isquery) --- */
/* ---
 * pre-process expression
 * ---------------------- */
if ( !isempty(TEXTAREANAME) )		/* if <form>'s are allowed */
 if ( isquery )				/*check for <form> on query strings*/
  if(memcmp(expression,TEXTAREANAME,strlen(TEXTAREANAME))==0) { /*have form*/
    char *delim = strchr(expression,'='); /* find = following TEXTAREANAME */
    if ( delim != (char *)NULL )	/* found unescaped equal sign */
      {strsqueeze(expression,((int)(delim-expression)+1));} /*squeeze name=*/
    while ( (delim=strchr(expression,'+')) != NULL ) /*unescaped plus sign*/
      *delim = ' ';			/* is "shorthand" for blank space */
    unescape_url(expression);		/*convert %20, etc (twice for form)*/
    } /* --- end-of-if(memcmp()==0) --- */
unescape_url(expression);		/* convert %20 to blank space, etc */
mathprep(expression);			/* convert &lt; to <, etc */
validate(expression);			/* remove \input, etc */
/* ---
 * check if this http_referer is allowed to use mathTeX
 * ---------------------------------------------------- */
msgnumber = 0;				/* default invalid message number */
/* --- see if user fails to match -DREFERER list of valid domains --- */
if ( isquery )				/* not relevant if in command mode */
  if ( !isempty(REFERER) )		/* nor if compiled w/o -DREFERER= */
    if ( !isstrstr(http_referer,REFERER,0) ) /* invalid http_referer */
      isvalidreferer = 0;		/* signal invalid referer */
/* --- see if user matches -DDENYREFERER list of invalid domains --- */
if ( isquery )				/* not relevant if in command mode */
  if ( isvalidreferer )			/* or if alteady invalid */
    if ( !isempty(DENYREFERER) )	/*nor if compiled w/o -DDENYREFERER*/
      if ( isstrstr(http_referer,DENYREFERER,0) ) /* invalid http_referer */
        isvalidreferer = 0;		/* signal invalid referer */
/* --- see if user matches -DHTACCESS list of invalid domains --- */
if ( isquery )				/* not relevant if in command mode */
  if ( isvalidreferer ) {		/* or if alteady invalid */
    int	iaccess=0;			/* htaccess[] index */
    msgnumber = (-999);			/* 0 or positive if match found */
    for ( iaccess=0; msgnumber<0; iaccess++ ) { /*run thru htaccess[] table*/
      char *deny = htaccess[iaccess].deny; /* referer to be denied */
      if ( deny == NULL ) break;	/* null signals end-of-table */
      if ( isempty(deny) )		/* signal to check for no referer */
	{ if ( isempty(http_referer) )	/* http_referer not supplied */
	   msgnumber = htaccess[iaccess].msgnumber; } /* so set message# */
      else				/* have referer to check for */
       if ( !isempty(http_referer) )	/* and have referer to be checked */
	if ( isstrstr(http_referer,deny,0) ) /* invalid http_referer */
	 msgnumber = htaccess[iaccess].msgnumber; /* so set message# */
      } /* --- end-of-for(iaccess) --- */
    if ( msgnumber >= 0 )		/* deny access to this referer */
      isvalidreferer = 0; }		/* signal invalid referer */
/* --- render short expressions even for invalid referers --- */
if ( !isvalidreferer )			/* have an invalied referer */
  if ( MAXINVALID > 0 )			/* but short expressions allowed */
    if ( strlen(expression) <= MAXINVALID ) /* and this one is short enough*/
      isvalidreferer = 1;		/* so let it through */
/* --- check for invalidreferer directive (user wants to see message)  --- */
if ( getdirective(expression,"\\invalidreferer",1,0,1,argstring)
!=   NULL ) {				/* found \invalidreferer directive */
  isvalidreferer = 0;			/* signal invalid referer */
  msgnumber = atoi(argstring); }	/* requested message number */
/* --- substitute invalid referer message if referer is indeed invalid --- */
if ( !isvalidreferer ) {		/* invalid referer detected */
  if ( msgnumber<0 || msgnumber>maxinvalidmsg ) /* check message range */
    msgnumber = 0;			/* default if out-of-bounds */
  strcpy(expression,invalid_referer_msg[msgnumber]); /* choose message */
  strreplace(expression,"%%referer%%",	/* replace actual referer */
    (isempty(http_referer)?"your domain":nomath(http_referer)),0,0); }
msgnumber = 0;				/* reset global message number */
/* ---
 * check for embedded image \message directive (which supercedes everything)
 * ----------------------------------------------------------------------- */
if ( getdirective(expression,"\\message",1,0,1,argstring)
!=   NULL ) {				/* found \message directive */
  if ( msgfp!=NULL && msglevel>=1 )	/* copyright notice */
    fprintf(msgfp,"%s%s\n",copyright1,copyright2); /* always show copyright*/
  msgnumber = atoi(argstring);		/* requested message number */
  emitembedded(msgnumber,(isquery?2:0)); /* emit image, suppress msgfp */
  goto end_of_job; }			/*nothing to do after emitting image*/
/* ---
 * check for \switches directive (which supercedes everything else)
 * ---------------------------------------------------------------- */
if ( strreplace(expression,"\\switches","",0,0) /* remove \switches */
>=   1 ) {				/* found \switches */
  char	*pathsource[] = { "default", "switch", "which", "locate" };
  /*iscaching = 0;*/			/* don't cache \switches image */
  *expression = '\000';			/* reset expression */
  /*setpaths(imagemethod);*/		/* set paths */
  setpaths(0);				/* show _all_ set paths */
  strcat(expression,"\\parstyle");	/* set paragraph mode */
  strcat(expression,"\\small\\tt");	/* set font,size */
  strcat(expression,"\\fparbox{");	/* emit -Dswitches in framed box */
  strcat(expression,"Program image...\\\\\n"); /* image */
  sprintf(expression+strlen(expression),"%s\\\\",argv[0]);
  strcat(expression,"Paths...\\\\\n");	/* paths */
  sprintf(expression+strlen(expression), /* latex path */
    "-DLATEX=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    latexpath,pathsource[islatexpath]);
  sprintf(expression+strlen(expression), /* pdflatex path */
    "-DPDFLATEX=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    pdflatexpath,pathsource[ispdflatexpath]);
  sprintf(expression+strlen(expression), /* dvipng path */
    "-DDVIPNG=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    dvipngpath,pathsource[isdvipngpath]);
  sprintf(expression+strlen(expression), /* dvips path */
    "-DDVIPS=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    dvipspath,pathsource[isdvipspath]);
  sprintf(expression+strlen(expression), /* ps2epsi path */
    "-DPS2EPSI=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    ps2epsipath,pathsource[isps2epsipath]);
  sprintf(expression+strlen(expression), /* convert path */
    "-DCONVERT=$\\backslash$\"%s$\\backslash$\" \\ (%s)\\\\ \n",
    convertpath,pathsource[isconvertpath]);
  strcat(expression,"}");		/* end-of-\fparbox{} */
  } /* --- end-of-if(strreplace("\\switches")>=1) --- */
else					/* no \switches in expression */
 /* ---
  * check for \environment directive (which supercedes everything else)
  * ------------------------------------------------------------------- */
 if ( strreplace(expression,"\\environment","",0,0) /* remove \environment */
 >=   1 ) {				/* found \environment */
  int	ienv = 0;			/* environ[] index */
  /*iscaching = 0;*/			/* don't cache \environment image */
  *expression = '\000';			/* reset expression */
  setpaths(10*latexmethod+imagemethod);	/* set paths */
  strcat(expression,"\\parstyle");	/* set paragraph mode */
  strcat(expression,"\\scriptsize\\tt"); /* set font,size */
  strcat(expression,"\\noindent");	/* don't indent first line */
  strcat(expression,"\\begin{verbatim}"); /* begin verbatim environment */
  for ( ienv=0; ; ienv++ ) {		/* loop over environ[] strings */
    if ( environ[ienv] == (char *)NULL ) break; /* null terminates list */
    if ( *(environ[ienv]) == '\000' ) break; /* double-check empty string */
    sprintf(expression+strlen(expression), /* display environment string */
    "  %2d. %s \n", ienv+1,strwrap(environ[ienv],50,-6));
    /* "%2d.\\ \\ %s\\ \\\\ \n", ienv+1,nomath(environ[ienv])); */
    } /* --- end-of-for(ienv) --- */
  strcat(expression,"\\end{verbatim}");	/* end verbatim environment */
  } /* --- end-of-if(strreplace("\\environment")>=1) --- */
/* ---
 * save copy of expression before further preprocessing for MD5 hash
 * ------------------------------------------------------------------ */
strcpy(hashexpr,expression);		/* save unmodified expr for hash */
/* ---
 * check for \which directive (supercedes everything not above)
 * ------------------------------------------------------------ */
if ( getdirective(expression,"\\which",1,0,1,argstring)
!=   NULL ) {				/* found \which directive */
  int	ispermitted = 1;		/* true if a legitimate request */
  int	nlocate = 1;			/* use locate if which fails */
  char	*path = NULL;			/* whichpath() to argstring program*/
  char	whichmsg[512];			/* displayed message */
  trimwhite(argstring);			/*remove leading/trailing whitespace*/
  if ( isempty(argstring) ) ispermitted = 0; /* arg is an empty string */
  else {				/* have non-empty argstring */
    int	arglen = strlen(argstring);	/* #chars in argstring */
    if ( strcspn(argstring,WHITESPACE) < arglen /* embedded whitespace */
    ||   strcspn(argstring,"{}[]()<>") < arglen /* illegal char */
    ||   strcspn(argstring,"|/\"\'\\") < arglen /* illegal char */
    ||   strcspn(argstring,"`!@%&*+=^")< arglen /* illegal char */
    ) ispermitted = 0;			/*probably not a legitimate request*/
    } /* --- end-of-if/else(isempty(argstring)) --- */
  if ( ispermitted ) {			/* legitimate request */
    path = whichpath(argstring,&nlocate); /* path to argstring program */
    sprintf(whichmsg,			/* display path or "not found" */
      "%s(%s) = %s", (path==NULL||nlocate<1?"which":"locate"),
       argstring,(path!=NULL?path:"not found")); }
  else					/* display "not permitted" message */
    sprintf(whichmsg,"which(%s) = not permitted", argstring);
  strreplace(whichtemplate,"%%whichpath%%",whichmsg,0,0); /*insert message*/
  adtemplate = whichtemplate;		/* set which message */
  adfrequency = 1;			/* force its display */
  /*iscaching = 0;*/			/* don't cache it */
  if ( path != NULL )			/* change \ to $\backslash$ */
    /*strreplace(path,"\\","$\\backslash$",0,0)*/; /* make \ displayable */
  /*strcpy(expression,"\\parstyle\\small\\tt ");*/ /* re-init expression */
  /*strcat(expression,"\\fparbox{");*/	/* emit path in framed box */
  /*sprintf(expression+strlen(expression),*/ /* display path or "not found" */
    /*"which(%s) = %s",argstring,(path!=NULL?path:"not found"));*/
  /*strcat(expression,"}");*/		/* end-of-\fparbox{} */
  } /* --- end-of-if(getdirective("\\which")!=NULL) --- */
/* ---
 * check for picture environment, i.e., \begin{picture}, but don't remove it
 * ----------------------------------------------------------------------- */
if ( strstr(expression,"picture") != NULL ) /* picture environment used */
  ispicture = 1;			/* signal picture environment */
if ( strreplace(expression,"\\nopicture","",0,0) /* remove \nopicture */
>=   1 ) ispicture = 0;			/* user wants to handle picture */
if ( ispicture ) {			/* set picture environment defaults*/
  imagemethod = 2;			/* must use convert, not dvipng */
  mathmode = 2;				/* must be in paragraph mode */
  isdepth = 0;				/* reset default in case it's true */
  if ( !ISGAMMA ) strcpy(gamma,CONVERTGAMMA); /* default convert gamma */
  } /* --- end-of-if(ispicture) --- */
/* ---
 * check for embedded directives, remove them and set corresponding values
 * ----------------------------------------------------------------------- */
/* --- first (re)set default modes required for certain environments --- */
if ( strstr(expression,"gather") != NULL ) /* gather environment used */
  mathmode = 2;				/* need paragraph style for gather */
if ( strstr(expression,"eqnarray") != NULL ) /* eqnarray environment used */
  mathmode = 2;				/*need paragraph style for eqnarray*/
/* --- check for explicit displaystyle/textstyle/parstyle directives --- */
if ( strreplace(expression,"\\displaystyle","",0,0) /*remove \displaystyle*/
>=   1 ) mathmode = 0;			/* found \displaystyle so set flag*/
if ( strreplace(expression,"\\textstyle","",0,0) /* remove \textstyle */
>=   1 ) mathmode = 1;			/* found \textstyle so reset flag */
if ( strreplace(expression,"\\parstyle","",0,0) /* remove \parstyle */
>=   1 ) mathmode = 2;			/* found \parstyle so reset flag */
if ( strreplace(expression,"\\parmode","",0,0) /*\parmode same as \parstyle*/
>=   1 ) mathmode = 2;			/* found \parmode so reset flag */
/* --- check for quiet/halt directives (\quiet or \halt) --- */
if ( strreplace(expression,"\\quiet","",0,0) /*remove occurrences of \quiet*/
>=   1 ) isquiet = 64;			/* found \quiet so set isquiet flag*/
if ( strreplace(expression,"\\noquiet","",0,0) /* remove \noquiet */
>=   1 ) isquiet = 0;			/* found \noquiet so reset flag */
if ( getdirective(expression,"\\nquiet",1,0,1,argstring) /*\nquiet{#Enters}*/
!=   NULL ) {				/* found \nquiet{#<Enter>'s} */
  isquiet = atoi(argstring); }		/* interpret arg as isquiet value */
/* --- check for program paths --- */
if ( getdirective(expression,"\\convertpath",1,0,1,argstring) /*convert path*/
!=   NULL ) {				/* \convertpath{/path/to/convert} */
  strcpy(convertpath,argstring);	/*interpret arg as /path/to/convert*/
  if ( strstr(convertpath,"convert") == NULL ) { /* just path without name */
    if ( lastchar(convertpath) != '/' )	/* not even a / at end of path */
      strcat(convertpath,"/");		/* so add a / */
    strcat(convertpath,"convert"); }	/* add name of convert program */
  isconvertpath = 1; }			/* treat like -DCONVERT switch */
/* --- check for fontsize directives (\tiny ... \Huge) --- */
if ( 1 || mathmode != 2 )		/*latex sets font size in \parstyle*/
  for (irep=1; !isempty(sizedirectives[irep]); irep++) /*1=\tiny...10=\Huge*/
    if ( strstr(expression,sizedirectives[irep]) != NULL ) { /*found \size*/
      if ( mathmode != 2 )		/* not in paragraph mode */
        strreplace(expression,sizedirectives[irep],"",1,0); /*remove \size*/
      fontsize = irep; }		/* found \size so set fontsize */
/* --- check for \depth or \nodepth directive --- */
if ( strreplace(expression,"\\depth","",0,0) /* \depth requested */
>=   1 ) {				/* found \depth */
  if ( 1 ) {				/* guard */
  /* ---
   * note: curl_init() stops at the first whitespace char in $url argument,
   * so php functions using \depth replace blanks with tildes
   * ---------------------------------------------------------------------- */
    int ntilde = 0;			/* # ~ chars replaced */
    ntilde = strreplace(expression,"~"," ",0,0); }
  isdepth = 1;				/* so reset flag */
  latexwrapper = latexdepthwrapper; }	/* and wrapper */
if ( strreplace(expression,"\\nodepth","",0,0) /* \nodepth requested */
>=   1 ) {				/* found \nodepth */
  isdepth = 0;				/* so reset flag */
  latexwrapper = latexdefaultwrapper; }	/* and wrapper */
/* --- replace \time directive with timestamp (similar to \today) --- */
if ( strstr(expression,"\\time") != NULL ) { /* user wants a timestamp */
  strreplace(expression,"\\time",timestamp(TZDELTA,3),0,0); /* insert time */
  maxage = 5;				/* re-render after 5 secs */
  iscaching = 0; }			/* don't cache \time */
if ( strstr(expression,"\\today") != NULL ) { /* user has a datestamp */
  iscaching = 0; }			/* don't cache \today either */
/* --- replace \thisyear directive with 2012 --- */
if ( strstr(expression,"\\thisyear") != NULL ) /* user wants current year */
  strreplace(expression,"\\thisyear",timestamp(TZDELTA,5),0,0);/*insert year*/
/* --- replace \calendar directive with calendar string --- */
while ( strstr(expression,"\\calendar") != NULL ) { /* user wants calendar */
  int yy=0, mm=0, dd=0;			/* current or user-specified cal */
  mathmode = 2;				/* typeset in paragraph mode */
  strreplace(expression,"\\calendar",calendar(yy,mm,dd),0,1);
  maxage = 5;				/* re-render after 5 secs */
  iscaching = 0;			/* don't cache \calendar */
  } /* --- end-of-while("\\calendar") --- */
/* --- check for explicit usepackage directives in expression --- */
while ( npackages < 9 )			/* no more than 9 extra packages */
  if ( getdirective(expression,"\\usepackage",1,0,-1,packages[npackages])
  ==   NULL )  break;			/* no more \usepackage directives */
  else {				/* found another \usepackage */
    /* --- check for optional \usepackage [args] --- */
    *(packargs[npackages]) = '\000';	/* init for no optional args */
    if ( noptional > 0 ) {		/* but we found an optional arg */
      strninit(packargs[npackages],optionalargs[0],127); } /*copy the first*/
    /* --- check for particular packages --- */
    if ( strstr(packages[npackages],"color") != NULL ) /*\usepackage{color}*/
      iscolorpackage = 1;		/* set color package flag */
    if ( strstr(packages[npackages],"eepic") != NULL ) /*\usepackage{eepic}*/
      iseepicpackage = 1;		/* set eepic package flag */
    if ( strstr(packages[npackages],"pict2e") != NULL ) /* {pict2e} */
      ispict2epackage = 1;		/* set pict2e package flag */
    if ( strstr(packages[npackages],"preview") != NULL ) /* {preview} */
      ispreviewpackage = 1;		/* set preview package flag */
    npackages++; }			/* bump package count */
/* --- check for advertisement host and directive (\advertisement) --- */
if ( !isempty(host_showad) )		/*ad messages only for this referer*/
 if ( !isempty(http_host) )		/* have HTTP_HOST or SERVER_NAME */
   if ( strstr(http_host,host_showad)	/* see if this host sees ad */
   == NULL )				/* not mathtex host for ad message */
     adfrequency = 0;			/* turn off advertisements */
if ( strreplace(expression,"\\advertisement","",0,0) /*remove \advertisement*/
>=   1 ) adfrequency = 1;		/* force advertisement display */
/* --- check for version directive (\version) --- */
if ( strreplace(expression,"\\version","",0,0) /* remove \version */
>=   1 ) { adtemplate = versiontemplate; /* set version message */
  adfrequency = 1; }			/* and force its display */
/* --- check for image type directives (\gif or \png) --- */
if ( strreplace(expression,"\\png","",0,0) /* remove occurrences of \png */
>=   1 ) imagetype = 2;			/* found -png so set png imagetype */
if ( strreplace(expression,"\\gif","",0,0) /* remove occurrences of \gif */
>=   1 ) imagetype = 1;			/* found -gif so set gif imagetype */
/* --- check for latex method directives (\latex or \pdflatex) --- */
if ( strreplace(expression,"\\latex","",1,0) /*remove occurrences of \latex*/
>=   1 ) latexmethod = 1;		/* found \latex so set latex method */
if ( strreplace(expression,"\\pdflatex","",0,0) /* remove \pdflatex */
>=   1 ) latexmethod = 2;		/* found \pdflatex so set pdflatex */
/* --- check for image method directives (\dvips or \dvipng) --- */
if ( strreplace(expression,"\\dvipng","",0,0) /*remove occurrences of \dvipng*/
>=   1 ) {				/* found -dvipng in expression */
  imagemethod = 1;			/* set dvipng imagemethod */
  if ( !ISGAMMA ) strcpy(gamma,DVIPNGGAMMA); } /* default dvipng gamma */
if ( strreplace(expression,"\\dvips","",0,0) /* remove occurrences of -dvips*/
>=   1 ) {				/* found -dvips in expression */
  imagemethod = 2;			/* set dvips/convert imagemethod */
  if ( !ISGAMMA ) strcpy(gamma,CONVERTGAMMA); } /* default convert gamma */
/* --- check for convert/dvipng command's -density/-D parameter --- */
if ( getdirective(expression,"\\density",1,1,1,density) /*look for \density*/
==   NULL )				/* no \density directive */
  getdirective(expression,"\\dpi",1,1,1,density); /* so try \dpi instead */
/* --- check for convert command's \gammacorrection parameter --- */
getdirective(expression,"\\gammacorrection",1,1,1,gamma); /*look for \gamma*/
/* --- check for \cache or \nocache directive --- */
if ( strreplace(expression,"\\cache","",0,0) /* remove \cache */
>=   1 ) iscaching = 1;			/* cache this image */
if ( strreplace(expression,"\\nocache","",0,0) /* remove \nocache */
>=   1 ) iscaching = 0;			/* don't cache this image */
/* ---check for \eval{}'s in (1)submitted expressions, (2)\newcommand's--- */
for ( irep=1; irep<=2; irep++ ) {	/* 1=expression, 2=latexwrapper */
  char *thisrep = (irep==1?expression:latexwrapper); /* choose 1 or 2 */
  while ( (pdirective=getdirective(thisrep,"\\eval",1,0,1,argstring))
  !=   NULL ) {				/* found \eval{} directive */
    int  ival = (isempty(argstring)?0:evalterm(mathtexstore,argstring));
    char aval[256];			/* buffer to convert ival to alpha */
    sprintf(aval,"%d",ival);		/* convert ival to alpha */
    strchange(0,pdirective,aval); }	/* replace \eval{} with result */
  } /* --- end-of-for(irep) --- */
/* --- see if we need any packages not already \usepackage'd by user --- */
if ( npackages < 9			/* have room for one more package */
&&   !iscolorpackage )			/* \usepackage{color} not specified*/
  if ( strstr(expression,"\\color") != NULL ) { /* but \color is used */
    strcpy(packages[npackages],"color"); /* so \usepackage{color} is needed*/
    *(packargs[npackages]) = '\000';	/* no optional args */
    npackages++; }			/* bump package count */
if ( npackages < 9			/* have room for one more package */
&&   ispicture ) {			/* and using picture environment */
  if ( latexmethod == 1 )		/* using latex with eepic */
    if ( !iseepicpackage ) {		/* \usepackage{eepic} not specified*/
      strcpy(packages[npackages],"eepic"); /* \usepackage{eepic} needed */
      *(packargs[npackages]) = '\000';	/* no optional args */
      npackages++; }			/* bump package count */
  if ( latexmethod == 2			/* using pdflatex with pict2e */
  &&   npackages < 8 ) {		/* have room for two more packages */
    if ( !ispict2epackage ) {		/*\usepackage{pict2e} not specified*/
      strcpy(packages[npackages],"pict2e"); /* \usepackage{pict2e} needed */
      *(packargs[npackages]) = '\000';	/* no optional args */
      npackages++; }			/* bump package count */
    if ( !ispreviewpackage ) {		/*\usepackage{preview} not specified*/
      strcpy(packages[npackages],"preview"); /*\usepackage{preview} needed*/
      strcpy(packargs[npackages],"active,tightpage"); /* set optional args */
      npackages++; }			/* bump package count */
    } /* --- end-of-if(latexmethod==2) --- */
  } /* --- end-of-if(ispicture) --- */
/* ---
 * check for empty expression
 * -------------------------- */
trimwhite(expression);			/*remove leading/trailing whitespace*/
if ( isempty(expression) )		/* no expression supplied */
  if ( adfrequency != 1 )		/* and not a forced message */
    strcpy(expression,"\\fbox{\\rm No expression supplied}"); /* error msg */
/* ---
 * check for "advertisement"
 * ------------------------- */
if ( adfrequency > 0 ) {		/* advertising enabled */
  int	npump = crc16(expression)%16;	/* #times, 0-15, to pump rand() */
  srand(atoi(timestamp(TZDELTA,4)));	/* init rand() with mmddhhmmss */
  while ( npump-- >= 0 ) rand();	/* pre-pump rand() before use */
  if ( rand()%adfrequency == 0 ) {	/* once every adfrequency calls */
    advertisement(expression,adtemplate); /*wrap expression in advertisement*/
    if ( strcasestr(hashexpr,"\\advertisement") == NULL ) /* random ad */
      strchange(0,hashexpr,"\\advertisement "); } } /* signal ad in expr */
/* ---
 * hash the expression for name of cached image file
 * ------------------------------------------------- */
md5hash = md5str(hashexpr);		/* md5 hash of original expression */
/* ---
 * check for \msglevel{msglevel} for debugging
 * ------------------------------------------- */
if ( getdirective(expression,"\\msglevel",1,0,1,argstring)/*look for \msglvl*/
!=   NULL )				/* found \msglevel directive */
  if ( (msglevel = min2(atoi(argstring),MAXMSGLEVEL)) /*assign new msglevel*/
  >    0 )				/* user wants messages */
  if ( isquery )			/* can't write messages to stdout */
    msgfp = fopen(makepath(NULL,md5hash,".out"),"w"); /* file md5hash.out */
/* ---
 * emit initial messages
 * --------------------- */
if ( msgfp!=NULL && msglevel>=1 ) {	/*copyright notice and path to image*/
  fprintf(msgfp,"%s%s\n",copyright1,copyright2); /* always show copyright */
  showmsg(3,"running image",argv[0]);	/* and executable image if verbose */
  showmsg(2,"input expression",hashexpr); /* and input expression */
  if ( msglevel >= 8 ) {		/* and if very verbose */
    fprintf(msgfp,			/* timelimit info */
      "\nmathTeX> %s timelimit: warn/killtime=%d/%d, path=%s\n",
      (timelimit("",-99)==992?"Built-in":"Stub"), warntime, killtime,
      (istimelimitpath?timelimitpath:"none"));
    fflush(msgfp); }			/* flush message buffer */
  } /* --- end-of-if(msgfp!=NULL&&msglevel>=1) --- */
/* -------------------------------------------------------------------------
Emit cached image or render the expression
-------------------------------------------------------------------------- */
if ( md5hash != NULL ) {		/* md5str() almost surely succeeded*/
  /* ---
   * check for cache directory, and create it if it doesn't already exist
   * -------------------------------------------------------------------- */
  if ( isempty(outfile) )		/* no explicit output file given */
    if ( !isdexists(makepath(NULL,NULL,NULL)) ) /* and no cache directory */
      if ( mkdir(makepath(NULL,NULL,NULL),perm_all) /* so make cache dir */
      !=   0 ) {			/* failed, emit embedded error image*/
        emitembedded(CACHEFAILED,isquery); /* emit CACHEFAILED message */
        goto end_of_job; }		/* quit if failed to mkdir cache */
  /* ---
   * emit cached image if it already exists
   * -------------------------------------- */
  if ( isquery )			/* re-render if running from shell */
    if ( iscaching )			/* or if not caching image */
      if(emitcache(makepath(NULL,md5hash,extensions[imagetype]),maxage,0) > 0)
        goto end_of_job;		/* done if cached image emitted */
  /* ---
   * first log caching request for new image
   * --------------------------------------- */
  if ( msglevel >= 1 )			/* check if logging */
    if ( iscaching )			/* only log images being cached */
      if ( isempty(outfile) )		/* and not saved to explicit outfile*/
        mathlog(hashexpr,md5hash);	/* log request for new image */
  /* ---
   * now generate the new image and emit it
   * -------------------------------------- */
  /* --- set up name for temporary work directory --- */
  /*strninit(tempdir,tmpnam(NULL),255);*/ /* maximum name length is 255 */
  strninit(tempdir,md5hash,255);	/* maximum name length is 255 */
  /* --- additional message --- */
  showmsg(9,"working directory",tempdir); /* working directory */
  /* --- mathtex() generates the image and saves it to file --- */
  if ( mathtex(expression,md5hash)	/* generate new image */
  ==   imagetype ) {			/* and emit cache if succeeded */
    nbytes = 999;			/* signal rewritecache() success */
    /* --- rewrite dvipng or convert cachefile with extra imageinfo --- */
    if ( isdepth ) nbytes =		/* we have extra imageinfo */
      rewritecache(makepath(NULL,md5hash,extensions[imagetype]),maxage);
    /* --- emit the newly-generated image --- */
    if ( nbytes > 0 )			/* mathtex() and rewritecache() ok */
     if ( isquery )			/* don't emit cache to shell stdout*/
      emitcache(makepath(NULL,md5hash,extensions[imagetype]),maxage,0); }
  else {				/* latex failed to run */
    /* --- failed, so emit the embedded image of an error message --- */
    isdepth = 0;			/* no imageinfo in embedded images */
    if ( msgnumber < 1 ) msgnumber = 2;	/* general failure message */
    emitembedded(msgnumber,isquery); }	/* emit message image */
  /* ---
   * remove images not being cached
   * ------------------------------ */
  if ( !iscaching )			/* don't want this image cached */
    if ( isempty(outfile) )		/* unless explicit outfile provided*/
      remove(makepath(NULL,md5hash,extensions[imagetype])); /* remove file */
  } /* --- end-of-if(md5hash!=NULL) --- */
end_of_job:
  if ( msgfp!=NULL && msgfp!=stdout )	/* have an open message file */
    fclose(msgfp);			/* so close it at eoj */
  exit ( 0 );
} /* --- end-of-function main() --- */


/* ==========================================================================
 * Function:	mathtex ( expression, filename )
 * Purpose:	create image of latex math expression
 * --------------------------------------------------------------------------
 * Arguments:	expression (I)	pointer to null-terminated char string
 *				containing latex math expression
 *		filename (I)	pointer to null-terminated char string
 *				containing filename but not .extension
 *				of output file (should be the md5 hash
 *				of expression)
 * --------------------------------------------------------------------------
 * Returns:	( int )		imagetype if successful, 0=error
 * --------------------------------------------------------------------------
 * Notes:     o	created file will be filename.gif if imagetype=1
 *		or filename.png if imagetype=2.
 * ======================================================================= */
/* --- entry point --- */
int	mathtex ( char *expression, char *filename )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
char  errormsg[1024] =			/* latex runs but can't make .dvi */
	"\\fbox{\\footnotesize $\\mbox{Latex failed, probably due to} "
	"\\atop \\mbox{an error in your expression.}$}";
char  usepackage[1024] = "\000";	/* additional \usepackage{}'s */
char  convertargs[1024] =		/* args/switches for convert */
	" -density %%dpi%% -gamma %%gamma%%"
	/*" -border 0% -fuzz 2%"*/
	" -trim -transparent \"#FFFFFF\" ";
char  dvipngargs[1024] =		/* args/switches for dvipng */
	" --%%imagetype%% -D %%dpi%% --gamma %%gamma%%"
	" -bg Transparent -T tight -v"	/* -q for quiet, -v for verbose */
	" -o %%giffile%% ";		/* output filename supplied as -o */
/* --- other variables --- */
static	int iserror = 0;		/* true if procesing error message */
int	setpaths();			/* set paths for latex,dvipng,etc */
char	*makepath(), latexfile[256],giffile[256]="\000";/*path/filename.ext*/
FILE	*latexfp = NULL;		/*latex wrapper file for expression*/
char	command[2048], subcommand[1024]; /*system(command) runs latex, etc*/
char	*beginmath[] =			/* start math mode */
	  { " \\noindent $\\displaystyle ", " \\noindent $ ", " " },
	*endmath[] =			/* end math mode */
	  { " $ ",                          " $ ", " " };
int	perm_all = (S_IRWXU|S_IRWXG|S_IRWXO); /* 777 permissions */
int	dir_stat = 0;			/* 1=mkdir okay, 2=chdir okay */
int	sys_stat = 0;			/* system() return status */
int	isdexists(), isfexists(),	/* check if dir, .dvi file created */
	isnotfound();			/* check .err file for "not found" */
int	strreplace();			/* replace template directives */
int	ipackage = 0;			/* packages[] index 0...npackages-1*/
int	rrmdir();			/* rm -r */
int	gifpathlen = 0;			/* ../ or ../../ prefix of giffile */
int	status = 0;			/* imagetype or 0=error */
int	timelimit();			/* and using built-in timelimit() */
/* -------------------------------------------------------------------------
Make temporary work directory and change to it
-------------------------------------------------------------------------- */
msgnumber = 0;				/* no error to report yet */
if ( !isdexists(tempdir/*filename*/) )	/* if temp directory doesn't exist */
  if ( mkdir(tempdir/*filename*/,perm_all) /* make temp dirextory */
  !=   0 ) {				/* mkdir failed */
    msgnumber = MKDIRFAILED;		/* set corresponding message number*/
    goto end_of_job; }			/* and quit */
dir_stat++;				/* signal mkdir successful */
if ( chdir(tempdir/*filename*/)		/* cd to temp directory */
!=   0 ) {				/* cd  failed */
    msgnumber = CHDIRFAILED;		/* set corresponding message number*/
    goto end_of_job; }			/* and quit */
dir_stat++;				/* signal chdir successful */
/* -------------------------------------------------------------------------
set up latex directives for user-specified additional \usepackage{}'s
-------------------------------------------------------------------------- */
if ( !iserror )				/* don't \usepackage for error */
 if ( npackages > 0 )			/* have additional packages */
  for ( ipackage=0; ipackage<npackages; ipackage++ ) { /*make \usepackage{}*/
    strcat(usepackage,"\\usepackage");	/* start with a directive */
    if ( !isempty(packargs[ipackage]) ) { /* have an optional arg */
      strcat(usepackage,"[");		/* begin optional argument */
      strcat(usepackage,packargs[ipackage]); /* add optional arg */
      strcat(usepackage,"]"); }		/* finish optional arg */
    strcat(usepackage,"{");		/* begin package name argument */
    strcat(usepackage,packages[ipackage]); /* add package name */
    strcat(usepackage,"}\n"); }		/* finish constructing directive */
/* -------------------------------------------------------------------------
Replace "keywords" in latex template with expression and other directives
-------------------------------------------------------------------------- */
/* --- usually replace %%pagestyle%% with \pagestyle{empty} --- */
  if ( !ispicture			/* not \begin{picture} environment */
  ||   latexmethod == 1 )		/* or using latex/dvips/ps2epsi */
    strreplace(latexwrapper,"%%pagestyle%%","\\pagestyle{empty}",1,0);
/* --- replace %%previewenviron%% if a picture and using pfdlatex --- */
  if ( ispicture			/* have \begin{picture} environment */
  &&   latexmethod == 2 )		/* and using pdflatex/convert */
    strreplace(latexwrapper,"%%previewenviron%%",
    "\\PreviewEnvironment{picture}",1,0);
/* --- replace %%beginmath%%...%%endmath%% with \[...\] or with $...$ --- */
  if ( mathmode<0 || mathmode>2 ) mathmode=0; /* mathmode validity check */
  strreplace(latexwrapper,"%%beginmath%%",beginmath[mathmode],1,0);
  strreplace(latexwrapper,"%%endmath%%",endmath[mathmode],1,0);
/* --- replace %%fontsize%% in template with \tiny...\Huge --- */
  strreplace(latexwrapper,"%%fontsize%%",sizedirectives[fontsize],1,0);
/* --- replace %%setlength%% in template for pictures when necessary --- */
  if ( ispicture			/* have \begin{picture} environment */
  &&   strstr(expression,"\\unitlength") == NULL ) /* but no \unitlength */
    strreplace(latexwrapper,"%%setlength%%", /* so default it to 1 inch */
    "\\setlength{\\unitlength}{1.0in}",1,0);
/* --- replace %%usepackage%% in template with extra \usepackage{}'s --- */
  strreplace(latexwrapper,"%%usepackage%%",usepackage,1,0);
/* --- replace %%expression%% in template with expression --- */
  strreplace(latexwrapper,"%%expression%%",expression,1,0);
/* -------------------------------------------------------------------------
Create latex document wrapper file containing expression
-------------------------------------------------------------------------- */
strcpy(latexfile,makepath("","latex",".tex")); /* latex filename latex.tex */
latexfp = fopen(latexfile,"w");		/* open latex file for write */
if ( latexfp == NULL ) {		/* couldn't open latex file */
  msgnumber = FOPENFAILED;		/* set corresponding message number*/
  goto end_of_job; }			/* and quit */
fprintf(latexfp,"%s",latexwrapper);	/* write file */
fclose(latexfp);			/* close file after writing it */
/* -------------------------------------------------------------------------
Set paths to programs we'll need to run
-------------------------------------------------------------------------- */
setpaths(10*latexmethod+imagemethod);
/* -------------------------------------------------------------------------
Execute the latex file
-------------------------------------------------------------------------- */
/* --- initialize system(command); to execute the latex file --- */
*command = '\000';			/* init command as empty string */
/* --- run latex under timelimit if explicitly given -DTIMELIMIT switch --- */
if ( istimelimitpath			/* given explict -DTIMELIMIT path */
&&   warntime > 0			/* and positive warntime, and... */
&&   !iscompiletimelimit ) {		/* not using built-in timelimit() */
  if ( killtime < 1 ) killtime=1;	/* don't make trouble for timelimit*/
  strcat(command,makepath("",timelimitpath,NULL)); /* timelimit program */
  if ( isempty(command) )		/* no path to timelimit */
    warntime = (-1);			/*reset flag to signal no timelimit*/
  else {				/* have path to timelimit program */
    sprintf(command+strlen(command),	/* add timelimit args after path */
      " -t%d -T%d ",warntime,killtime); }
  } /* --- end-of-if(warntime>0) --- */
/* --- path to latex executable image followed by args --- */
if ( latexmethod != 2 )			/* not explicitly using pdflatex */
  strcpy(subcommand,makepath("",latexpath,NULL)); /* running latex program */
else					/* explicitly using pdflatex */
  strcpy(subcommand,makepath("",pdflatexpath,NULL)); /* running pdflatex */
if ( isempty(subcommand) ) {		/* no program path to latex */
  msgnumber = SYLTXFAILED;		/* set corresponding error message */
  goto end_of_job; }			/* signal failure and emit error */
strcat(command,subcommand);		/* add latex path (after timelimit)*/
strcat(command," ");			/* add a blank before latex args */
strcat(command,latexfile);		/* run on latexfile we just wrote */
if ( isquiet > 0 ) {			/* to continue after latex error */
    if ( isquiet > 99 )			/* explicit q requested */
      system("echo \"q\" > reply.txt");	/* reply  q  to latex error prompt */
    else {				/* reply <Enter>'s followed by x */
      int  nquiet = isquiet;		/* this many <Enter>'s before x */
      FILE *freply =fopen("reply.txt","w"); /* open reply.txt for write */
      if ( freply != NULL ) {		/* opened successfully */
	while ( --nquiet >= 0 )		/* nquiet times... */
	  fputs("\n",freply);		/* ...write <Enter> to reply.txt */
	fputs("x",freply);		/* finally followed by an x */
	fclose(freply); } }		/* close reply.txt */
    strcat(command," < reply.txt"); }	/*by redirecting stdin to reply.txt*/
  else strcat(command," < /dev/null");	/* or redirect stdin to /dev/null */
strcat(command," >latex.out 2>latex.err"); /* redirect stdout and stderr */
showmsg(5,"latex command executed",command); /* show latex command executed*/
/* --- execute the latex file --- */
sys_stat = timelimit(command,killtime);	/* throttle the latex command */
if ( msgfp!=NULL && msglevel>=3 )	/* and show command's return status*/
  fprintf(msgfp,"\nmathTeX> system() return status: %d\n", sys_stat);
if ( latexmethod != 2 )			/* ran latex */
  if ( !isfexists(makepath("","latex",".dvi")) )  /* but no latex dvi */
    sys_stat = (-1);			/* signal that latex failed */
if ( latexmethod == 2 )			/* ran pdflatex */
  if ( !isfexists(makepath("","latex",".pdf")) )  /* but no pdflatex pdf */
    sys_stat = (-1);			/* signal that pdflatex failed */
if ( sys_stat == (-1) ) {		/* system() or pdf/latex failed */
  if ( !iserror ) {			/* don't recurse if errormsg fails */
    iserror = 1;			/* set error flag */
    isdepth = ispicture = 0;		/* reset depth, picture mode */
    status = mathtex(errormsg,filename); /* recurse just once for error msg*/
    goto end_of_job; }			/* ignore original expression */
  else {				/* ignore 2nd try to recurse */
    msgnumber =				/* set corresponding message number*/
      (sys_stat==(-1)?SYLTXFAILED:	/* system() failed */
      (isnotfound("latex")?SYLTXFAILED:	/* latex program not found */
      LATEXFAILED));			/* latex ran but failed */
    goto end_of_job; }			/* and quit */
  } /* --- end-of-if(system()==-1||!isfexists()) --- */
/* -------------------------------------------------------------------------
Extract image info from latex.info (if available)
-------------------------------------------------------------------------- */
if ( isdepth )				/* image info requested */
 if ( isfexists(makepath("","latex",".info")) ) { /* and have latex.info */
  FILE *info = fopen(makepath("","latex",".info"),"r"); /*open it for read*/
  char infoline[256];			/* read a line from latex.info */
  if ( info != NULL ) {			/* open succeeded */
   while ( fgets(infoline,255,info) != NULL ) { /* read until eof or error */
    int  i = 0;				/* imageinfo[] index */
    char *delim = NULL;			/* find '=' in infoline */
    trimwhite(infoline);		/*remove leading/trailing whitespace*/
    for ( i=0; imageinfo[i].format!=NULL; i++ ) /* all imageinfo[] fields */
     if ( strstr(infoline,imageinfo[i].identifier) == infoline ) {
      imageinfo[i].value = (-9999.);	/* init value to signal error */
      *(imageinfo[i].units) = '\000';	/* init units to signal error */
      if ( (delim=strchr(infoline,'=')) == NULL ) break; /* no '=' delim */
      memmove(infoline,delim+1,strlen(delim)); /* get value after '=' */
      trimwhite(infoline);		/*remove leading/trailing whitespace*/
      imageinfo[i].value = strtod(infoline,&delim); /* convert to integer */
      if ( !isempty(delim) ) {		/* units, e.g., "pt", after value */
       memmove(infoline,delim,strlen(delim)+1); /* get units field */
       trimwhite(infoline);		/*remove leading/trailing whitespace*/
       strninit(imageinfo[i].units,infoline,16); } /* copy units */
      break;				/* don't check further identifiers */
      } /* --- end-of-if(strstr(infoline,identifier)) --- */
    } /* --- end-of-while(fgets()!=NULL) --- */
   fclose(info);			/* close info file */
   } /* --- end-of-if(info!=NULL) --- */
  } /* --- end-of-if(isfexists("latex.info")) --- */
/* -------------------------------------------------------------------------
Construct the output path/filename.[gif,png] for the image file
-------------------------------------------------------------------------- */
if ( isempty(outfile)			/* using default cache directory */
||   !isthischar(*outfile,"/\\") ) {	/* or just given a relative path */
  strcpy(giffile,"../");		/* output file will be in cache */
  if ( iserror ) strcat(giffile,"../");	/* we're in error subdirectory */
  gifpathlen = strlen(giffile); }	/* #chars in ../ or ../../ prefix */
if ( isempty(outfile) )			/* using default output filename */
  strcat(giffile,makepath(NULL,filename,extensions[imagetype]));
else					/* have an explicit output file */
  strcat(giffile,makepath("",outfile,extensions[imagetype]));
showmsg(3,"output image file",giffile+gifpathlen); /* show output filename */
/* -------------------------------------------------------------------------
Run dvipng for .dvi-to-gif/png
-------------------------------------------------------------------------- */
if ( imagemethod == 1 ) {		/*dvipng method requested (default)*/
  /* ---
   * First replace "keywords" in dvipngargs template with actual values
   *------------------------------------------------------------------- */
  /* --- replace %%imagetype%% in dvipng arg template with gif or png --- */
  strreplace(dvipngargs,"%%imagetype%%",extensions[imagetype],1,0);
  /* --- replace %%dpi%% in dvipng arg template with actual dpi --- */
  strreplace(dvipngargs,"%%dpi%%",density,1,0);
  /* --- replace %%gamma%% in dvipng arg template with actual gamma --- */
  strreplace(dvipngargs,"%%gamma%%",gamma,1,0);
  /* --- replace %%giffile%% in dvipng arg template with actual giffile --- */
  strreplace(dvipngargs,"%%giffile%%",giffile,1,0);
  /* ---
   * And run dvipng to convert .dvi file directly to .gif/.png
   *---------------------------------------------------------- */
  strcpy(command,makepath("",dvipngpath,NULL)); /* running dvipng program */
  if ( isempty(command) ) {		/* no program path to dvipng */
    msgnumber = SYPNGFAILED;		/* set corresponding error message */
    goto end_of_job; }			/* signal failure and emit error */
  strcat(command,dvipngargs);		/* add dvipng switches */
  strcat(command,makepath("","latex",".dvi")); /* run dvipng on latex.dvi */
  strcat(command," >dvipng.out 2>dvipng.err"); /* redirect stdout, stderr */
  showmsg(5,"dvipng command executed",command); /* dvipng command executed */
  sys_stat = system(command);		/* execute the dvipng command */
  if ( sys_stat == (-1)			/* system(dvipng) failed */
  ||   !isfexists(giffile) ) {		/*or dvipng failed to create giffile*/
    msgnumber =				/* set corresponding message number*/
      (sys_stat==(-1)?SYPNGFAILED:	/* system() failed */
      (isnotfound("dvipng")?SYPNGFAILED: /* dvipng program not found */
      DVIPNGFAILED));			/* dvipng ran but failed */
    goto end_of_job; }			/* and quit */
  } /* --- end-of-if(imagemethod==1) --- */
/* -------------------------------------------------------------------------
Run dvips for .dvi-to-postscript and convert for postscript-to-gif/png
-------------------------------------------------------------------------- */
if ( imagemethod == 2 ) {		/* dvips/convert method requested */
  /* ---
   * First run dvips to convert .dvi file to .ps postscript
   *------------------------------------------------------- */
  if ( latexmethod != 2 ) {		/* only if not using pdflatex */
    strcpy(command,makepath("",dvipspath,NULL)); /* running dvips program */
    if ( isempty(command) ) {		/* no program path to dvips */
      msgnumber = SYPSFAILED;		/* set corresponding error message */
      goto end_of_job; }		/* signal failure and emit error */
    if ( !ispicture ) strcat(command," -E"); /*add -E switch if not picture*/
    strcat(command," ");		/* add a blank */
    strcat(command,makepath("","latex",".dvi")); /* run dvips on latex.dvi */
    strcat(command," -o ");		/* to produce output file in */
    if ( !ispicture )			/* when not a picture (usually) */
      strcat(command,makepath("","dvips",".ps")); /*dvips.ps postscript file*/
    else				/*intermediate temp file for ps2epsi*/
      strcat(command,makepath("","dvitemp",".ps")); /*temp postscript file*/
    strcat(command," >dvips.out 2>dvips.err"); /* redirect stdout, stderr */
    showmsg(5,"dvips command executed",command); /* dvips command executed */
    sys_stat = system(command);		/* execute system(dvips) */
    /* --- run ps2epsi if dvips ran without -E (for \begin{picture}) --- */
    if ( sys_stat != (-1)		/* system(dvips) succeeded */
    &&   ispicture ) {			/* and we ran dvips without -E */
      strcpy(command,makepath("",ps2epsipath,NULL)); /* running ps2epsi */
      if ( isempty(command) ) {		/* no program path to ps2epsi */
        msgnumber = SYPSFAILED;		/* set corresponding error message */
        goto end_of_job; }		/* signal failure and emit error */
      strcat(command," ");		/* add a blank */
      strcat(command,makepath("","dvitemp",".ps")); /*temp postscript file*/
      strcat(command," ");		/* add a blank */
      strcat(command,makepath("","dvips",".ps")); /*dvips.ps postscript file*/
      strcat(command," >ps2epsi.out 2>ps2epsi.err");/*redirect stdout,stderr*/
      showmsg(5,"ps2epsi command executed",command); /* command executed */
      sys_stat = system(command);	/* execute system(ps2epsi) */
      } /* --- end-of-if(ispicture) --- */
    if ( sys_stat == (-1)		/* system(dvips) failed */
    ||   !isfexists(makepath("","dvips",".ps")) ) { /*dvips didn't create .ps*/
      msgnumber =			/* set corresponding message number*/
        (sys_stat==(-1)?SYPSFAILED:	/* system() failed */
        (isnotfound("dvips")?SYPSFAILED: /* dvips program not found */
        DVIPSFAILED));			/* dvips ran but failed */
      goto end_of_job; }		/* and quit */
    } /* --- end-of-if(latexmethod!=2) --- */
  /* ---
   * Then replace "keywords" in convertargs template with actual values
   *------------------------------------------------------------------- */
  /* --- replace %%dpi%% in convert arg template with actual density --- */
  strreplace(convertargs,"%%dpi%%",density,1,0);
  /* --- replace %%gamma%% in convert arg template with actual gamma --- */
  strreplace(convertargs,"%%gamma%%",gamma,1,0);
  /* ---
   * And run convert to convert .ps file to .gif/.png
   *-------------------------------------------------- */
  strcpy(command,makepath("",convertpath,NULL)); /*running convert program*/
  if ( isempty(command) ) {		/* no program path to convert */
    msgnumber = SYCVTFAILED;		/* set corresponding error message */
    goto end_of_job; }			/* signal failure and emit error */
  strcat(command,convertargs);		/* add convert switches */
  if ( latexmethod != 2 )		/* we ran latex and dvips */
    strcat(command,makepath("","dvips",".ps")); /* convert from postscript */
  if ( latexmethod == 2 )		/* we ran pdflatex */
    strcat(command,makepath("","latex",".pdf")); /* convert from pdf */
  strcat(command," ");			/* field separator */
  strcat(command,giffile);		/* followed by ../cache/filename */
  strcat(command," >convert.out 2>convert.err"); /*redirect stdout, stderr*/
  showmsg(5,"convert command executed",command); /*convert command executed*/
  sys_stat = system(command);		/* execute system(convert) command */
  if ( sys_stat == (-1)			/* system(convert) failed */
  ||   !isfexists(giffile) ) {		/* or convert didn't create giffile*/
    msgnumber =				/* set corresponding message number*/
      (sys_stat==(-1)?SYCVTFAILED:	/* system() failed */
      (isnotfound("convert")?SYCVTFAILED: /* convert program not found */
      CONVERTFAILED));			/* convert ran but failed */
    goto end_of_job; }			/* and quit */
  } /* --- end-of-if(imagemethod==2) --- */
status = imagetype;			/* signal success */
/* -------------------------------------------------------------------------
Back to caller
-------------------------------------------------------------------------- */
end_of_job:
  if ( dir_stat >= 2 ) chdir("..");	/* return up to original dir */
  if ( dir_stat >= 1 ) rrmdir(tempdir/*filename*/); /*rm -r temp working dir*/
  iserror = 0;				/* always reset error flag */
  return ( status );
} /* --- end-of-function mathtex() --- */


/* ==========================================================================
 * Function:	setpaths ( method )
 * Purpose:	try to set accurate paths for
 *		latex,pdflatex,timelimit,dvipng,dvips,convert
 * --------------------------------------------------------------------------
 * Arguments:	method (I)	10*ltxmethod + imgmethod where
 *				ltxmethod =
 *				   1 for latex, 2 for pdflatex,
 *				   0 for both
 *				imgmethod =
 *				   1 for dvipng, 2 for dvips/convert,
 *				   0 for both
 * --------------------------------------------------------------------------
 * Returns:	( int )		1
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	setpaths ( int method )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
char	*programpath=NULL, *whichpath(); /* look for latex,dvipng,etc */
int	nlocate = 0;			/*if which fails, #locate|grep hits*/
int	ltxmethod = method/10,		/* divide by 10 */
	imgmethod = method%10;		/* remainder after division by 10 */
static	int islatexwhich=0, ispdflatexwhich=0,    /* set true after we try */
	isdvipngwhich=0,    isdvipswhich=0,  /*whichpath() for that program*/
	isps2epsiwhich=0,   isconvertwhich=0,   istimelimitwhich=0;
/* ---
 * set paths, either from -DLATEX=\"path/latex\", etc, or call whichpath()
 * ----------------------------------------------------------------------- */
/* --- path for latex program --- */
if ( ltxmethod==1 || ltxmethod==0 )	/* not needed for pdflatex only */
 /*if ( !ISLATEXSWITCH && !islatexwhich )*/ /* nb, use islatexpath instead */
 if ( !islatexpath && !islatexwhich ) {	/* no -DLATEX=\"path/latex\" */
  islatexwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("latex",&nlocate)) /* try to find latex path */
  !=   NULL ) {				/* succeeded */
    islatexpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(latexpath,programpath,255); } }/* copy path from whichpath() */
/* --- path for pdflatex program --- */
if ( ltxmethod==2 || ltxmethod==0 )	/* not needed for latex only */
 /*if ( !ISPDFLATEXSWITCH && !ispdflatexwhich )*/ /*nb, use ispdflatexpath*/
 if ( !ispdflatexpath && !ispdflatexwhich ) { /*no -DPDFLATEX=\"pdflatex\"*/
  ispdflatexwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("pdflatex",&nlocate)) /*try to find pdflatex*/
  !=   NULL ) {				/* succeeded */
    ispdflatexpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(pdflatexpath,programpath,255); } }/*copy path from whichpath()*/
/* --- path for timelimit program --- */
if ( 0 )				/* only use explicit -DTIMELIMIT */
 if ( warntime > 0 )			/* have -DWARNTIME or -DTIMELIMIT */
  /*if ( !ISTIMELIMITSWITCH*/		/* nb, use istimelimitpath instead */
  if ( !istimelimitpath			/*no -DTIMELIMIT=\"path/timelimit\"*/
  &&   !istimelimitwhich ) {
    istimelimitwhich = 1;		/* signal that which already tried */
    nlocate = ISLOCATE;			/* use locate if which fails??? */
    if ( (programpath=whichpath("timelimit",&nlocate)) /* try to find path */
    !=   NULL ) {			/* succeeded */
      istimelimitpath = (nlocate==0?2:3); /* set flag signalling which() */
      strninit(timelimitpath,programpath,255); } } /* copy from whichpath()*/
/* --- path for dvipng program --- */
if ( imgmethod != 2 )			/* not needed for dvips/convert */
 /*if ( !ISDVIPNGSWITCH && !isdvipngwhich )*/ /*nb, use isdvipngpath instead*/
 if ( !isdvipngpath && !isdvipngwhich ) { /* no -DDVIPNG=\"path/dvipng\" */
  isdvipngwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("dvipng",&nlocate)) /* try to find dvipng */
  !=   NULL ) {				/* succeeded */
    isdvipngpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(dvipngpath,programpath,255); } }/*so copy path from whichpath()*/
/* --- path for dvips program --- */
if ( imgmethod != 1 )			/* not needed for dvipng */
 /*if ( !ISDVIPSSWITCH && !isdvipswhich )*/ /* nb, use isdvipspath instead */
 if ( !isdvipspath && !isdvipswhich ) {	/* no -DDVIPS=\"path/dvips\" */
  isdvipswhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("dvips",&nlocate)) /* try to find dvips path */
  !=   NULL ) {				/* succeeded */
    isdvipspath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(dvipspath,programpath,255); } }/* so copy path from whichpath()*/
/* --- path for ps2epsi program --- */
if ( (ispicture || method==0)		/* only needed for \begin{picture} */
&&   imgmethod != 1			/* not needed for dvipng */
&&   ltxmethod != 2 )			/* not needed for pdflatex */
 /*if ( !ISPS2EPSISWITCH && !isps2epsiwhich )*/ /*use isps2epsipath instead*/
 if ( !isps2epsipath && !isps2epsiwhich ) { /*no -DPS2EPSI=\"path/ps2epsi\"*/
  isps2epsiwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("ps2epsi",&nlocate))/*try to find ps2epsi path*/
  !=   NULL ) {				/* succeeded */
    isps2epsipath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(ps2epsipath,programpath,255); } } /*copy path from whichpath()*/
/* --- path for convert program --- */
if ( imgmethod != 1 )			/* not needed for dvipng */
 /*if ( !ISCONVERTSWITCH && !isconvertwhich )*/ /*use isconvertpath instead*/
 if ( !isconvertpath && !isconvertwhich ) { /*no -DCONVERT=\"path/convert\"*/
  isconvertwhich = 1;			/* signal that which already tried */
  nlocate = ISLOCATE;			/* use locate if which fails??? */
  if ( (programpath=whichpath("convert",&nlocate)) /* try to find convert */
  !=   NULL ) {				/* succeeded */
    isconvertpath = (nlocate==0?2:3);	/* set flag signalling which() */
    strninit(convertpath,programpath,255); } } /*copy path from whichpath()*/
/* --- adjust imagemethod to comply with available programs --- */
if ( imgmethod != imagemethod		/* ignore recursive call */
&&   imgmethod != 0 ) goto end_of_job;	/* but 0 is a call for both methods*/
if ( imagemethod == 1 )			/* dvipng wanted */
  if ( !isdvipngpath ) {		/* but we have no real path to it */
    if ( imgmethod == 1 ) setpaths(2);	/* try to set dvips/convert paths */
    if ( isdvipspath && isconvertpath )	{ /* and we do have dvips, convert */
      imagemethod = 2;			/* so flip default to use them */
      if ( !ISGAMMA ) strcpy(gamma,CONVERTGAMMA); } }/*default convert gamma*/
if ( imagemethod == 2 )			/* dvips, convert wanted */
  if ( !isdvipspath || !isconvertpath ) {/*but we have no real paths to them*/
    if ( imgmethod == 2 ) setpaths(1);	/* try to set dvipng path */
    if ( isdvipngpath ) {		/* and we do have dvipng path */
      imagemethod = 1;			/* so flip default to use it */
      if ( !ISGAMMA ) strcpy(gamma,DVIPNGGAMMA); } }/* default dvipng gamma */
/* --- back to caller --- */
end_of_job:
  return ( 1 );
} /* --- end-of-function setpaths() --- */


/* ==========================================================================
 * Function:	isnotfound ( filename )
 * Purpose:	check a  "... 2>filename.err" file for a "not found" message,
 *		indicating the corresponding program path is incorrect
 * --------------------------------------------------------------------------
 * Arguments:	filename (I)	pointer to null-terminated char string
 *				containing filename (an .err extension
 *				is tacked on, just pass the filename)
 *				to be checked for "not found" message
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 if filename.err contains "not found",
 *				0 otherwise (or for any error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	isnotfound ( char *filename )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	status = 0;			/* set 1 if "not found" found */
int	isfexists();			/* check if filename exists */
char	command[256];			/* grep program */
FILE    *grepout = NULL;		/* grep's stdout */
char	grepline[256];			/* line from grep's stdout */
int	strreplace();			/* make sure "not found" was found */
int	nlines = 0;			/* #lines read */
/* -------------------------------------------------------------------------
grep filename.err for filename (i.e., the program name), then check each
such line for "not found", "no such", etc.
-------------------------------------------------------------------------- */
/* --- initialization --- */
if ( isempty(filename) ) goto end_of_job; /* no input filename supplied */
sprintf(command,"%s.err",filename);	/* look for filename.err */
if ( !isfexists(command) ) {		/* filename.err doesn't exist */
  status = 1;				/* assume this means "not found" */
  goto end_of_job; }			/* and return that to caller */
/* --- grep filename.err for filename (i.e., for the program name) --- */
sprintf(command,"grep -i \"%s\" %s.err",filename,filename); /*construct cmd*/
/* --- use popen() to invoke grep --- */
grepout = popen( command, "r" );	/* issue grep and capture stdout */
if( grepout == NULL ) goto end_of_job;	/* failed */
/* --- read the pipe one line at a time --- */
while ( fgets(grepline,255,grepout)	/* get next line */
!= NULL ) {				/* not at eof yet */
  if ( strreplace(grepline,"not found","",0,0) /* check for "not found" */
  >    0 ) nlines++;			/* count another "not found" line */
  if ( strreplace(grepline,"no such","",0,0) /* check for "no such" */
  >    0 ) nlines++;			/* count another "no such" line */
  } /* --- end-of-while(fgets()!=NULL) -- */
if ( nlines > 0 ) status = 1;		/* "not found" found */
/* --- pclose() waits for command to terminate --- */
pclose( grepout );			/* finish */
end_of_job:				/* back to caller */
  return ( status );			/*1 if filename contains "not found"*/
} /* --- end-of-function isnotfound() --- */


/* ==========================================================================
 * Function:	validate ( expression )
 * Purpose:	remove/replace illegal \commands from expression
 * --------------------------------------------------------------------------
 * Arguments:	expression (I/O) pointer to null-terminated char string
 *				containing latex expression to be validated.
 *				Upon return, invalid \commands have been
 *				removed or replaced
 * --------------------------------------------------------------------------
 * Returns:	( int )		#illegal \commands found (hopefully 0)
 * --------------------------------------------------------------------------
 * Notes:     o	Ignore this note and see the one below it instead...
 *		  This routine is currently "stubbed out",
 *		  i.e., the invalid[] list of invalid \commands is empty.
 *		  Instead, \renewcommand's are embedded in the LaTeX wrapper.
 *		  So the user's -DNEWCOMMAND=\"filename\" can contain
 *		  additional \renewcomand's for any desired validity checks.
 *	      o	Because some \renewcommand's appear to occasionally cause
 *		latex to go into a loop after encountering syntax errors,
 *		I'm now using validate() to disable \input, etc.
 *		For example, the following short document...
 *		  \documentclass[10pt]{article}
 *		  \begin{document}
 *		  \renewcommand{\input}[1]{dummy renewcommand}
 *		  %%\renewcommand{\sqrt}[1]{dummy renewcommand}
 *		  %%\renewcommand{\beta}{dummy renewcommand}
 *		  \[ \left( \begin{array}{cccc} 1 & 0 & 0 &0
 *		  \\ 0 & 1  %%% \end{array}\right)
 *		  \]
 *		  \end{document}
 *		reports a  "! LaTeX Error:"  and then goes into
 *		a loop if you reply q.  But if you comment out the
 *		\renewcommand{\input} and uncomment either or both
 *		of the other \renewcommand's, then latex runs to
 *		completion (with the same syntax error, of course,
 *		but without hanging).
 *
 * ======================================================================= */
/* --- entry point --- */
int	validate ( char *expression )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- list of invalid \commands --- */
static	struct {
	  int  action;			/* 0=ignore, 1=apply, 2=abort */
	  char *command;		/* invalid \command */
	  int  nargs;			/* #args, \command{arg1}...{nargs} */
	  int  optionalpos;		/* #{args} before optional [arg] */
	  int  argformat;		/* 0=LaTeX {arg} or [arg], 1=arg */
	  char *displaystring; }	/* display this instead */
	invalid[] = {			/* list of invalid commands */
	  #if defined(INVALID)		/* cc -DINVALID=\"filename\" */
	    #include INVALID		/* filename with invalid \commands */
	  #endif			/* as illustrated below... */
	/* actn  "command"        #args pos fmt "replacement string" or NULL
	 * ---- ----------------- ----- --- --- -------------------------- */
	  { 1,  "\\newcommand",      2,  1,  0,  NULL },
	  { 1,  "\\providecommand",  2,  1,  0,  NULL },
	  { 1,  "\\renewcommand",    2,  1,  0,  NULL },
	  { 1,  "\\input",           1, -1,  0,  NULL },
	/* --plain TeX commands with non-{}-enclosed args we can't parse-- */
	  { 1,  "\\def",             2, -1, 20,  NULL },
	  { 1,  "\\edef",            2, -1, 20,  NULL },
	  { 1,  "\\gdef",            2, -1, 20,  NULL },
	  { 1,  "\\xdef",            2, -1, 20,  NULL },
	  { 1,  "\\loop",            0, -1,  0,  NULL },
	  { 1,  "\\csname",          0, -1,  0,  NULL },
	  { 1,  "\\catcode",         0, -1,  0,  NULL },
	  { 1,  "\\output",          0, -1,  0,  NULL },
	  { 1,  "\\everycr",         0, -1,  0,  NULL },
	  { 1,  "\\everypar",        0, -1,  0,  NULL },
	  { 1,  "\\everymath",       0, -1,  0,  NULL },
	  { 1,  "\\everyhbox",       0, -1,  0,  NULL },
	  { 1,  "\\everyvbox",       0, -1,  0,  NULL },
	  { 1,  "\\everyjob",        0, -1,  0,  NULL },
	  { 1,  "\\openin",          0, -1,  0,  NULL },
	  { 1,  "\\read",            0, -1,  0,  NULL },
	  { 1,  "\\openout",         0, -1,  0,  NULL },
	  { 1,  "\\write",           0, -1,  0,  NULL },
	/* --- other dangerous notation --- */
	  { 1,  "^^",                0, -1,  0,  NULL },
#if 0
	/* --- test cases --- */
	/*{ 1,  "\\input",      1,  -1,  0,
		"{\\mbox{$\\backslash$input\\{#1\\}}}" },*/
	  { 1,  "\\input",      1,  -1,  0,
		"{\\mbox{~$\\backslash$input\\{#1\\}~not~permitted~}}" },
	  { 1,  "\\newcommand", 2,   1,  0, NULL},
		 /*"{\\mbox{~$\\backslash$newcommand\\{#1\\}[#0]\\{#2\\}"
		 "~not~permitted~}}" },*/
#endif
	  { 0, NULL, 0, -1, 0, NULL } } ; /* end-of-list */
/* --- other variables --- */
int	ninvalid = 0,			/* #invalid =commands found */
	ivalid = 0;			/* invalid[ivalid] list index */
char	*getdirective(), *pcommand=NULL; /* find and remove invalid command */
static	char args[10][512]= {"","","","","","","","","",""}; /*\cmd{arg}'s*/
char	*pargs[11] = { args[0],args[1],args[2],args[3], /* ptrs to them */
	args[4],args[5],args[6],args[7],args[8],args[9], NULL };
char	display[2048], argstr[256], optstr[1024]; /*displaystring with args*/
char	*strchange();			/* place display where command was */
int	strreplace();			/* replace #1 with args[0], etc */
char	*nomath();			/* change \ to \\, { to \{, etc */
int	iarg=0, iopt=0;			/* args[], optionalargs[] indexes */
/* -------------------------------------------------------------------------
Initialization
-------------------------------------------------------------------------- */
if ( isempty(expression) )		/* no input to validate */
  goto end_of_job;			/* so quit */
/* -------------------------------------------------------------------------
Check each invalid command in list
-------------------------------------------------------------------------- */
for ( ivalid=0; !isempty(invalid[ivalid].command); ivalid++ ) { /*run list*/
  /* --- extract local copy of invalid command list elements --- */
  int	action        = invalid[ivalid].action; /* 0=ignore,1=apply,2=abort*/
  char	*command      = invalid[ivalid].command; /* invalid \command */
  int	nargs         = invalid[ivalid].nargs; /*number of \command {arg}'s*/
  int	myoptionalpos = invalid[ivalid].optionalpos; /*#{args} before [arg]*/
  int	myargformat   = invalid[ivalid].argformat; /* 0={arg},1=arg*/
  char	*displaystring= invalid[ivalid].displaystring; /*display template*/
  /* --- arg format info --- */
  int	nfmt=0,isnegfmt=0,argfmt[9]={0,0,0,0,0,0,0,0,0}; /*argformat digits*/
  /* --- args returned as (char *) if nargs=1 or (char **) if nargs>1 --- */
  void	*argptr = (nargs<2?(void *)args[0]:(void *)pargs); /*(char * or **)*/
  /* --- find and remove/replace all invalid command occurrences --- */
  if ( action < 1 ) continue;		/* ignore this invalid \command */
  optionalpos = myoptionalpos;		/* set global arg for getdirective */
  argformat = myargformat;		/* set global arg for getdirective */
  /* --- interpret argformat digits --- */
  if ( argformat != 0 ) {		/* have argformat */
    int	myfmt = argformat;		/* local copy */
    if ( myfmt < 0 ) { isnegfmt=1; myfmt=(-myfmt); } /* check sign */
    while ( myfmt>0 && nfmt<9 ) {	/* have more format digits */
      argfmt[nfmt] = myfmt%10;		/* store low-order decimal digit */
      myfmt /= 10;			/* and shift it out */
      nfmt++; }				/* count another format digit */
    } /* --- end-of-if(argformat!=0) --- */
  /* --- remove/replace each occurrence of invalid \command --- */
  while ( (pcommand = getdirective(expression,command,1,0,nargs,argptr))
  != NULL ) {				/* found and removed an occurrence */
    ninvalid++;				/* count another invalid \command */
    if ( noptional >= 8 ) noptional=7;	/* don't overflow our buffers */
    /* --- construct optional [arg]...[arg] for display --- */
    *optstr = '\000';			/*init optional [arg]...[arg] string*/
    if ( noptional > 0 )		/* have optional [args] */
      for ( iopt=0; iopt<noptional; iopt++ ) /* construct "[arg]...[arg]" */
	if ( !isempty(optionalargs[iopt]) ) { /* have an optional arg */
	  strcat(optstr,"[");		/* leading [ for optional arg */
	  strcat(optstr,nomath(optionalargs[iopt])); /*optional arg string*/
	  strcat(optstr,"]"); }		/* trailing ] */
    /* --- construct error message display or use supplied template --- */
    if ( isempty(displaystring) ) {	/*replace \command by not~permitted*/
      strcpy(display,"\\mbox{~\\underline{"); /* underline error in \mbox{}*/
      strcat(display,nomath(command));	/* command without \ */
      for ( iarg=0; iarg<nargs; iarg++ ) { /* add {args} and any [args] */
	int ifmt = (nfmt<=iarg?0:argfmt[nfmt-iarg-1]); /* arg format digit */
	if ( iarg==myoptionalpos && noptional>0 ) /* [args] belong here */
	  strcat(display,optstr);	/* insert them before next {arg} */
	if ( isempty(args[iarg]) ) break; /* no more args */
	if(ifmt==0) strcat(display,"\\{"); /* insert leading \{ for arg */
	strcat(display,nomath(args[iarg])); /* arg */
	if(ifmt==0) strcat(display,"\\}"); } /* trailing \} for arg */
      strcat(display,"~not~permitted}~}"); } /* finish error, close \mbox{}*/
    else {				/*replace \command by displaystring*/
      strcpy(display,displaystring);	/* local copy of display template */
      if ( noptional < 1 )		/* no optional args */
	strreplace(display,"[#0]","",0,0); /* so remove tag from template */
      for ( iarg=0; iarg<nargs; iarg++ ) { /* replace #1 with args[0], etc */
	if ( iarg==myoptionalpos && noptional>0 ) /* [args] belong here */
	  strreplace(display,"[#0]",optstr,0,0); /*insert them before {arg}*/
	if ( isempty(args[iarg]) ) break; /* no more args */
	else {				/* have an arg */
	  sprintf(argstr,"#%d",iarg+1);	/* #1 in template displays args[0] */
	  strreplace(display,argstr,nomath(args[iarg]),0,0); } /* replace */
        } /* --- end-of-for(iarg) --- */
      } /* --- end-of-if/else(isempty(displaystring)) --- */
    strchange(0,pcommand,display);	/* place display where command was */
    for ( iarg=0; iarg<10; iarg++ ) *args[iarg] = '\000'; /* reset args */
    } /* --- end-of-while(getdirective()!=NULL) --- */
  } /* --- end-of-for(ivalid) --- */
end_of_job:
  return ( ninvalid );			/* back to caller with #invalid */
} /* --- end-of-function validate() --- */


/* ==========================================================================
 * Function:	advertisement ( expression, message )
 * Purpose:	wrap expression in advertisement message
 * --------------------------------------------------------------------------
 * Arguments:	expression (I/O) pointer to null-terminated char string
 *				containing expression to be "wrapped",
 *				and returning wrapped expression
 *		message (I)	pointer to null-terminated char string
 *				containing template for advertisement
 *				message, or NULL to use default message
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 if successful, 0=error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	advertisement ( char *expression, char *message )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- advertisement template --- */
char  *adtemplate =
	#if defined(ADVERTISEMENT)	/* cc -DADVERTISEMENT=\"filename\" */
	  #include ADVERTISEMENT	/* filename with advertisement */
	#else				/* formatted as illustrated below */
	"\\begin{center}\n"
	"\\fbox{$\\mbox{\\footnotesize\\LaTeX{} rendering courtesy of}\\atop\n"
	"\\mbox{\\scriptsize http://www.forkosh.com/mathtex.html}$}\\\\ \n"
	"\\vspace*{-4mm}\n"
	" %%beginmath%% %%expression%% %%endmath%% \n"
	"\\end{center}\n"
	#endif
	;				/* terminating semicolon */
/* --- other variables --- */
char	adbuffer[MAXEXPRSZ+2048];	/*construct wrapped expression here*/
char	*beginmath[] = { "\\[", "$", " " }, /* start math mode */
	*endmath[] =   { "\\]", "$", " " }; /* end math mode */
int	strreplace();			/* replace %%keywords%% with values*/
/* -------------------------------------------------------------------------
wrap expression in advertisement
-------------------------------------------------------------------------- */
/* --- start with template --- */
if ( isempty(message) )			/* caller didn't supply message */
  message = adtemplate;			/* so use default message */
strcpy(adbuffer,message);		/* copy message template to buffer */
/* --- replace %%beginmath%%...%%endmath%% with \[...\] or with $...$ --- */
  if ( mathmode<0 || mathmode>2 ) mathmode=0; /* out-of-bounds sanity check*/
  if ( isempty(expression) ) mathmode = 2; /*ad only, no need for math mode*/
  strreplace(adbuffer,"%%beginmath%%",beginmath[mathmode],1,0);
  strreplace(adbuffer,"%%endmath%%",endmath[mathmode],1,0);
/* --- replace %%expression%% in template with expression --- */
  strreplace(adbuffer,"%%expression%%",expression,1,0);
/* --- replace original expression --- */
strcpy(expression,adbuffer);		/* expression mow wrapped in ad */
mathmode = 2;				/* \[...\] already in expression */
return ( 1 );				/* always just return 1 */
} /* --- end-of-function advertisement() --- */


/* ==========================================================================
 * Function:	mathlog ( expression, filename )
 * Purpose:	write entry in log file
 * --------------------------------------------------------------------------
 * Arguments:	expression (I)	pointer to null-terminated char string
 *				containing latex math expression
 *		filename (I)	pointer to null-terminated char string
 *				containing filename but not .extension
 *				of output file
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 if successful, 0=error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	mathlog ( char *expression, char *filename )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
FILE	*filefp = NULL;			/* fopen(filename) for logfile */
char	*timestamp();			/* time stamp for logged messages */
char	*makepath();			/* construct full path/filename.ext*/
char	*http_referer = getenv("HTTP_REFERER"); /* referer using mathTeX */
char	*dashes =			/* separates logfile entries */
 "--------------------------------------------------------------------------";
/* -------------------------------------------------------------------------
write message to cache log file
-------------------------------------------------------------------------- */
if ( !isempty(CACHELOG) ) {		/* if a logfile is given */
  if ( (filefp=fopen(makepath(NULL,CACHELOG,NULL),"a")) /*open logfile*/
  !=   NULL ) {				/* ignore logging if can't open */
    int isreflogged = 0;		/* set true if http_referer logged */
    fprintf(filefp,"%s                 %s\n", /* timestamp, md5 file */
    timestamp(TZDELTA,0),		/* timestamp */
    makepath("",filename,extensions[imagetype])); /* hashed filename */
    fprintf(filefp,"%s\n",expression);	/* expression in filename */
    if ( !isempty(http_referer) ) {	/* show referer if we have one */
      int loglen = strlen(dashes);	/* #chars on line in log file*/
      char *refp = http_referer;	/* line to be printed */
      isreflogged = 1;			/* signal http_referer logged*/
      while ( 1 ) {			/* printed in parts if needed*/
	fprintf(filefp,"%.*s\n",loglen,refp); /* print a part */
	if ( strlen(refp) <= loglen ) break;  /* no more parts */
	refp += loglen; }		/* bump ptr to next part */
      } /* --- end-of-if(!isempty(http_referer)) --- */
    if ( !isreflogged )			/* http_referer not logged */
      fprintf(filefp,"http://none\n");	/* so log dummy referer line */
    fprintf(filefp,"%s\n",dashes);	/* separator line */
    fclose(filefp);			/* close logfile immediately */
    } /* --- end-of-if(filefp!=NULL) --- */
  } /* --- end-of-if(!isempty(CACHELOG)) --- */
return ( 1 );
} /* --- end-of-function mathlog() --- */


/* ==========================================================================
 * Function:	makepath ( path, name, extension )
 * Purpose:	return string containing path/name.extension
 * --------------------------------------------------------------------------
 * Arguments:	path (I)	pointer to null-terminated char string
 *				containing "" path or path/ or NULL to
 *				use cachepath if caching enabled
 *		name (I)	pointer to null-terminated char string
 *				containing filename but not .extension
 *				of output file or NULL
 *		extension (I)	pointer to null-terminated char string
 *				containing extension or NULL
 * --------------------------------------------------------------------------
 * Returns:	( char * )	"cachepath/filename.extension" or NULL=error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*makepath ( char *path, char *name, char *extension )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char namebuff[512];		/* buffer for constructed filename */
char	*filename = NULL;		/*ptr to filename returned to caller*/
/* -------------------------------------------------------------------------
construct filename
-------------------------------------------------------------------------- */
/* --- validity checks --- */
/*if ( isempty(name) ) goto end_of_job;*/ /* no name supplied by caller */
/* --- start with caller's path/ or default path to cache directory --- */
*namebuff = '\000';			/* re-init namebuff */
if ( path == NULL ) {			/* use default path to cache */
  if ( !isempty(cachepath) )		/* have a cache path */
    strcpy(namebuff,cachepath); }	/* begin filename with path */
else					/* or use caller's supplied path */
  if ( *path != '\000' )		/* if it's not an empty string */
    strcpy(namebuff,path);		/* begin filename with path */
if ( !isempty(namebuff) )		/* have a leading path */
  if ( !isthischar(lastchar(namebuff),"\\/") ) /* no \ or / at end of path */
    strcat(namebuff,(iswindows?"\\":"/")); /* so add windows\ or unix/ */
/* --- add name after path/ (name arg might just be a blank space) --- */
if ( !isempty(name) ) {			/* name supplied by caller */
  if ( !isempty(namebuff) )		/* and if we already have a path */
    if ( isthischar(*name,"\\/") ) name++; /* skip leading \ or / in name */
  strcat(namebuff,name); }		/* name concatanated after path/ */
/* --- add extension after path/name */
if ( !isempty(extension) ) {		/* have a filename extension */
  if ( !isthischar(lastchar(namebuff),".") ) { /* no . at end of name */
    if ( !isthischar(*extension,".") )	/* and extension has no leading . */
      strcat(namebuff,".");		/* so we need to add our own . */
    strcat(namebuff,extension); }	/* add extension after path/name. */
  else					/* . already at end of name */
    strcat(namebuff,			/* add extension without . */
    (!isthischar(*extension,".")?extension:extension+1)); /*skip leading . */
  } /* --- end-of-if(!isempty(extension)) --- */
filename = namebuff;			/* successful name back to caller */
/*end_of_job:*/
  return ( filename );			/* back with name or NULL=error */
} /* --- end-of-function makepath() --- */


/* ==========================================================================
 * Function:	isfexists ( filename )
 * Purpose:	check whether or not filename exists
 * --------------------------------------------------------------------------
 * Arguments:	filename (I)	pointer to null-terminated char string
 *				containing filename to check for
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 = filename exists, 0 = not
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	isfexists ( char *filename )
{
FILE	*fp = (isempty(filename)?NULL:fopen(filename,"r")); /* try to fopen*/
int	status = 0;			/* init for non-existant filename */
if ( fp != NULL ) {			/* but filename does exist */
  status = 1;				/* so flip status */
  fclose(fp); }				/* and close the file */
return ( status );			/* tell caller if we found filename*/
} /* --- end-of-function isfexists() --- */


/* ==========================================================================
 * Function:	isdexists ( dirname )
 * Purpose:	check whether or not directory exists
 * --------------------------------------------------------------------------
 * Arguments:	dirname (I)	pointer to null-terminated char string
 *				containing directory name to check for
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 = directory exists, 0 = not
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	isdexists ( char *dirname )
{
int	status = 0;			/* init for non-existant dirname */
if ( !isempty(dirname) ) {		/* must have directory name */
  char	directory[512];			/* local copy of dirname */
  DIR	*dp = NULL;			/* dp=opendir() opens directory */
  strcpy(directory,dirname);		/* start with name given by caller */
  if ( !isthischar(lastchar(directory),"/") ) /* no / at end of directory */
    strcat(directory,"/");		/* so add one ourselves */
  if ( (dp=opendir(directory)) != NULL ) { /* dirname exists */
    status = 1;				/* so flip status */
    closedir(dp); }			/* and close the directory */
  } /* --- end-of-if(!isempty(dirname)) --- */
return ( status );			/* tell caller if we found dirname */
} /* --- end-of-function isdexists() --- */


/* ==========================================================================
 * Function:	whichpath ( program, nlocate )
 * Purpose:	determines the path to program with popen("which 'program'")
 * --------------------------------------------------------------------------
 * Arguments:	program (I)	pointer to null-terminated char string
 *				containing program whose path is desired
 *		nlocate (I/0)	addr of int containing NULL to ignore,
 *				or (addr of int containing) 0 to *not*
 *				use locate if which fails.  If non-zero,
 *				use locate if which fails, and return
 *				number of locate lines (if locate succeeds)
 * --------------------------------------------------------------------------
 * Returns:	( char * )	path to program, or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*whichpath ( char *program, int *nlocate )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char pathbuff[256];		/* buffer for returned path */
char	command[256];			/* which program */
FILE    *whichout = NULL;		/* which's stdout */
int	nchars = 0,			/* read whichout one char at a time*/
	pathchar;			/* fgetc(whichout) */
char	*path = NULL;			/* either pathbuff or NULL=error */
char	*locatepath();			/* try locate if which fails */
int	islocate = (nlocate==NULL? 1 : (*nlocate!=0?1:0)); /* 1=use locate*/
/* -------------------------------------------------------------------------
Issue which command and read its output
-------------------------------------------------------------------------- */
/* --- check if which() suppressed --- */
if ( !ISWHICH ) return ( NULL );	/*not running which, return failure*/
/* --- first construct the command --- */
if ( isempty(program) ) goto end_of_job; /* no input */
sprintf(command,"which %s",program);	/* construct command */
/* --- use popen() to invoke which --- */
whichout = popen( command, "r" );	/* issue which and capture stdout */
if( whichout == NULL ) goto end_of_job;	/* failed */
/* --- read the pipe one char at a time --- */
while ( (pathchar=fgetc(whichout))	/* get one more char */
!= EOF ) {				/* not at eof yet */
  pathbuff[nchars] = (char)pathchar;	/* store the char */
  if ( ++nchars >= 255 ) break; }	/* don't overflow buffer */
pathbuff[nchars] = '\000';		/* null-terminate path */
trimwhite(pathbuff);			/*remove leading/trailing whitespace*/
/* --- pclose() waits for command to terminate --- */
pclose( whichout );			/* finish */
if ( nchars > 0 ) {			/* found path with which */
  path = pathbuff;			/* give user successful path */
  if ( islocate && nlocate!=NULL ) *nlocate = 0; } /* signal we used which */
end_of_job:
  if ( path == NULL )			/* which failed to find program */
    if ( islocate )			/* and we're using locate */
      path = locatepath(program,nlocate); /* try locate instead */
  return ( path );			/* give caller path to command */
} /* --- end-of-function whichpath() --- */


/* ==========================================================================
 * Function:	locatepath ( program, nlocate )
 * Purpose:	determines the path to program with popen("locate 'program'")
 * --------------------------------------------------------------------------
 * Arguments:	program (I)	pointer to null-terminated char string
 *				containing program whose path is desired
 *		nlocate (O)	addr to int returning #lines locate|grep
 *				found, or NULL to ignore
 * --------------------------------------------------------------------------
 * Returns:	( char * )	path to program, or NULL for any error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*locatepath ( char *program, int *nlocate )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char pathbuff[256];		/* buffer for returned path */
char	command[256];			/* locate program | grep /program$ */
FILE    *locateout = NULL;		/* locate's stdout */
char	pathline[256];			/* read locateout one line at a time*/
int	nlines = 0;			/* #lines read */
int	linelen=0, pathlen = 9999;	/* choose shortest path */
char	*path = NULL;			/* either pathbuff or NULL=error */
/* -------------------------------------------------------------------------
Issue locate|grep command and read its output
-------------------------------------------------------------------------- */
/* --- first construct the command --- */
if ( isempty(program) ) goto end_of_job; /* no input */
/*if ( strlen(program) < 2 ) goto end_of_job; */ /* might run forever */
sprintf(command,"locate -q -r \"/%s$\" | grep \"bin\"",program);
/* --- use popen() to invoke locate|grep --- */
locateout = popen( command, "r" );	/* issue locate and capture stdout */
if( locateout == NULL ) goto end_of_job; /* failed */
/* --- read the pipe one line at a time --- */
while ( fgets(pathline,255,locateout)	/* get next line */
!= NULL ) {				/* not at eof yet */
  trimwhite(pathline);			/*remove leading/trailing whitespace*/
  if ( (linelen=strlen(pathline)) > 0 ) { /* ignore empty lines */
    if ( linelen < pathlen ) {		/* new shortest path */
      strcpy(pathbuff,pathline);	/* store shortest for caller */
      pathlen = linelen; }		/* and reset new shortest length */
    nlines++; } }			/* count another non-empty line */
/* --- pclose() waits for command to terminate --- */
pclose( locateout );			/* finish */
if ( pathlen>0 && pathlen<256 ) path = pathbuff; /*give user successful path*/
if ( nlocate != NULL ) *nlocate = nlines; /* and number of locate|grep hits*/
end_of_job:
  return ( path );			/* give caller path to command */
} /* --- end-of-function locatepath() --- */


/* ==========================================================================
 * Function:	rrmdir ( path )
 * Purpose:	rm -r path
 * --------------------------------------------------------------------------
 * Arguments:	path (I)	pointer to null-terminated char string
 *				containing path to be rm'ed,
 *				relative to cwd.
 * --------------------------------------------------------------------------
 * Returns:	( int )		0 = success, -1 = error
 * --------------------------------------------------------------------------
 * Notes:     o	Based on Program 4.7, pages 108-111, in
 *		Advanced Programming in the UNIX Environment,
 *		W. Richard Stevens, Addison-Wesley 1992, ISBN 0-201-56317-7
 * ======================================================================= */
/* --- entry point --- */
int	rrmdir ( char *path )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
DIR	*directory = NULL;		/* opendir() opens path directory */
struct	dirent	*entry = NULL;		/* readdir() gives directory entry */
struct	stat	st_info;		/* lstat() gives info about entry */
char	nextpath[512], *pnext=NULL;	/* recurse path/filename in dir */
int	status = (-1);			/* init in case of any error */
/* -------------------------------------------------------------------------
Check file type at path
-------------------------------------------------------------------------- */
if ( path != NULL )			/* have path argument */
  if ( *path != '\000' )		/* and it's not an empty string */
    if ( strcmp(path,".") != 0		/* and it's not "." */
    &&   strcmp(path,"..") != 0 )	/* and also not ".." */
      status = lstat(path,&st_info);	/* what kind of file is at path? */
if ( status < 0 ) goto end_of_job;	/* no kind of file found at path */
/* -------------------------------------------------------------------------
Recurse level if file is a directory
-------------------------------------------------------------------------- */
if ( S_ISDIR(st_info.st_mode) ) {	/* have a directory */
  /* ---
   * Initialization
   * -------------- */
  /* --- init path for each file in directory --- */
  strcpy(nextpath,path);		/* start with path from caller */
  pnext = nextpath + strlen(path);	/* ptr to '\000' at end of path */
  if ( *(pnext-1) != '/' )		/* no trailing / at end of path */
    *pnext++ = '/';			/* so add one */
  *pnext = '\000';			/* null-terminate nextpath */
  /* --- open the directory at caller's path (with trailing /) --- */
  directory = opendir(nextpath);	/* open the directory */
  /* ---
   * Run through all files in directory
   * ---------------------------------- */
  if ( directory != NULL ) {		/* directory successfully opened */
    while ( (entry = readdir(directory)) /* next entry in directory */
    != NULL )				/* NULL signals end-of-directory */
      if ( strcmp(entry->d_name,".") != 0      /* filename not "." */
      &&   strcmp(entry->d_name,"..") != 0 ) { /* and also not ".." */
        strcpy(pnext,entry->d_name);	/* add filename to path */
        status = rrmdir(nextpath); }	/* recurse */
    closedir(directory);		/* close directory after last file */
    showmsg(29,"rrmdir removing directory",path); /* show directory removed*/
    } /* --- end-of-if(directory!=NULL) --- */
  } /* --- end-of-if(S_ISDIR(st_info.st_mode)) --- */
else					/* file isn't a directory */
  showmsg(29,"     rrmdir removing file",path); /* show file removed */
/* -------------------------------------------------------------------------
Remove file or directory (keep it for debugging if msglevel>=9)
-------------------------------------------------------------------------- */
if ( msglevel < 9 ) status = remove(path); /* remove file or directory */
end_of_job:
  return ( status );
} /* --- end-of-function rrmdir() --- */


/* ==========================================================================
 * Function:	rewritecache ( cachefile, maxage )
 * Purpose:	rewrites cached image file with headers and extra image info
 * --------------------------------------------------------------------------
 * Arguments:	cachefile (I)	pointer to null-terminated char string
 *				containing full path to existing image file
 *				written by dvipng or convert
 *		maxage (I)	int containing maxage, in seconds, for
 *				http header
 * --------------------------------------------------------------------------
 * Returns:	( int )		#bytes in original image (0 signals error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	rewritecache ( char *cachefile, int maxage )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	nbytes=0, readcachefile();	/* read cache file */
FILE	*rewriteptr = NULL;		/* write cachefile back over itself*/
unsigned char buffer[MAXGIFSZ+1];	/* bytes from cachefile */
int	i = 0;				/* imageinfo[] index */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- read original image file --- */
if ( (nbytes = readcachefile(cachefile,buffer)) /* read the file */
< 1 ) {					/* file not read */
  msgnumber = EMITFAILED;  goto end_of_job; } /* signal error to caller */
/* --- re-open the file for write --- */
if ( (rewriteptr = fopen(cachefile,"wb")) /*open cachefile for binary write*/
== NULL ) {				/* failed to open for write */
  nbytes = 0;  goto end_of_job; }	/* signal error to caller */
/* --- first emit max-age http header --- */
fprintf( rewriteptr, "Cache-Control: max-age=%d\n",max2(0,maxage) );
/* --- now emit our "special" image info http headers --- */
for ( i=0; imageinfo[i].format!=NULL; i++ ) /* all imageinfo[] fields */
 if ( (imageinfo[i].value) + 9999. > 1.0 ) { /*this field has actual value*/
  double value = imageinfo[i].value;	/* value from latex.info file */
  char  *units = imageinfo[i].units;	/* value units, e.g., "pt" */
  int algorithm= imageinfo[i].algorithm; /* conversion algorithm */
  char *format = imageinfo[i].format;	/* http header output format */
  int  px = 0;				/* depth in px */
  switch( algorithm ) {			/* convert value for http header */
   default: break;
   case 1:				/* depth in pts converted to px */
    if ( 1 || (strstr(units,"pt")!=NULL) ) /* units are pts */
      px = (int)((value/72.)*strtod(density,NULL) + 0.5); /* pts-to-px */
    if ( algorithm == 1 ) px = (-px);	/* vertical alignment is -depth */
    fprintf( rewriteptr, format, px );	/* write http header for depth */
    break;
   } /* --- end-of- switch(algorithm) --- */
  } /* --- end-of-if(imageinfo[i].value!=-9999) --- */
/* --- finally emit content length and type http headers --- */
fprintf( rewriteptr, "Content-Length: %d\n",nbytes );
fprintf( rewriteptr, "Content-type: image/%s\n\n",extensions[imagetype] );
/* -------------------------------------------------------------------------
rewrite bytes from cachefile
-------------------------------------------------------------------------- */
/* --- write bytes back to cachefile --- */
if ( fwrite(buffer,sizeof(unsigned char),nbytes,rewriteptr) /*write buffer*/
<    nbytes )				/* failed to write all bytes */
  nbytes = 0;				/* reset total count to 0 */
end_of_job:
  if ( rewriteptr != NULL ) fclose(rewriteptr); /* close cachefile */
  return ( nbytes );			/* back with #bytes emitted */
} /* --- end-of-function rewritecache() --- */


/* ==========================================================================
 * Function:	emitcache ( cachefile, maxage, isbuffer )
 * Purpose:	dumps bytes from cachefile to stdout
 * --------------------------------------------------------------------------
 * Arguments:	cachefile (I)	pointer to null-terminated char string
 *				containing full path to file to be dumped,
 *				or contains buffer of bytes to be dumped
 *		maxage (I)	int containing maxage, in seconds, for
 *				http header, or -1 to not emit headers
 *		isbuffer (I)	>=1 if cachefile is buffer of bytes to be
 *				dumped, and if isbuffer>=9 it's the #bytes
 *				in buffer to be dumped.  Otherwise,
 *				strlen(cachefile) determines #bytes.
 * --------------------------------------------------------------------------
 * Returns:	( int )		#bytes dumped (0 signals error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	emitcache ( char *cachefile, int maxage, int isbuffer )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	nbytes=0, readcachefile();	/* read cache file */
FILE	*emitptr = stdout;		/* emit cachefile to stdout */
unsigned char buffer[MAXGIFSZ+1];	/* bytes from cachefile */
unsigned char *buffptr = buffer;	/* ptr to buffer */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- check that files opened okay --- */
if ( emitptr == (FILE *)NULL )		/* failed to open emit file */
  goto end_of_job;			/* so return 0 bytes to caller */
/* --- read the file if necessary --- */
if ( isbuffer ) {			/* cachefile is buffer */
 buffptr = (unsigned char *)cachefile;	/* so reset buffer pointer */
 nbytes = (isbuffer<9?strlen((char *)buffptr):isbuffer); }/*determine #bytes*/
else					/* cachefile is file name */
 if ( (nbytes = readcachefile(cachefile,buffer)) /* read the file */
 < 1 ) {				/* file not read */
   msgnumber = EMITFAILED;		/* signal error */
   goto end_of_job; }			/* quit if file not read */
/* --- first emit http headers if requested --- */
if ( isdepth == 0			/* http headers not already in file*/
&& maxage >= 0 )			/* and caller wants http headers */
 { /* --- emit mime content-type line --- */
   fprintf( emitptr, "Cache-Control: max-age=%d\n",maxage );
   fprintf( emitptr, "Content-Length: %d\n",nbytes );
   fprintf( emitptr, "Content-type: image/%s\n\n",extensions[imagetype] ); }
/* -------------------------------------------------------------------------
set stdout to binary mode (for Windows)
-------------------------------------------------------------------------- */
/* emitptr = fdopen(STDOUT_FILENO,"wb"); */  /* doesn't work portably, */
#ifdef WINDOWS				/* so instead... */
  #ifdef HAVE_SETMODE			/* prefer (non-portable) setmode() */
    if ( setmode ( fileno (stdout), O_BINARY) /* windows specific call */
    == -1 ) ; /* handle error */	/* sets stdout to binary mode */
  #else					/* setmode() not available */
    #if 1
      freopen ("CON", "wb", stdout);	/* freopen() stdout binary */
    #else
      stdout = fdopen (STDOUT_FILENO, "wb"); /* fdopen() stdout binary */
    #endif
  #endif
#endif
/* -------------------------------------------------------------------------
emit bytes from cachefile
-------------------------------------------------------------------------- */
/* --- write bytes to stdout --- */
if ( fwrite(buffptr,sizeof(unsigned char),nbytes,emitptr) /* write buffer */
<    nbytes )				/* failed to write all bytes */
  nbytes = 0;				/* reset total count to 0 */
end_of_job:
  return ( nbytes );			/* back with #bytes emitted */
} /* --- end-of-function emitcache() --- */


/* ==========================================================================
 * Function:	readcachefile ( cachefile, buffer )
 * Purpose:	read cachefile into buffer
 * --------------------------------------------------------------------------
 * Arguments:	cachefile (I)	pointer to null-terminated char string
 *				containing full path to file to be read
 *		buffer (O)	pointer to unsigned char string
 *				returning contents of cachefile
 * --------------------------------------------------------------------------
 * Returns:	( int )		#bytes read (0 signals error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	readcachefile ( char *cachefile, unsigned char *buffer )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
FILE	*cacheptr = fopen(cachefile,"rb"); /*open cachefile for binary read*/
unsigned char cachebuff[512];		/* bytes from cachefile */
int	buflen = 256,			/* #bytes we try to read from file */
	nread = 0,			/* #bytes actually read from file */
	maxbytes = MAXGIFSZ,		/* max #bytes returned in buffer */
	nbytes = 0;			/* total #bytes read */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- check that files opened okay --- */
if ( cacheptr == (FILE *)NULL ) goto end_of_job; /*failed to open cachefile*/
/* --- check that output buffer provided --- */
if ( buffer == (unsigned char *)NULL ) goto end_of_job; /* no buffer */
/* -------------------------------------------------------------------------
read bytes from cachefile
-------------------------------------------------------------------------- */
while ( 1 )
  {
  /* --- read bytes from cachefile --- */
  nread = fread(cachebuff,sizeof(unsigned char),buflen,cacheptr); /* read */
  if ( nbytes + nread > maxbytes )	/* block too big for buffer */
    nread = maxbytes - nbytes;		/* so truncate it */
  if ( nread < 1 ) break;		/* no bytes left in cachefile */
  /* --- store bytes in buffer --- */
  memcpy(buffer+nbytes,cachebuff,nread); /* copy current block to buffer */
  /* --- ready to read next block --- */
  nbytes += nread;			/* bump total #bytes emitted */
  if ( nread < buflen ) break;		/* no bytes left in cachefile */
  if ( nbytes >= maxbytes ) break;	/* avoid buffer overflow */
  } /* --- end-of-while(1) --- */
end_of_job:
  if ( cacheptr != NULL ) fclose(cacheptr); /* close file if opened */
  return ( nbytes );			/* back with #bytes emitted */
} /* --- end-of-function readcachefile() --- */


/* ==========================================================================
 * Function:	crc16 ( s )
 * Purpose:	16-bit crc of string s
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		pointer to null-terminated char string
 *				whose crc is desired
 * --------------------------------------------------------------------------
 * Returns:	( int )		16-bit crc of s
 * --------------------------------------------------------------------------
 * Notes:     o	From Numerical Recipes in C, 2nd ed, page 900.
 * ======================================================================= */
/* --- entry point --- */
int	crc16 ( char *s )
{
/* -------------------------------------------------------------------------
Compute the crc
-------------------------------------------------------------------------- */
unsigned short crc = 0;			/* returned crc */
int	ibit;				/* for(ibit) eight one-bit shifts */
while ( !isempty(s) ) {			/* while there are still more chars*/
  crc = (crc ^ (*s)<<8);		/* add next char */
  for ( ibit=0; ibit<8; ibit++ )	/* generator polynomial */
    if ( crc & 0x8000 ) { crc<<=1; crc=crc^4129; }
    else crc <<= 1;
  s++;					/* next xhar */
  } /* --- end-of-while(!isempty(s)) --- */
return ( (int)crc );			/* back to caller with crc */
} /* --- end-of-function crc16() --- */


/* ==========================================================================
 * Function:	md5str ( instr )
 * Purpose:	returns null-terminated character string containing
 *		md5 hash of instr (input string)
 * --------------------------------------------------------------------------
 * Arguments:	instr (I)	pointer to null-terminated char string
 *				containing input string whose md5 hash
 *				is desired
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to null-terminated 32-character
 *				md5 hash of instr
 * --------------------------------------------------------------------------
 * Notes:     o	Other md5 library functions are included below.
 *		They're all taken from Christophe Devine's code,
 *		which (as of 04-Aug-2004) is available from
 *		     http://www.cr0.net:8040/code/crypto/md5/
 *	      o	The P,F,S macros in the original code are replaced
 *		by four functions P1()...P4() to accommodate a problem
 *		with Compaq's vax/vms C compiler.
 * ======================================================================= */
/* --- #include "md5.h" --- */
#ifndef uint8
  #define uint8  unsigned char
#endif
#ifndef uint32
  #define uint32 unsigned long int
#endif
typedef struct
  { uint32 total[2];
    uint32 state[4];
    uint8 buffer[64];
  } md5_context;
void md5_starts( md5_context *ctx );
void md5_update( md5_context *ctx, uint8 *input, uint32 length );
void md5_finish( md5_context *ctx, uint8 digest[16] );
/* --- md5.h --- */
#define GET_UINT32(n,b,i)                       \
  { (n) = ( (uint32) (b)[(i)    ]       )       \
        | ( (uint32) (b)[(i) + 1] <<  8 )       \
        | ( (uint32) (b)[(i) + 2] << 16 )       \
        | ( (uint32) (b)[(i) + 3] << 24 ); }
#define PUT_UINT32(n,b,i)                       \
  { (b)[(i)    ] = (uint8) ( (n)       );       \
    (b)[(i) + 1] = (uint8) ( (n) >>  8 );       \
    (b)[(i) + 2] = (uint8) ( (n) >> 16 );       \
    (b)[(i) + 3] = (uint8) ( (n) >> 24 ); }
/* --- P,S,F macros defined as functions --- */
void P1(uint32 *X,uint32 *a,uint32 b,uint32 c,uint32 d,int k,int s,uint32 t)
  { *a += (uint32)(d ^ (b & (c ^ d))) + X[k] + t;
    *a  = ((*a<<s) | ((*a & 0xFFFFFFFF) >> (32-s))) + b;
    return; }
void P2(uint32 *X,uint32 *a,uint32 b,uint32 c,uint32 d,int k,int s,uint32 t)
  { *a += (uint32)(c ^ (d & (b ^ c))) + X[k] + t;
    *a  = ((*a<<s) | ((*a & 0xFFFFFFFF) >> (32-s))) + b;
    return; }
void P3(uint32 *X,uint32 *a,uint32 b,uint32 c,uint32 d,int k,int s,uint32 t)
  { *a += (uint32)(b ^ c ^ d) + X[k] + t;
    *a  = ((*a<<s) | ((*a & 0xFFFFFFFF) >> (32-s))) + b;
    return; }
void P4(uint32 *X,uint32 *a,uint32 b,uint32 c,uint32 d,int k,int s,uint32 t)
  { *a += (uint32)(c ^ (b | ~d)) + X[k] + t;
    *a  = ((*a<<s) | ((*a & 0xFFFFFFFF) >> (32-s))) + b;
    return; }

/* --- entry point (this one little stub written by me)--- */
char *md5str( char *instr )
  { static char outstr[64];
    unsigned char md5sum[16];
    md5_context ctx;
    int j;
    md5_starts( &ctx );
    md5_update( &ctx, (uint8 *)instr, strlen(instr) );
    md5_finish( &ctx, md5sum );
    for( j=0; j<16; j++ )
      sprintf( outstr + j*2, "%02x", md5sum[j] );
    outstr[32] = '\000';
    return ( outstr ); }

/* --- entry point (all md5 functions below by Christophe Devine) --- */
void md5_starts( md5_context *ctx )
  { ctx->total[0] = 0;
    ctx->total[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476; }

void md5_process( md5_context *ctx, uint8 data[64] )
  { uint32 X[16], A, B, C, D;
    GET_UINT32( X[0],  data,  0 );
    GET_UINT32( X[1],  data,  4 );
    GET_UINT32( X[2],  data,  8 );
    GET_UINT32( X[3],  data, 12 );
    GET_UINT32( X[4],  data, 16 );
    GET_UINT32( X[5],  data, 20 );
    GET_UINT32( X[6],  data, 24 );
    GET_UINT32( X[7],  data, 28 );
    GET_UINT32( X[8],  data, 32 );
    GET_UINT32( X[9],  data, 36 );
    GET_UINT32( X[10], data, 40 );
    GET_UINT32( X[11], data, 44 );
    GET_UINT32( X[12], data, 48 );
    GET_UINT32( X[13], data, 52 );
    GET_UINT32( X[14], data, 56 );
    GET_UINT32( X[15], data, 60 );
    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    P1( X, &A, B, C, D,  0,  7, 0xD76AA478 );
    P1( X, &D, A, B, C,  1, 12, 0xE8C7B756 );
    P1( X, &C, D, A, B,  2, 17, 0x242070DB );
    P1( X, &B, C, D, A,  3, 22, 0xC1BDCEEE );
    P1( X, &A, B, C, D,  4,  7, 0xF57C0FAF );
    P1( X, &D, A, B, C,  5, 12, 0x4787C62A );
    P1( X, &C, D, A, B,  6, 17, 0xA8304613 );
    P1( X, &B, C, D, A,  7, 22, 0xFD469501 );
    P1( X, &A, B, C, D,  8,  7, 0x698098D8 );
    P1( X, &D, A, B, C,  9, 12, 0x8B44F7AF );
    P1( X, &C, D, A, B, 10, 17, 0xFFFF5BB1 );
    P1( X, &B, C, D, A, 11, 22, 0x895CD7BE );
    P1( X, &A, B, C, D, 12,  7, 0x6B901122 );
    P1( X, &D, A, B, C, 13, 12, 0xFD987193 );
    P1( X, &C, D, A, B, 14, 17, 0xA679438E );
    P1( X, &B, C, D, A, 15, 22, 0x49B40821 );
    P2( X, &A, B, C, D,  1,  5, 0xF61E2562 );
    P2( X, &D, A, B, C,  6,  9, 0xC040B340 );
    P2( X, &C, D, A, B, 11, 14, 0x265E5A51 );
    P2( X, &B, C, D, A,  0, 20, 0xE9B6C7AA );
    P2( X, &A, B, C, D,  5,  5, 0xD62F105D );
    P2( X, &D, A, B, C, 10,  9, 0x02441453 );
    P2( X, &C, D, A, B, 15, 14, 0xD8A1E681 );
    P2( X, &B, C, D, A,  4, 20, 0xE7D3FBC8 );
    P2( X, &A, B, C, D,  9,  5, 0x21E1CDE6 );
    P2( X, &D, A, B, C, 14,  9, 0xC33707D6 );
    P2( X, &C, D, A, B,  3, 14, 0xF4D50D87 );
    P2( X, &B, C, D, A,  8, 20, 0x455A14ED );
    P2( X, &A, B, C, D, 13,  5, 0xA9E3E905 );
    P2( X, &D, A, B, C,  2,  9, 0xFCEFA3F8 );
    P2( X, &C, D, A, B,  7, 14, 0x676F02D9 );
    P2( X, &B, C, D, A, 12, 20, 0x8D2A4C8A );
    P3( X, &A, B, C, D,  5,  4, 0xFFFA3942 );
    P3( X, &D, A, B, C,  8, 11, 0x8771F681 );
    P3( X, &C, D, A, B, 11, 16, 0x6D9D6122 );
    P3( X, &B, C, D, A, 14, 23, 0xFDE5380C );
    P3( X, &A, B, C, D,  1,  4, 0xA4BEEA44 );
    P3( X, &D, A, B, C,  4, 11, 0x4BDECFA9 );
    P3( X, &C, D, A, B,  7, 16, 0xF6BB4B60 );
    P3( X, &B, C, D, A, 10, 23, 0xBEBFBC70 );
    P3( X, &A, B, C, D, 13,  4, 0x289B7EC6 );
    P3( X, &D, A, B, C,  0, 11, 0xEAA127FA );
    P3( X, &C, D, A, B,  3, 16, 0xD4EF3085 );
    P3( X, &B, C, D, A,  6, 23, 0x04881D05 );
    P3( X, &A, B, C, D,  9,  4, 0xD9D4D039 );
    P3( X, &D, A, B, C, 12, 11, 0xE6DB99E5 );
    P3( X, &C, D, A, B, 15, 16, 0x1FA27CF8 );
    P3( X, &B, C, D, A,  2, 23, 0xC4AC5665 );
    P4( X, &A, B, C, D,  0,  6, 0xF4292244 );
    P4( X, &D, A, B, C,  7, 10, 0x432AFF97 );
    P4( X, &C, D, A, B, 14, 15, 0xAB9423A7 );
    P4( X, &B, C, D, A,  5, 21, 0xFC93A039 );
    P4( X, &A, B, C, D, 12,  6, 0x655B59C3 );
    P4( X, &D, A, B, C,  3, 10, 0x8F0CCC92 );
    P4( X, &C, D, A, B, 10, 15, 0xFFEFF47D );
    P4( X, &B, C, D, A,  1, 21, 0x85845DD1 );
    P4( X, &A, B, C, D,  8,  6, 0x6FA87E4F );
    P4( X, &D, A, B, C, 15, 10, 0xFE2CE6E0 );
    P4( X, &C, D, A, B,  6, 15, 0xA3014314 );
    P4( X, &B, C, D, A, 13, 21, 0x4E0811A1 );
    P4( X, &A, B, C, D,  4,  6, 0xF7537E82 );
    P4( X, &D, A, B, C, 11, 10, 0xBD3AF235 );
    P4( X, &C, D, A, B,  2, 15, 0x2AD7D2BB );
    P4( X, &B, C, D, A,  9, 21, 0xEB86D391 );
    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D; }

void md5_update( md5_context *ctx, uint8 *input, uint32 length )
  { uint32 left, fill;
    if( length < 1 ) return;
    left = ctx->total[0] & 0x3F;
    fill = 64 - left;
    ctx->total[0] += length;
    ctx->total[0] &= 0xFFFFFFFF;
    if( ctx->total[0] < length )
        ctx->total[1]++;
    if( left && length >= fill )
      { memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        md5_process( ctx, ctx->buffer );
        length -= fill;
        input  += fill;
        left = 0; }
    while( length >= 64 )
      { md5_process( ctx, input );
        length -= 64;
        input  += 64; }
    if( length >= 1 )
      memcpy( (void *) (ctx->buffer + left),
              (void *) input, length ); }

void md5_finish( md5_context *ctx, uint8 digest[16] )
  { static uint8 md5_padding[64] =
     { 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint32 last, padn;
    uint32 high, low;
    uint8 msglen[8];
    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );
    PUT_UINT32( low,  msglen, 0 );
    PUT_UINT32( high, msglen, 4 );
    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );
    md5_update( ctx, md5_padding, padn );
    md5_update( ctx, msglen, 8 );
    PUT_UINT32( ctx->state[0], digest,  0 );
    PUT_UINT32( ctx->state[1], digest,  4 );
    PUT_UINT32( ctx->state[2], digest,  8 );
    PUT_UINT32( ctx->state[3], digest, 12 ); }
/* --- end-of-function md5str() and "friends" --- */


/* ==========================================================================
 * Functions:	int  unescape_url ( char *url )
 *		char x2c ( char *what )
 * Purpose:	unescape_url replaces 3-character sequences %xx in url
 *		    with the single character represented by hex xx.
 *		x2c returns the single character represented by hex xx
 *		    passed as a 2-character sequence in what.
 * --------------------------------------------------------------------------
 * Arguments:	url (I)		char * containing null-terminated
 *				string with embedded %xx sequences
 *				to be converted.
 *		what (I)	char * whose first 2 characters are
 *				interpreted as ascii representations
 *				of hex digits.
 * --------------------------------------------------------------------------
 * Returns:	( int )		length of url string after replacements.
 *		( char )	x2c returns the single char
 *				corresponding to hex xx passed in what.
 * --------------------------------------------------------------------------
 * Notes:     o	These two functions were taken from util.c in
 *   ftp://ftp.ncsa.uiuc.edu/Web/httpd/Unix/ncsa_httpd/cgi/ncsa-default.tar.Z
 *	      o	Added ^M,^F,etc, and +'s to blank xlation 0n 01-Oct-06
 * ======================================================================= */
/* --- entry point --- */
int unescape_url ( char *url ) {
    int x=0, y=0;
    char x2c();
    static char *hex="0123456789ABCDEFabcdef";
    int xlatectrl=1, xlateblank=0/*1*/;
    /* ---
     * first xlate ctrl chars and +'s to blanks
     * ---------------------------------------- */
    if ( xlatectrl || xlateblank ) {	/*xlate ctrl chars and +'s to blanks*/
      char *ctrlchars = (!xlateblank? "\n\t\v\b\r\f\a\015" :
           (!xlatectrl? "+" : "+\n\t\v\b\r\f\a\015" ));
      int  urllen = strlen(url);	/* total length of url string */
      /* --- replace ctrlchars with blanks --- */
      while ( (x=strcspn(url,ctrlchars)) < urllen ) /* found a ctrlchar */
	url[x] = ' ';			/*replace ctrl char or + with blank*/
      /* --- get rid of leading/trailing ctrlchars (now whitespace) --- */
      trimwhite(url);			/*remove leading/trailing whitespace*/
      } /* --- end-of-if(xlatectrl||xlateblank) --- */
    /* ---
     * now xlate %nn to corresponding char (original ncsa code)
     * -------------------------------------------------------- */
    for(x=y=0;url[y];++x,++y)
      if((url[x] = url[y]) == '%')
	if(isthischar(url[y+1],hex)
	&& isthischar(url[y+2],hex))
	  { url[x] = x2c(&url[y+1]);
	    y+=2; }
    url[x] = '\000';
    return x;
} /* --- end-of-function unescape_url() --- */
/* --- entry point --- */
char x2c(char *what) {
    char digit;
    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
} /* --- end-of-function x2c() --- */


/* ==========================================================================
 * Function:	timelimit ( char *command, int killtime )
 * Purpose:	Issues a system(command) call, but throttles command
 *		after killtime seconds if it hasn't already completed.
 * --------------------------------------------------------------------------
 * Arguments:	command (I)	char * to null-terminated string
 *				containing system(command) to be executed
 *		killtime (I)	int containing maximum #seconds
 *				to allow command to run
 * --------------------------------------------------------------------------
 * Returns:	( int )		return status from command,
 *				or -1 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	The timelimit() code is adapted from
 *		   http://devel.ringlet.net/sysutils/timelimit/
 *		Compile with -DTIMELIMIT=\"$(which timelimit)\" to use an
 *		installed copy of timelimit rather than this built-in code.
 *	      o if symbol ISCOMPILETIMELIMIT is false, a stub function
 *		that just issues system(command) is compiled instead.
 * ======================================================================= */
#if !ISCOMPILETIMELIMIT
/* --- entry point for stub timelimit() function --- */
int	timelimit(char *command, int killtime) {
  if ( isempty(command) )		/* no command given */
    return( (killtime==(-99)?991:(-1)) ); /* return -1 or stub identifier */
  return ( system(command) ); }		/* just issue system(command) */
#else
/* --- entry points for signal handlers --- */
void	sigchld(int sig)    { fdone  = 1; }
void	sigalrm(int sig)    { falarm = 1; }
void	sighandler(int sig) { sigcaught = sig; fsig = 1; }

/* --- entry point --- */
int	timelimit(char *command, int killtime)
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
pid_t	pid = 0;
int	killsig = (int)(SIGKILL);
int	setsignal();
int	status = (-1);
/* -------------------------------------------------------------------------
check args
-------------------------------------------------------------------------- */
if ( isempty(command) )			/* no command given */
  return( (killtime==(-99)?992:(-1)) );	/* return -1 or built-in identifier*/
if ( killtime < 1   ) return ( system(command) ); /* throttling disabled */
if ( killtime > 999 ) killtime = 999;	/* default maximum to 999 seconds */
/* -------------------------------------------------------------------------
install signal handlers
-------------------------------------------------------------------------- */
fdone = falarm = fsig = sigcaught = 0;
if ( setsignal(SIGALRM, sigalrm)    < 0 ) return(-1);
if ( setsignal(SIGCHLD, sigchld)    < 0 ) return(-1);
if ( setsignal(SIGTERM, sighandler) < 0 ) return(-1);
if ( setsignal(SIGHUP,  sighandler) < 0 ) return(-1);
if ( setsignal(SIGINT,  sighandler) < 0 ) return(-1);
if ( setsignal(SIGQUIT, sighandler) < 0 ) return(-1);
/* -------------------------------------------------------------------------
fork off the child process
-------------------------------------------------------------------------- */
fflush(NULL);				/* flush all buffers before fork */
if ( (pid=fork()) < 0 ) return(-1);	/* failed to fork */
if ( pid == 0 ) {			/* child process... */
  status = system(command);		/* ...submits command */
  _exit(status); }			/* and _exits without user cleanup */
/* -------------------------------------------------------------------------
parent process sleeps for allowed time
-------------------------------------------------------------------------- */
alarm(killtime);
while ( !(fdone||falarm||fsig) ) pause();
alarm(0);
/* -------------------------------------------------------------------------
send kill signal if child hasn't completed command
-------------------------------------------------------------------------- */
if ( fsig )  return(-1);		/* some other signal stopped child */
if ( !fdone ) kill(pid, killsig);	/* not done, so kill it */
/* -------------------------------------------------------------------------
return status of child pid
-------------------------------------------------------------------------- */
if ( waitpid(pid, &status, 0) == -1 ) return(-1); /* can't get status */
if ( 1 ) return(status);		/* return status to caller */
#if 0					/* interpret status */
  if ( WIFEXITED(status) )
	return (WEXITSTATUS(status));
  else if ( WIFSIGNALED(status) )
	return (WTERMSIG(status) + 128);
  else
	return (EX_OSERR);
#endif
} /* --- end-of-function timelimit() --- */

/* --- entry point --- */
int	setsignal ( int sig, void (*handler)(int) )
{
 #ifdef HAVE_SIGACTION
    struct  sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    act.sa_flags = 0;
    #ifdef SA_NOCLDSTOP
      act.sa_flags |= SA_NOCLDSTOP;
    #endif
    if (sigaction(sig, &act, NULL) < 0) return(-1);
 #else
    if (signal(sig, handler) == SIG_ERR) return(-1);
 #endif
    return(0);
} /* --- end-of-function setsignal() --- */
#endif /* ISCOMPILETIMELIMIT */


/* ==========================================================================
 * Function:	getdirective(string, directive, iscase, isvalid, nargs, args)
 * Purpose:	Locates the first \directive{arg1}...{nargs} in string,
 *		returns arg1...nargs in args[],
 *		and removes \directive and its args from string.
 * --------------------------------------------------------------------------
 * Arguments:	string (I/0)	char * to null-terminated string from which
 *				the first occurrence of \directive will be
 *				interpreted and removed
 *		directive (I)	char * to null-terminated string containing
 *				the \directive to be interpreted in string
 *		iscase (I)	int containing 1 if match of \directive
 *				in string should be case-sensitive,
 *				or 0 if match is case-insensitive.
 *		isvalid (I)	int containing validity check option:
 *				0=no checks, 1=must be numeric
 *		nargs (I)	int containing (maximum) number of
 *				{args} following \directive, or 0 if none.
 *		args (O)	void * interpreted as (char *) if nargs=1
 *				to return the one and only arg,
 *				or interpreted as (char **) if nargs>1
 *				to array of returned arg strings
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to first char after removed \directive, or
 *				NULL if \directive not found, or any error.
 * --------------------------------------------------------------------------
 * Notes:     o	If optional [arg]'s are found, they're stored in the global
 *		optionalargs[] buffer, and the noptional counter is bumped.
 *	      o	set global argformat's decimal digits for each arg,
 *		e.g., 1357... means 1 for 1st arg, 3 for 2nd, 5 for 3rd, etc.
 *		0 for an arg is the default format (i.e., argformat=0),
 *		and means it's formatted as a LaTeX {arg} or [arg].
 *		1 for an arg means arg terminated by first non-alpha char
 *		2 means arg terminated by {   (e.g., as for /def)
 *		8 means arg terminated by first whitespace char
 * ======================================================================= */
/* --- entry point --- */
char	*getdirective ( char *string, char *directive,
	int iscase, int isvalid, int nargs, void *args )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	iarg = (-1);			/* init to signal error */
char	*pfirst = NULL,			/* ptr to 1st char of directive */
	*plast = NULL,			/* ptr past last char of last arg */
	*plbrace=NULL, *prbrace=NULL;	/* ptr to left,right brace of arg */
int	fldlen = 0;			/* #chars between { and } delims */
char	argfld[512];			/* {arg} characters */
int	nfmt=0,isnegfmt=0,argfmt[9]={0,0,0,0,0,0,0,0,0}; /*argformat digits*/
int	gotargs = (args==NULL?0:1);	/* true if args array supplied */
int	isdalpha = 1;			/* true if directive ends with alpha*/
char	*strpspn(char *s,char *reject,char *segment); /*non-() not in rej*/
/* -------------------------------------------------------------------------
Find first \directive in string
-------------------------------------------------------------------------- */
noptional = 0;				/* no optional [args] yet */
for ( iarg=0; iarg<8; iarg++ )		/* for each one... */
  *optionalargs[iarg] = '\000';		/* re-init optional [arg] buffer */
if ( argformat != 0 ) {			/* have argformat */
  int	myfmt = argformat;		/* local copy */
  if ( myfmt < 0 ) { isnegfmt=1; myfmt=(-myfmt); } /* check sign */
  while ( myfmt>0 && nfmt<9 ) {		/* have more format digits */
    argfmt[nfmt] = myfmt%10;		/* store low-order decimal digit */
    myfmt /= 10;			/* and shift it out */
    nfmt++; }				/* count another format digit */
  } /* --- end-of-if(argformat!=0) --- */
if ( isempty(directive) ) goto end_of_job; /* no input \directive given */
if ( !isalpha((int)(directive[strlen(directive)-1])) )isdalpha=0;/*not alpha*/
pfirst = string;			/* start at beginning of string */
while ( 1 ) {				/* until we find \directive */
  if ( !isempty(pfirst) )		/* still have string from caller */
    pfirst =				/* ptr to 1st char of directive */
     (iscase>0? strstr(pfirst,directive): /* case-sensistive match */
      strcasestr(pfirst,directive));	/* case-insensistive match */
  if ( isempty(pfirst) ) {		/* \directive not found in string */
    pfirst = NULL;			/* signal \directive not found */
    goto end_of_job; }			/* quit, signalling error to caller*/
  plast = pfirst + strlen(directive);	/*ptr to fist char past directive*/
  if ( !isdalpha || !isalpha((int)(*plast)) ) break; /* found \directive */
  pfirst = plast;			/* keep looking */
  plast = NULL;				/* reset plast */
  } /* --- end-of-while(1) --- */
if ( nargs < 0 ) {			/* optional [arg] may be present */
  nargs = -nargs;			/* flip sign back to positive */
  /*noptional = 1;*/ }			/* and set optional flag */
/* -------------------------------------------------------------------------
Get arguments
-------------------------------------------------------------------------- */
iarg = 0;				/* no args yet */
if ( nargs > 0 )			/* \directive has {args} */
  while ( iarg < nargs+noptional ) {	/* get each arg */
    int karg = iarg-noptional;		/* non-optional arg index */
    int kfmt = (nfmt<=karg?0:argfmt[nfmt-karg-1]); /* arg format digit */
    /* --- find left { and right } arg delimiters --- */
    plbrace = plast;			/*ptr to fist char past previous arg*/
    skipwhite(plbrace);			/* push it to first non-white char */
    if ( isempty(plbrace) ) break;	/* reached end-of-string */
    /* --- check LaTeX for single-char arg or {arg} or optional [arg] --- */
    if ( kfmt == 0 ) {			/* interpret LaTeX {arg} format */
     if ( !isthischar(*plbrace,(iarg==optionalpos+noptional?"{[":"{")) ) {
      /* --- single char argument --- */
      plast = plbrace + 1;		/* first char after single-char arg*/
      argfld[0] = *plbrace;		/* arg field is this one char */
      argfld[1] = '\000'; }		/* null-terminate field */
     else {				/* have {arg} or optional [arg] */
      /* note: to use for validation, need robust {} match like strpspn() */
      if ( (prbrace = strchr(plbrace,(*plbrace=='{'?'}':']'))) /*right }or]*/
      ==   NULL ) break;		/*and no more args if no right brace*/
      if ( 1 )				/* true to use strpspn() */
	prbrace = strpspn(plbrace,NULL,NULL); /* push to matching } or ] */
      plast = prbrace + 1;		/* first char after right brace */
      /* --- extract arg field between { and } delimiters --- */
      fldlen = (int)(prbrace-plbrace) - 1; /* #chars between { and } delims*/
      if ( fldlen >= 256 ) fldlen=255;	/* don't overflow argfld[] buffer */
      if ( fldlen > 0 )			/* have chars in field */
        memcpy(argfld,plbrace+1,fldlen); /*copy field chars to local buffer*/
      argfld[fldlen] = '\000';		/* and null-terminate field */
      trimwhite(argfld);		/* trim whitespace from argfld */
      } /* --- end-of-if/else(!isthischar(*plbrace,...)) --- */
     } /* --- end-of-if(kfmt==0) --- */
    /* --- check plain TeX for arg terminated by whitespace --- */
    if ( kfmt != 0 ) {			/* interpret plain TeX arg format */
     char *parg = NULL;			/* ptr into arg, used as per kfmt */
     plast = plbrace;			/* start at first char of arg */
     if ( *plast == '\\' ) plast++;	/* skip leading \command backslash */
     /* --- interpret arg according to its format --- */
     switch ( kfmt ) {
       case 1:
       default: skipcommand(plast); break; /* push ptr to non-alpha char */
       case 2: parg = strchr(plast,'{'); /* next arg always starts with { */
	if ( parg != NULL ) plast=parg; else plast++; /* up to { or 1 char */
	break;
       case 8: findwhite(plast); break;	/*ptr to whitespace after last char*/
       } /* --- end-of-switch(kfmt) --- */
     /* --- extract arg field --- */
     fldlen = (int)(plast-plbrace);	/* #chars between in field */
     if ( fldlen >= 256 ) fldlen=255;	/* don't overflow argfld[] buffer */
     if ( fldlen > 0 )			/* have chars in field */
       memcpy(argfld,plbrace,fldlen);	/*copy field chars to local buffer*/
     argfld[fldlen] = '\000';		/* and null-terminate field */
     if ( 1 ) { trimwhite(argfld); }	/* trim whitespace from argfld */
     } /* --- end-of-if(kfmt!=0) --- */
    if ( isvalid != 0 ) {		/* argfld[] validity check desired */
     if ( isvalid == 1 ) {		/* numeric check wanted */
       int validlen = strspn(argfld," +-.0123456789"); /*very simple check*/
       argfld[validlen] = '\000'; }	/* truncate invalid chars */
     } /* --- end-of-if(isvalid!=0) --- */
    /* --- store argument field in caller's array --- */
    if ( kfmt==0 && *plbrace=='[' ) {	/*store [arg] as optionalarg instead*/
     if ( noptional < 8 ) {		/* don't overflow our buffer */
       strninit(optionalargs[noptional],argfld,254); } /*copy to optionalarg*/
     noptional++; }			/* count another optional [arg] */
    else				/*{args} returned in caller's array*/
     if ( gotargs ) {			/*caller supplied address or array*/
      if ( nargs < 2 )			/*just one arg, so it's an address*/ 
        strcpy((char *)args,argfld);	/* so copy arg field there */
      else {				/* >1 arg, so it's a ptr array */
	char *argptr = ((char **)args)[karg]; /* arg ptr in array of ptrs */
	if ( argptr != NULL )		/* array has iarg-th address */
	  strcpy(argptr,argfld);	/* so copy arg field there */
	else gotargs = 0; } }		/* no more addresses in array */
    /* --- completed this arg --- */
    iarg++;				/* bump arg count */
    } /* --- end-of-while(iarg<nargs) --- */
/* -------------------------------------------------------------------------
Back to caller
-------------------------------------------------------------------------- */
end_of_job:
  if ( 1 ) argformat = 0;		/* always/never reset global arg */
  if ( 1 ) optionalpos = 0;		/* always/never reset global arg */
  if ( pfirst!=NULL && plast!=NULL )	/* have directive field delims */
    {strsqueeze(pfirst,((int)(plast-pfirst)));} /* squeeze out directive */
  return ( pfirst );			/* ptr to 1st char after directive */
} /* --- end-of-function getdirective() --- */


/* ==========================================================================
 * Function:	mathprep ( expression )
 * Purpose:	preprocessor for mathTeX input, e.g.,
 *		(a) removes leading/trailing $'s from $$expression$$
 *		(b) xlates &html; special chars to equivalent latex
 *		(c) xlates &#nnn; special chars to equivalent latex
 *		Should only be called once (after unescape_url())
 * --------------------------------------------------------------------------
 * Arguments:	expression (I/O) char * to first char of null-terminated
 *				string containing mathTeX/LaTeX expression,
 *				and returning preprocessed string
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to input expression,
 *				or NULL for any parsing error.
 * --------------------------------------------------------------------------
 * Notes:     o	The ten special symbols  $ & % # _ { } ~ ^ \  are reserved
 *		for use in LaTeX commands.  The corresponding directives
 *		\$ \& \% \# \_ \{ \}  display the first seven, respectively,
 *		and \backslash displays \.  It's not clear to me whether
 *		or not mathprep() should substitute the displayed symbols,
 *		e.g., whether &#36; better xlates to \$ or to $.
 *		Right now, it's the latter.
 * ======================================================================= */
/* --- entry point --- */
char	*mathprep ( char *expression )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	isym=0, inum=0;			/* symbols[], numbers[] indexes */
char	*strchange();			/* change leading chars of string */
char	*strwstr();			/*use strwstr() instead of strstr()*/
int	strreplace();			/* substitute/from/to/ */
int	ndollars = 0;			/* #leading/trailing $$...$$'s */
int	explen = (isempty(expression)?0:strlen(expression)); /*#input chars*/
/* ---
 * html special/escape chars converted to latex equivalents
 * -------------------------------------------------------- */
char	*htmlsym=NULL;			/* symbols[isym].html */
static	struct { char *html; char *termchar; char *latex; } symbols[] =
 { /* ---------------------------------------------------------
     user-supplied newcommands (different than -DNEWCOMMAND)
   --------------------------------------------------------- */
 #ifdef NEWCOMMANDS			/* -DNEWCOMMANDS=\"filename.h\" */
   #include NEWCOMMANDS
 #endif
   /* ----------------------------------------
    html char termchar  LaTeX equivalent...
   ---------------------------------------- */
   { "&quot",	";",	"\"" },		/* &quot; is first, &#034; */
   { "&amp",	";",	"&" },
   { "&lt",	";",	"<" },
   { "&gt",	";",	">" },
   { "&backslash",";",	"\\" },
   { "&nbsp",	";",	" " /*"~"*/ },
   { "&iexcl",	";",	"{\\mbox{!`}}" },
   { "&brvbar",	";",	"|" },
   { "&plusmn",	";",	"\\pm" },
   { "&sup2",	";",	"{{}^2}" },
   { "&sup3",	";",	"{{}^3}" },
   { "&micro",	";",	"\\mu" },
   { "&sup1",	";",	"{{}^1}" },
   { "&frac14",	";",	"{\\frac14}" },
   { "&frac12",	";",	"{\\frac12}" },
   { "&frac34",	";",	"{\\frac34}" },
   { "&iquest",	";",	"{\\mbox{?`}}" },
   { "&Acirc",	";",	"{\\rm\\hat A}" },
   { "&Atilde",	";",	"{\\rm\\tilde A}" },
   { "&Auml",	";",	"{\\rm\\ddot A}" },
   { "&Aring",	";",	"{\\overset{o}{\\rm A}}" },
   { "&atilde",	";",	"{\\rm\\tilde a}" },
   { "&yuml",	";",	"{\\rm\\ddot y}" },  /* &yuml; is last, &#255; */
   { "&#",	";",	"{[\\&\\#nnn?]}" },  /* all other &#nnn's */
   /* ----------------------------------------
    html tag  termchar  LaTeX equivalent...
   ---------------------------------------- */
   { "< br >",	NULL,	" \000" /*"\\\\"*/ },
   { "< br / >", NULL,	" \000" /*"\\\\"*/ },
   { "< dd >",	NULL,	" \000" },
   { "< / dd >", NULL,	" \000" },
   { "< dl >",	NULL,	" \000" },
   { "< / dl >", NULL,	" \000" },
   { "< p >",	NULL,	" \000" },
   { "< / p >",	NULL,	" \000" },
   /* ---------------------------------------
    garbage  termchar  LaTeX equivalent...
   --------------------------------------- */
   { "< tex >",	NULL,	"\000" },
   { "< / tex >", NULL,	"\000" },
   { NULL,	NULL,	NULL }
 } ; /* --- end-of-symbols[] --- */
/* ---
 * html &#nn chars converted to latex equivalents
 * ---------------------------------------------- */
int	htmlnum=0;			/* numbers[inum].html */
static	struct { int html; char *latex; } numbers[] =
 { /* ---------------------------------------
    html num  LaTeX equivalent...
   --------------------------------------- */
   { 9,		" " },			/* horizontal tab */
   { 10,	" " },			/* line feed */
   { 13,	" " },			/* carriage return */
   { 32,	" " },			/* space */
   { 33,	"!" },			/* exclamation point */
   { 34,	"\"" },			/* &quot; */
   { 35,	"#" },			/* hash mark */
   { 36,	"$" },			/* dollar */
   { 37,	"%" },			/* percent */
   { 38,	"&" },			/* &amp; */
   { 39,	"\'" },			/* apostrophe (single quote) */
   { 40,	")" },			/* left parenthesis */
   { 41,	")" },			/* right parenthesis */
   { 42,	"*" },			/* asterisk */
   { 43,	"+" },			/* plus */
   { 44,	"," },			/* comma */
   { 45,	"-" },			/* hyphen (minus) */
   { 46,	"." },			/* period */
   { 47,	"/" },			/* slash */
   { 58,	":" },			/* colon */
   { 59,	";" },			/* semicolon */
   { 60,	"<" },			/* &lt; */
   { 61,	"=" },			/* = */
   { 62,	">" },			/* &gt; */
   { 63,	"\?" },			/* question mark */
   { 64,	"@" },			/* commercial at sign */
   { 91,	"[" },			/* left square bracket */
   { 92,	"\\" },			/* backslash */
   { 93,	"]" },			/* right square bracket */
   { 94,	"^" },			/* caret */
   { 95,	"_" },			/* underscore */
   { 96,	"`" },			/* grave accent */
   { 123,	"{" },			/* left curly brace */
   { 124,	"|" },			/* vertical bar */
   { 125,	"}" },			/* right curly brace */
   { 126,	"~" },			/* tilde */
   { 160,	"~" },			/* &nbsp; (use tilde for latex) */
   { 166,	"|" },			/* &brvbar; (broken vertical bar) */
   { 173,	"-" },			/* &shy; (soft hyphen) */
   { 177,	"{\\pm}" },		/* &plusmn; (plus or minus) */
   { 215,	"{\\times}" },		/* &times; (plus or minus) */
   { -999,	NULL }
 } ; /* --- end-of-numbers[] --- */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
if ( explen < 1 ) goto end_of_job;	/* no input expression supplied */
/* -------------------------------------------------------------------------
remove leading/trailing $$...$$'s and set mathmode accordingly
-------------------------------------------------------------------------- */
/* --- count and remove leading/trailing $'s from $$expression$$ --- */
while ( explen > 2 )			/* don't exhaust entire expression */
  if ( expression[0] == '$'		/* have leading $ char */
  &&   expression[explen-1] == '$' ) {	/* and trailing $ char */
    explen -= 2;			/* remove leading and trailing $'s */
    strsqueeze(expression,1);		/* squeeze out leading $ */
    expression[explen] = '\000';	/* and terminate at trailing $ */
    ndollars++; }			/* count another dollar */
  else break;				/* no more $...$ pairs */
/* --- set mathmode for input $$expression$$ --- */
if ( ndollars > 0 )			/* have $$expression$$ input */
  switch ( ndollars ) {			/* set mathmode accordingly */
    case 1: mathmode = 1; break;	/* $...$ is \textstyle */
    case 2: mathmode = 0; break;	/* $$...$$ is \displaystyle */
    case 3: mathmode = 2; break;	/* $$$...$$$ is \parstyle */
    default: break; }			/* I have no idea what you want */
/* --- check for input \[expression\] if no $$'s --- */
if ( ndollars < 1 )			/* not an $$expression$$ */
  if ( explen > 4 )			/* long enough to contain \[...\] */
    if ( strncmp(expression,"\\[",2) == 0 /* so check for leading \[ */
    &&   strncmp(expression+explen-2,"\\]",2) == 0 ) { /* and trailing \] */
      explen -= 4;			/* remove leading/trailing \[...\] */
      strsqueeze(expression,2);		/* squeeze out leading \[ */
      expression[explen] = '\000';	/* and terminate at trailing \] */
      mathmode = 0; }			/* set \displaystyle */
/* -------------------------------------------------------------------------
run thru table, converting all occurrences of each html to latex equivalent
-------------------------------------------------------------------------- */
for ( isym=0; (htmlsym=symbols[isym].html)!=NULL; isym++ )
  {
  char	*htmlterm = symbols[isym].termchar, /* &symbol; terminator */
	*latexsym = symbols[isym].latex, /* latex replacement */
	errorsym[256];			/* error message replacement */
  int	htmllen = strlen(htmlsym),	/* length of html token */
	wstrlen = htmllen,		/* token length found by strwstr() */
	latexlen = strlen(latexsym);	/* length of latex replacement */
  int	isstrwstr = 1,			/* true to use strwstr() */
	istag = (isthischar(*htmlsym,"<")?1:0),	/*html <tag> starts with <*/
	isamp = (isthischar(*htmlsym,"&")?1:0);	/*html char starts with & */
  char	wstrwhite[128] = "i";		/* whitespace chars for strwstr() */
  char	*expptr = expression,		/* ptr within expression */
	*tokptr = NULL;			/*ptr to token found in expression*/
  /* ---
   * xlate every occurrence of current htmlsym command
   * ------------------------------------------------- */
  skipwhite(htmlsym);			/*skip any bogus leading whitespace*/
  htmllen = wstrlen = strlen(htmlsym);	/*reset length of html token and...*/
  istag = (isthischar(*htmlsym,"<")?1:0); /* ...html <tag> starts with < */
  isamp = (isthischar(*htmlsym,"&")?1:0); /* ...html char starts with & */
  if ( isamp ) isstrwstr = 0;		/* don't use strwstr() for &char */
  if ( istag ) {			/* use strwstr() for <tag> */
    isstrwstr = 1;			/* make sure flag is set true */
    if ( !isempty(htmlterm) )		/* got a term char string */
      strninit(wstrwhite,htmlterm,64);	/* interpret it as whitespace arg */
    htmlterm = NULL; }			/* rather than as a terminater */
  while ( (tokptr=(!isstrwstr?strstr(expptr,htmlsym):  /*strtsr or strwstr*/
  strwstr(expptr,htmlsym,wstrwhite,&wstrlen))) /*looks for another htmlsym*/
  != NULL ) {				/* we found another htmlsym */
    char termchar = *(tokptr+wstrlen),	/* char terminating html sequence */
         prevchar = (tokptr==expptr?' ':*(tokptr-1)); /*char preceding tok*/
    int  toklen = wstrlen;		/* tot length of token+terminator */
    /* --- ignore match if leading char escaped (not really a match) --- */
    if ( isthischar(prevchar,"\\") ) {	/* inline symbol escaped */
        expptr = tokptr+toklen;		/*just resume search after literal*/
        continue; }			/* but don't replace it */
    /* --- ignore match if it's just a prefix of a longer expression --- */
    if ( !istag )			/*<br>-type command can't be prefix*/
      if ( isalpha((int)termchar) ) {	/*we just have prefix of longer sym*/
        expptr = tokptr+toklen;		/* just resume search after prefix */
        continue; }			/* but don't replace it */
    /* --- check for &# prefix signalling &#nnn --- */
    if ( strcmp(htmlsym,"&#") == 0 ) {	/* replacing special &#nnn; chars */
      /* --- accumulate chars comprising number following &# --- */
      char anum[32];			/* chars comprising number after &# */
      inum = 0;				/* no chars accumulated yet */
      while ( termchar != '\000' ) {	/* don't go past end-of-string */
        if ( !isdigit((int)termchar) ) break; /* and don't go past digits */
        if ( inum > 10 ) break;		/* some syntax error in expression */
        anum[inum] = termchar;		/* accumulate this digit */
        inum++;  toklen++;		/* bump field length, token length */
        termchar = *(tokptr+toklen); }	/* char terminating html sequence */
      anum[inum] = '\000';		/* null-terminate anum */
      /* --- look up &#nnn in number[] table --- */
      htmlnum = atoi(anum);		/* convert anum[] to an integer */
      latexsym = errorsym;		/* init latex replacement for error*/
      strninit(latexsym,symbols[isym].latex,128); /* init error message */
      strreplace(latexsym,"nnn",anum,1,1); /* place actual &#num in message*/
      latexlen = strlen(latexsym);	/* and length of latex replacement */
      for ( inum=0; numbers[inum].html>=0; inum++ ) /* run thru numbers[] */
        if ( htmlnum ==  numbers[inum].html ) { /* till we find a match */
	  latexsym = numbers[inum].latex; /* latex replacement */
	  latexlen = strlen(latexsym);	/* length of latex replacement */
          break; }			/* no need to look any further */
      } /* --- end-of-if(strcmp(htmlsym,"&#")==0) --- */
    /* --- check for optional ; terminator after &symbol --- */
    if ( !istag )			/* html <tag> doesn't have term. */
      if ( termchar != '\000' )		/* token not at end of expression */
        if ( !isempty(htmlterm) )	/* sequence may have terminator */
          toklen += (isthischar(termchar,htmlterm)?1:0); /*add terminator*/
    /* --- replace html command with latex equivalent --- */
    strchange(toklen,tokptr,latexsym);	/* replace html symbol with latex */
    expptr = tokptr + latexlen;		/* resume search after replacement */
    } /* --- end-of-while(tokptr!=NULL) --- */
  } /* --- end-of-for(isymbol) --- */
/* -------------------------------------------------------------------------
back to caller with preprocessed expression
-------------------------------------------------------------------------- */
trimwhite(expression);			/*remove leading/trailing whitespace*/
showmsg(99,"mathprep expression",expression); /*show preprocessed expression*/
end_of_job:
  return ( expression );
} /* --- end-of-function mathprep() --- */


/* ==========================================================================
 * Function:	strwstr (char *string, char *substr, char *white, int *sublen)
 * Purpose:	Find first substr in string, but wherever substr contains
 *		a whitespace char (in white), string may contain any number
 *		(including 0) of whitespace chars. If white contains I or i,
 *		then match is case-insensitive (and I,i _not_ whitespace).
 * --------------------------------------------------------------------------
 * Arguments:	string (I)	char * to null-terminated string in which
 *				first occurrence of substr will be found
 *		substr (I)	char * to null-terminated string containing
 *				"template" that will be searched for
 *		white (I)	char * to null-terminated string containing
 *				whitespace chars.  If NULL or empty, then
 *				" \t\n\r\f\v" (see #define WHITESPACE) used.
 *				If white contains I or i, then match is
 *				case-insensitive (and I,i _not_ considered
 *				whitespace).
 *		sublen (O)	address of int returning "length" of substr
 *				found in string (which may be longer or
 *				shorter than substr itself).
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to first char of substr in string
 *				or NULL if not found or for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	Wherever a single whitespace char appears in substr,
 *		the corresponding position in string may contain any
 *		number (including 0) of whitespace chars, e.g.,
 *		string="abc   def" and string="abcdef" both match
 *		substr="c d" at offset 2 of string.
 *	      o	If substr="c  d" (two spaces between c and d),
 *		then string must have at least one space, so now "abcdef"
 *		doesn't match.  In general, the minimum number of spaces
 *		in string is the number of spaces in substr minus 1
 *		(so 1 space in substr permits 0 spaces in string).
 *	      o	Embedded spaces are counted in sublen, e.g.,
 *		string="c   d" (three spaces) matches substr="c d"
 *		with sublen=5 returned.  But string="ab   c   d" will
 *		also match substr="  c d" returning sublen=5 and
 *		a ptr to the "c".  That is, the mandatory preceding
 *		space is _not_ counted as part of the match.
 *		But all the embedded space is counted.
 *		(An inconsistent bug/feature is that mandatory
 *		terminating space is counted.)
 *	      o	Moreover, string="c   d" matches substr="  c d", i.e.,
 *		the very beginning of a string is assumed to be preceded
 *		by "virtual blanks".
 * ======================================================================= */
/* --- entry point --- */
char	*strwstr ( char *string, char *substr, char *white, int *sublen )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
char	*psubstr=substr, *pstring=string,/*ptr to current char in substr,str*/
	*pfound = (char *)NULL;		/*ptr to found substr back to caller*/
char	*pwhite=NULL, whitespace[256];	/* callers white whithout i,I */
int	iscase = 1;			/* case-insensitive if i,I in white*/
int	foundlen = 0;			/* length of substr found in string*/
int	nstrwhite=0, nsubwhite=0,	/* #leading white chars in str,sub */
	nminwhite=0;			/* #mandatory leading white in str */
int	nstrchars=0, nsubchars=0,	/* #non-white chars to be matched */
	isncmp=0;			/*strncmp() or strncasecmp() result*/
/* -------------------------------------------------------------------------
Initialization
-------------------------------------------------------------------------- */
/* --- set up whitespace --- */
strcpy(whitespace,WHITESPACE);		/*default if no user input for white*/
if ( white != NULL )			/*user provided ptr to white string*/
 if ( *white != '\000' ) {		/*and it's not just an empty string*/
   strcpy(whitespace,white);		/* so use caller's white spaces */
   while ( (pwhite=strchr(whitespace,'i')) != NULL ) /* have an embedded i */
     { iscase = 0; strsqueeze(pwhite,1); } /*set flag and squeeze it out*/
   while ( (pwhite=strchr(whitespace,'I')) != NULL ) /* have an embedded I */
     { iscase = 0; strsqueeze(pwhite,1); } /*set flag and squeeze it out*/
   if ( *whitespace == '\000' )		/* caller's white just had i,I */
     strcpy(whitespace,WHITESPACE); }	/* so revert back to default */
/* -------------------------------------------------------------------------
Find first occurrence of substr in string
-------------------------------------------------------------------------- */
if ( string != NULL )			/* caller passed us a string ptr */
 while ( *pstring != '\000' ) {		/* break when string exhausted */
  char	*pstrptr = pstring;		/* (re)start at next char in string*/
  int	leadingwhite = 0;		/* leading whitespace */
  psubstr = substr;			/* start at beginning of substr */
  foundlen = 0;				/* reset length of found substr */
  if ( substr != NULL )			/* caller passed us a substr ptr */
   while ( *psubstr != '\000' ) {	/*see if pstring begins with substr*/
    /* --- check for end-of-string before finding match --- */
    if ( *pstrptr == '\000' )		/* end-of-string without a match */
      goto nextstrchar;			/* keep trying with next char */
    /* --- actual amount of whitespace in string and substr --- */
    nsubwhite = strspn(psubstr,whitespace); /* #leading white chars in sub */
    nstrwhite = strspn(pstrptr,whitespace); /* #leading white chars in str */
    nminwhite = max2(0,nsubwhite-1);	/* #mandatory leading white in str */
    /* --- check for mandatory leading whitespace in string --- */
    if ( pstrptr != string )		/*not mandatory at start of string*/
      if ( nstrwhite < nminwhite )	/* too little leading white space */
	goto nextstrchar;		/* keep trying with next char */
    /* ---hold on to #whitespace chars in string preceding substr match--- */
    if ( pstrptr == pstring )		/* whitespace at start of substr */
      leadingwhite = nstrwhite;		/* save it as leadingwhite */
    /* --- check for optional whitespace --- */
    if ( psubstr != substr )		/* always okay at start of substr */
      if ( nstrwhite>0 && nsubwhite<1 )	/* too much leading white space */
	goto nextstrchar;		/* keep trying with next char */
    /* --- skip any leading whitespace in substr and string --- */
    psubstr += nsubwhite;		/* push past leading sub whitespace*/
    pstrptr += nstrwhite;		/* push past leading str whitespace*/
    /* --- now get non-whitespace chars that we have to match --- */
    nsubchars = strcspn(psubstr,whitespace); /* #non-white chars in sub */
    nstrchars = strcspn(pstrptr,whitespace); /* #non-white chars in str */
    if ( nstrchars < nsubchars )	/* too few chars for match */
      goto nextstrchar;			/* keep trying with next char */
    /* --- see if next nsubchars are a match --- */
    isncmp = (iscase? strncmp(pstrptr,psubstr,nsubchars): /*case sensitive*/
		strncasecmp(pstrptr,psubstr,nsubchars)); /*case insensitive*/
    if ( isncmp != 0 )			/* no match */
      goto nextstrchar;			/* keep trying with next char */
    /* --- push past matched chars --- */
    psubstr += nsubchars;  pstrptr += nsubchars;  /*nsubchars were matched*/
    } /* --- end-of-while(*psubstr!='\000') --- */
  pfound = pstring + leadingwhite;	/* found match starting at pstring */
  foundlen = (int)(pstrptr-pfound);	/* consisting of this many chars */
  goto end_of_job;			/* back to caller */
  /* ---failed to find substr, continue trying with next char in string--- */
  nextstrchar:				/* continue outer loop */
    pstring++;				/* bump to next char in string */
  } /* --- end-of-while(*pstring!='\000') --- */
/* -------------------------------------------------------------------------
Back to caller with ptr to first occurrence of substr in string
-------------------------------------------------------------------------- */
end_of_job:
  if ( msgfp!=NULL && msglevel>=999 ) {	/* debugging/diagnostic output */
    fprintf(msgfp,"strwstr> str=\"%.72s\" sub=\"%s\" found at offset %d\n",
    string,substr,(pfound==NULL?(-1):(int)(pfound-string))); fflush(msgfp); }
  if ( sublen != NULL )			/*caller wants length of found substr*/
    *sublen = foundlen;			/* give it to him along with ptr */
  return ( pfound );			/*ptr to first found substr, or NULL*/
} /* --- end-of-function strwstr() --- */


/* ==========================================================================
 * Function:	strreplace ( string, from, to, iscase, nreplace )
 * Purpose:	Changes the first nreplace occurrences of 'from' to 'to'
 *		in string, or all occurrences if nreplace=0.
 * --------------------------------------------------------------------------
 * Arguments:	string (I/0)	char * to null-terminated string in which
 *				occurrence of 'from' will be replaced by 'to'
 *		from (I)	char * to null-terminated string
 *				to be replaced by 'to'
 *		to (I)		char * to null-terminated string that will
 *				replace 'from'
 *		iscase (I)	int containing 1 if matches of 'from'
 *				in 'string' should be case-sensitive,
 *				or 0 if matches are case-insensitive.
 *		nreplace (I)	int containing (maximum) number of
 *				replacements, or 0 to replace all.
 * --------------------------------------------------------------------------
 * Returns:	( int )		number of replacements performed,
 *				or 0 for no replacements or -1 for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	strreplace ( char *string, char *from, char *to,
	int iscase, int nreplace )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	fromlen = (from==NULL?0:strlen(from)), /* #chars to be replaced */
	tolen = (to==NULL?0:strlen(to)); /* #chars in replacement string */
int	iscommand = (fromlen<2?0:(*from=='\\'?1:0)); /*is from a \command ?*/
char	*pfrom = (char *)NULL,		/*ptr to 1st char of from in string*/
	*pstring = string,		/*ptr past previously replaced from*/
	*strchange();			/* change 'from' to 'to' */
/*int	iscase = 1;*/			/* true for case-sensitive match */
int	nreps = 0;			/* #replacements returned to caller*/
/* -------------------------------------------------------------------------
repace occurrences of 'from' in string to 'to'
-------------------------------------------------------------------------- */
if ( string == (char *)NULL		/* no input string */
||   (fromlen<1 && nreplace<=0) )	/* replacing empty string forever */
  nreps = (-1);				/* so signal error */
else					/* args okay */
  while (nreplace<1 || nreps<nreplace) { /* up to #replacements requested */
    if ( fromlen > 0 )			/* have 'from' string */
      pfrom =				/*ptr to 1st char of from in string*/
       (iscase>0? strstr(pstring,from):	/* case-sensistive match */
        strcasestr(pstring,from));	/* case-insensistive match */
    else  pfrom = pstring;		/*or empty from at start of string*/
    if ( pfrom == (char *)NULL ) break;	/*no more from's, so back to caller*/
    if ( iscommand )			/* ignore prefix of longer string */
      if ( isalpha((int)(*(pfrom+fromlen))) ) {	/* just a longer string */
        pstring = pfrom+fromlen;	/* pick up search after 'from' */
        continue; }			/* don't change anything */
    if ( iscase > 1 ) { ; }		/* ignore \escaped matches */
    if ( strchange(fromlen,pfrom,to)	/* leading 'from' changed to 'to' */
    ==   (char *)NULL ) { nreps=(-1); break; } /* signal error to caller */
    nreps++;				/* count another replacement */
    pstring = pfrom+tolen;		/* pick up search after 'to' */
    if ( *pstring == '\000' ) break;	/* but quit at end of string */
    } /* --- end-of-while() --- */
return ( nreps );			/* #replacements back to caller */
} /* --- end-of-function strreplace() --- */


/* ==========================================================================
 * Function:	strchange ( nfirst, from, to )
 * Purpose:	Changes the nfirst leading chars of `from` to `to`.
 *		For example, to change char x[99]="12345678" to "123ABC5678"
 *		call strchange(1,x+3,"ABC")
 * --------------------------------------------------------------------------
 * Arguments:	nfirst (I)	int containing #leading chars of `from`
 *				that will be replace by `to`
 *		from (I/O)	char * to null-terminated string whose nfirst
 *				leading chars will be replaced by `to`
 *		to (I)		char * to null-terminated string that will
 *				replace the nfirst leading chars of `from`
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to first char of input `from`
 *				or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	If strlen(to)>nfirst, from must have memory past its null
 *		(i.e., we don't do a realloc)
 * ======================================================================= */
/* --- entry point --- */
char	*strchange ( int nfirst, char *from, char *to )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	tolen = (to==NULL?0:strlen(to)), /* #chars in replacement string */
	nshift = abs(tolen-nfirst);	/*need to shift from left or right*/
if ( from == NULL ) goto end_of_job;	/* error if no source string */
/* -------------------------------------------------------------------------
shift from left or right to accommodate replacement of its nfirst chars by to
-------------------------------------------------------------------------- */
if ( tolen < nfirst )			/* shift left is easy */
    {strsqueeze(from,nshift);}		/* squeeze out extra bytes */
if ( tolen > nfirst )			/* need more room at start of from */
  { char *pfrom = from+strlen(from);	/* ptr to null terminating from */
    for ( ; pfrom>=from; pfrom-- )	/* shift all chars including null */
      *(pfrom+nshift) = *pfrom; }	/* shift chars nshift places right */
/* -------------------------------------------------------------------------
from has exactly the right number of free leading chars, so just put to there
-------------------------------------------------------------------------- */
if ( tolen != 0 )			/* make sure to not empty or null */
  memcpy(from,to,tolen);		/* chars moved into place */
end_of_job: return ( from );		/* changed string back to caller */
} /* --- end-of-function strchange() --- */


/* ==========================================================================
 * Function:	isstrstr ( char *string, char *snippets, int iscase )
 * Purpose:	determine whether any substring of 'string'
 *		matches any of the comma-separated list of 'snippets',
 *		ignoring case if iscase=0.
 * --------------------------------------------------------------------------
 * Arguments:	string (I)	char * containing null-terminated
 *				string that will be searched for
 *				any one of the specified snippets
 *		snippets (I)	char * containing null-terminated,
 *				comma-separated list of snippets
 *				to be searched for in string
 *		iscase (I)	int containing 0 for case-insensitive
 *				comparisons, or 1 for case-sensitive
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 if any snippet is a substring of
 *				string, 0 if not
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	isstrstr ( char *string, char *snippets, int iscase )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	status = 0;			/*1 if any snippet found in string*/
char	snip[256], *snipptr = snippets,	/* munge through each snippet */
	delim = ',', *delimptr = NULL,	/* separated by delim's */
	*strstrptr = NULL;		/* looking for a match in string */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- arg check --- */
if ( isempty(string)			/*both string and snippets required*/
||   isempty(snippets) ) goto end_of_job; /* quit if either missing */
/* -------------------------------------------------------------------------
extract each snippet and see if it's a substring of string
-------------------------------------------------------------------------- */
while ( snipptr != NULL ) {		/* while we still have snippets */
  /* --- extract next snippet --- */
  if ( (delimptr = strchr(snipptr,delim)) /* locate next comma delim */
  ==   NULL ) {				/*not found following last snippet*/
    strcpy(snip,snipptr);		/* local copy of last snippet */
    snipptr = NULL; }			/* signal end-of-string */
  else {				/* snippet ends just before delim */
    int sniplen = (int)(delimptr-snipptr) - 1;  /* #chars in snippet */
    memcpy(snip,snipptr,sniplen);	/* local copy of snippet chars */
    snip[sniplen] = '\000';		/* null-terminated snippet */
    snipptr = delimptr + 1; }		/* next snippet starts after delim */
  /* --- check if snippet in string --- */
  strstrptr = (iscase? strstr(string,snip) : strcasestr(string,snip) );
  if ( strstrptr != NULL ) {		/* found snippet in string */
    status = 1;				/* so reset return status */
    break; }				/* no need to check any further */
  } /* --- end-of-while(*snipptr!=0) --- */
end_of_job: return ( status );		/*1 if snippet found in list, else 0*/
} /* --- end-of-function isstrstr() --- */


/* ==========================================================================
 * Function:	nomath ( s )
 * Purpose:	Removes/replaces any LaTeX math chars in s
 *		so that s can be rendered in paragraph mode.
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		char * to null-terminated string
 *				whose math chars are to be removed/replaced
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to "cleaned" copy of s
 *				or "" (empty string) for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	The returned pointer addresses a static buffer,
 *		so don't call nomath() again until you're finished
 *		with output from the preceding call.
 * ======================================================================= */
/* --- entry point --- */
char	*nomath ( char *s )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char sbuff[4096];		/* copy of s with no math chars */
int	strreplace();			/* replace _ with -, etc */
/* -------------------------------------------------------------------------
Make a clean copy of s
-------------------------------------------------------------------------- */
/* --- check input --- */
*sbuff = '\000';			/* initialize in case of error */
if ( isempty(s) ) goto end_of_job;	/* no input */
/* --- start with copy of s --- */
strninit(sbuff,s,3000);			/* leave room for replacements */
/* --- make some replacements (*must* replace \ first) --- */
strreplace(sbuff,"\\","\\textbackslash ",0,0); /* change all \'s to text */
strreplace(sbuff,"_","\\textunderscore ",0,0); /* change all _'s to text */
strreplace(sbuff,"<","\\textlangle ",0,0); /* change all <'s to text */
strreplace(sbuff,">","\\textrangle ",0,0); /* change all >'s to text */
strreplace(sbuff,"$","\\textdollar ",0,0); /* change all $'s to text */
strreplace(sbuff,"&","\\&",0,0);           /* change every & to \& */
strreplace(sbuff,"%","\\%",0,0);           /* change every % to \% */
strreplace(sbuff,"#","\\#",0,0);           /* change every # to \# */
strreplace(sbuff,"~","\\~",0,0);           /* change every ~ to \~ */
strreplace(sbuff,"{","\\{",0,0);           /* change every { to \{ */
strreplace(sbuff,"}","\\}",0,0);           /* change every } to \} */
strreplace(sbuff,"^","\\ensuremath{\\widehat{~}}",0,0); /* change every ^ */
end_of_job:
  return ( sbuff );			/* back with clean copy of s */
} /* --- end-of-function nomath() --- */


/* ==========================================================================
 * Function:	strwrap ( s, linelen, tablen )
 * Purpose:	Inserts \n's and spaces in (a copy of) s to wrap lines
 *		at linelen and indent them by tablen.
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		char * to null-terminated string
 *				to be wrapped.
 *		linelen (I)	int containing maximum linelen
 *				between \n's.
 *		tablen (I)	int containing number of spaces to indent
 *				lines.  0=no indent.  Positive means
 *				only indent first line and not others.
 *				Negative means indent all lines except first.
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to "line-wrapped" copy of s
 *				or "" (empty string) for any error.
 * --------------------------------------------------------------------------
 * Notes:     o	The returned copy of s has embedded \n's as necessary
 *		to wrap lines at linelen.  Any \n's in the input copy
 *		are removed first.  If (and only if) the input s contains
 *		a terminating \n then so does the returned copy.
 *	      o	The returned pointer addresses a static buffer,
 *		so don't call strwrap() again until you're finished
 *		with output from the preceding call.
 * ======================================================================= */
/* --- entry point --- */
char	*strwrap ( char *s, int linelen, int tablen )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char sbuff[4096];		/* line-wrapped copy of s */
char	*sol = sbuff;			/* ptr to start of current line*/
char	tab[32] = "                 ";	/* tab string */
int	strreplace();			/* remove \n's */
char	*strchange();			/* add \n's and indent space */
int	finalnewline = (lastchar(s)=='\n'?1:0); /*newline at end of string?*/
int	istab = (tablen>0?1:0);		/* init true to indent first line */
int	rhslen  = 0,			/* remaining right hand side length*/
	thislen = 0,			/* length of current line segment */
	thistab = 0,			/* length of tab on current line */
	wordlen = 0;			/* length to next whitespace char */
/* -------------------------------------------------------------------------
Make a clean copy of s
-------------------------------------------------------------------------- */
/* --- check input --- */
*sbuff = '\000';			/* initialize in case of error */
if ( isempty(s) ) goto end_of_job;	/* no input */
if ( tablen < 0 ) tablen = (-tablen);	/* set positive tablen */
if ( tablen >= linelen ) tablen = linelen-1; /* tab was longer than line */
tab[min2(tablen,16)] = '\000';		/* null-terminate tab string */
tablen = strlen(tab);			/* reset to actual tab length */
/* --- start with copy of s --- */
strninit(sbuff,s,3000);			/* leave room for \n's and tabs */
if ( linelen < 1 ) goto end_of_job;	/* can't do anything */
trimwhite(sbuff);			/*remove leading/trailing whitespace*/
strreplace(sbuff,"\n"," ",0,0);		/* remove any original \n's */
strreplace(sbuff,"\r"," ",0,0);		/* remove any original \r's */
strreplace(sbuff,"\t"," ",0,0);		/* remove any original \t's */
/* -------------------------------------------------------------------------
Insert \n's and spaces as needed
-------------------------------------------------------------------------- */
while ( 1 ) {				/* till end-of-line */
  /* --- init --- */
  trimwhite(sol);			/*remove leading/trailing whitespace*/
  thislen = thistab = 0;		/* no chars in current line yet */
  if ( istab && tablen>0 ) {		/* need to indent this line */
    strchange(0,sol,tab);		/* insert indent at start of line */
    thistab = tablen; }			/* line starts with whitespace tab */
  if ( sol == sbuff ) istab = 1-istab;	/* flip tab flag after first line */
  sol += thistab;			/* skip tab */
  rhslen = strlen(sol);			/* remaining right hand side chars */
  if ( rhslen <= linelen ) break;	/* no more \n's needed */
  if ( 0 && msglevel >= 99 ) {
    fprintf(stdout,"strwrap> rhslen=%d, sol=\"\"%s\"\"\n",rhslen,sol);
    fflush(stdout); }
  /* --- look for last whitespace preceding linelen --- */
  while ( 1 ) {				/* till we exceed linelen */
    wordlen = strcspn(sol+thislen,WHITESPACE); /*ptr to next whitespace char*/
    if ( thislen+thistab+wordlen >= linelen ) break; /*next word won't fit*/
    thislen += (wordlen+1); }		/* ptr past next whitespace char */
  if ( thislen < 1 ) break;		/* line will have one too-long word*/
  sol[thislen-1] = '\n';		/* replace last space with newline */
  sol += thislen;			/* next line starts after newline */
  } /* --- end-of-while(1) --- */
end_of_job:
  if ( finalnewline ) strcat(sbuff,"\n"); /* replace final newline */
  return ( sbuff );			/* back with clean copy of s */
} /* --- end-of-function strwrap() --- */


/* ==========================================================================
 * Function:	strpspn ( char *s, char *reject, char *segment )
 * Purpose:	finds the initial segment of s containing no chars
 *		in reject that are outside (), [] and {} parens, e.g.,
 *		   strpspn("abc(---)def+++","+-",segment) returns
 *		   segment="abc(---)def" and a pointer to the first '+' in s
 *		because the -'s are enclosed in () parens.
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		(char *)pointer to null-terminated string
 *				whose initial segment is desired
 *		reject (I)	(char *)pointer to null-terminated string
 *				containing the "reject chars"
 *				If reject contains a " or a ', then the
 *				" or ' isn't itself a reject char,
 *				but other reject chars within quoted
 *				strings (or substrings of s) are spanned.
 *		segment (O)	(char *)pointer returning null-terminated
 *				string comprising the initial segment of s
 *				that contains non-rejected chars outside
 *				(),[],{} parens, i.e., all the chars up to
 *				but not including the returned pointer.
 *				(That's the entire string if no non-rejected
 *				chars are found.)
 * --------------------------------------------------------------------------
 * Returns:	( char * )	pointer to first reject-char found in s
 *				outside parens, or a pointer to the
 *				terminating '\000' of s if there are
 *				no reject chars in s outside all () parens.
 *				But if reject is empty, returns pointer
 *				to matching )]} outside all parens.
 * --------------------------------------------------------------------------
 * Notes:     o	the return value is _not_ like strcspn()'s
 *	      o	improperly nested (...[...)...] are not detected,
 *		but are considered "balanced" after the ]
 *	      o	if reject not found, segment returns the entire string s
 *	      o	but if reject is empty, returns segment up to and including
 *		matching )]}
 *	      o	leading/trailing whitespace is trimmed from returned segment
 * ======================================================================= */
/* --- entry point --- */
char	*strpspn ( char *s, char *reject, char *segment )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
char	*ps = s;			/* current pointer into s */
char	*strqspn(char *s,char *q,int isunescape); /*span quoted string*/
char	qreject[256]="\000", *pq=qreject, *pr=reject; /*find "or' in reject*/
int	isqspan = 0;			/* true to span quoted strings */
int	depth = 0;			/* () paren nesting level */
int	seglen=0, maxseg=2047;		/* segment length, max allowed */
int	isescaped=0, checkescapes=1;	/* signals escaped chars */
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- check arguments --- */
if ( isempty(s)				/* no input string supplied */
/*||   isempty(reject)*/ ) goto end_of_job; /* no reject chars supplied */
/* --- set up qreject w/o quotes --- */
if ( !isempty(reject) )			/* have reject string from caller */
  while ( *pr != '\000' ) {		/* until end-of-reject string */
    if ( !isthischar(*pr,"\"\'") )	/* not a " or ' */
      *pq++ = *pr;			/* copy actual reject char */
    else isqspan = 1;			/* span rejects in quoted strings */
    pr++; }				/* next reject char from caller */
*pq = '\000';				/* null-terminate qreject */
/* -------------------------------------------------------------------------
find first char from s outside () parens (and outside ""'s) and in reject
-------------------------------------------------------------------------- */
while ( *ps != '\000' ) {		/* search till end of input string */
  int spanlen = 1;			/*span 1 non-reject, non-quoted char*/
  if ( !isescaped ) {			/* ignore escaped \(,\[,\{,\),\],\}*/
    if ( isthischar(*ps,"([{") ) depth++;   /* push another paren */
    if ( isthischar(*ps,")]}") ) depth--; } /* or pop another paren */
  if ( depth < 1 ) {			/* we're outside all parens */
    if ( isqspan )			/* span rejects in quoted strings */
      if ( isthischar(*ps,"\"\'") ) {	/* and we're at opening quote */
        pq = strqspn(ps,NULL,0);	/* locate matching closing quote */
        if ( pq != ps )			/* detected start of quoted string */
         if ( *pq == *ps )		/* and found closing quote */
          spanlen = ((int)(pq-ps)) + 1; } /* span the entire quoted string */
    if ( isempty(qreject) ) break;	/* no reject so break immediately */
    if ( isthischar(*ps,qreject) ) break; } /* only break on a reject char */
  if ( checkescapes )			/* if checking escape sequences */
    isescaped = (*ps=='\\'?1:0);	/* reset isescaped signal */
  if ( segment != NULL ) {		/* caller gave us segment */
    int copylen = min2(spanlen,maxseg-seglen); /* don't overflow segment */
    if ( copylen > 0 )			/* have room in segment buffer */
      memcpy(segment+seglen,ps,copylen); } /* so copy non-reject chars */
  seglen += spanlen;  ps += spanlen;	/* bump to next char */
  } /* --- end-of-while(*ps!=0) --- */
end_of_job:
  if ( segment != NULL ) {		/* caller gave us segment */
    if ( isempty(qreject) && !isempty(s) ) { /* no reject char */
      segment[min2(seglen,maxseg)] = *ps;  seglen++; } /*closing )]} to seg*/
    segment[min2(seglen,maxseg)] = '\000'; /* null-terminate the segment */
    trimwhite(segment); }		/* trim leading/trailing whitespace*/
  return ( ps );			/* back to caller */
} /* --- end-of-function strpspn() --- */


/* ==========================================================================
 * Function:	strqspn ( char *s, char *q, int isunescape )
 * Purpose:	finds matching/closing " or ' in quoted string
 *		that begins with " or ', and optionally changes
 *		escaped quotes to unescaped quotes.
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		(char *)pointer to null-terminated string
 *				that begins with " or ',
 *		q (O)		(char *)pointer returning null-terminated
 *				quoted token, with or without outer quotes,
 *				and with or without escaped inner quotes
 *				changed to unescaped quotes, depending
 *				on isunescape.
 *		isunescape (I)	int containing 1 to change \" to " if s
 *				is "quoted" or change \' to ' if 'quoted',
 *				or containing 2 to change both \" and \'
 *				to unescaped quotes.  Other \sequences aren't
 *				changed.  Note that \\" emits \".
 *				isunescape=0 makes no changes at all.
 *				Note: the following not implemented yet --
 *				If unescape is negative, its abs() is used,
 *				but outer quotes aren't included in q.
 * --------------------------------------------------------------------------
 * Returns:	( char * )	pointer to matching/closing " or '
 *				(or to char after quote if isunescape<0),
 *				or terminating '\000' if none found,
 *				or unchanged (same as s) if not quoted string
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*strqspn ( char *s, char *q, int isunescape )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
char	*ps = s,  *pq = q;		/* current pointer into s, q */
char	quote = '\000';			/* " or ' quote character */
int	qcopy = (isunescape<0?0:1);	/* true to copy outer quotes */
int	isescaped = 0;			/* true to signal \escape sequence */
int	maxqlen = 2400;			/* max length of returned q */
/* -------------------------------------------------------------------------
Initialization
-------------------------------------------------------------------------- */
/* --- check args --- */
if ( s == NULL ) goto end_of_job;	/* no string supplied */
skipwhite(ps);				/* skip leading whitespace */
if ( *ps == '\000'			/* string exhausted */
||   (!isthischar(*ps,"\"\'")) ) {	/* or not a " or ' quoted string */
  ps = s;  goto end_of_job; }		/*signal error/not string to caller*/
if ( isunescape < 0 ) isunescape = (-isunescape); /* flip positive */
/* --- set quote character --- */
quote = *ps;				/* set quote character */
if ( qcopy && q!=NULL ) *pq++ = quote;	/* and copy it to output token */
/* -------------------------------------------------------------------------
span characters between quotes
-------------------------------------------------------------------------- */
while ( *(++ps) != '\000' ) {		/* end-of-string always terminates */
  /* --- process escaped chars --- */
  if ( isescaped ) {			/* preceding char was \ */
    if ( *ps != '\\' ) isescaped = 0;	/* reset isescaped flag unless \\ */
    if ( q != NULL ) {			/* caller wants quoted token */
      if ( isunescape==0		/* don't unescape anything */
      ||   (isunescape==1 && *ps!=quote) /* escaped char not our quote */
      ||   (isunescape==2 && (!isthischar(*ps,"\"\'"))) ) /* not any quote */
        if ( --maxqlen > 0 )		/* so if there's room in token */
          *pq++ = '\\';			/*keep original \ in returned token*/
      if ( !isescaped )			/* will have to check 2nd \ in \\ */
        if ( --maxqlen > 0 )		/* if there's room in token */
          *pq++ = *ps; }		/* put escaped char in token */
    continue; }				/* go on to next char in string */
  /* --- check if next char escaped --- */
  if ( *ps == '\\' ) {			/* found escape char */
    isescaped=1; continue; }		/*set flag and process escaped char*/
  /* --- check for unescaped closing quote --- */
  if ( *ps == quote ) {			/* got an unescaped quote */
    if ( qcopy && q!=NULL ) *pq++ = quote; /* copy it to output token */
    if ( 0 && !qcopy ) ps++;		/* return ptr to char after quote */
    goto end_of_job; }			/* back to caller */
  /* --- process other chars --- */
  if ( q != NULL )			/* caller want token returned */
    if ( --maxqlen > 0 )		/* and there's still room in token */
      *pq++ = *ps;			/* put char in  token */
  } /* --- end-of-while(*(++ps)!='\000') --- */
/*ps = NULL;*/  /*pq = q;*/		/* error if no closing quote found */
end_of_job:
  if ( q != NULL ) *pq = '\000';	/* null-terminate returned token */
  return ( ps );			/* return ptr to " or ', or NULL */
} /* --- end-of-function strqspn() --- */


/* ==========================================================================
 * Function:	isnumeric ( s )
 * Purpose:	determine if s is an integer
 * --------------------------------------------------------------------------
 * Arguments:	s (I)		(char *)pointer to null-terminated string
 *				that's checked for a leading + or -
 *				followed by digits
 * --------------------------------------------------------------------------
 * Returns:	( int )		1 if s is numeric, 0 if it is not
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	isnumeric ( char *s )
{
/* -------------------------------------------------------------------------
determine whether s is an integer
-------------------------------------------------------------------------- */
int	status = 0;			/* return 0 if not numeric, 1 if is*/
char	*p = s;				/* pointer into s */
if ( isempty(s) ) goto end_of_job;	/* missing arg or empty string */
skipwhite(p);				/*check for leading +or- after space*/
if ( *p=='+' || *p=='-' ) p++;		/* skip leading + or - */
for ( ; *p != '\000'; p++ ) {		/* check rest of s for digits */
  if ( isdigit(*p) ) continue;		/* still got uninterrupted digits */
  if ( !isthischar(*p,WHITESPACE) ) goto end_of_job; /* non-numeric char */
  skipwhite(p);				/* skip all subsequent whitespace */
  if ( *p == '\000' ) break;		/* trailing whitespace okay */
  goto end_of_job;			/* embedded whitespace non-numeric */
  } /* --- end-of-for(*p) --- */
status = 1;				/* numeric after checks succeeded */
end_of_job:
  return ( status );			/*back to caller with 1=string, 0=no*/
} /* --- end-of-function isnumeric() --- */


/* ==========================================================================
 * Function:	evalterm ( STORE *store, char *term )
 * Purpose:	evaluates a term
 * --------------------------------------------------------------------------
 * Arguments:	store (I/O)	STORE * containing environment
 *				in which term is to be evaluated
 *		term (I)	char * containing null-terminated string
 *				with a term like "3" or "a" or "a+3"
 *				whose value is to be determined
 * --------------------------------------------------------------------------
 * Returns:	( int )		value of term,
 *				or NOVALUE for any error
 * --------------------------------------------------------------------------
 * Notes:     o	Also evaluates index?a:b:c:etc, returning a if index<=0,
 *		b if index=1, etc, and the last value if index is too large.
 *		Each a:b:c:etc can be another expression, including another
 *		(index?a:b:c:etc) which must be enclosed in parentheses.
 * ======================================================================= */
/* --- entry point --- */
int	evalterm ( STORE *store, char *term )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	termval = 0;			/* term value returned to caller */
char	token[2048] = "\000",		/* copy term */
	*delim = NULL;			/* delim '(' or '?' in token */
/*int	evalwff(),*/			/* recurse to evaluate terms */
/*	evalfunc();*/			/* evaluate function(arg1,arg2,...)*/
char	*strpspn();			/* span delims */
int	getstore();			/* lookup variables */
int	isnumeric();			/* numeric=constant, else variable */
static	int evaltermdepth = 0;		/* recursion depth */
int	novalue = (-89123456);		/* dummy (for now) error signal */
/* -------------------------------------------------------------------------
Initialization
-------------------------------------------------------------------------- */
if ( ++evaltermdepth > 99 ) goto end_of_job; /*probably recursing forever*/
if ( store==NULL || isempty(term) ) goto end_of_job; /*check for missing arg*/
skipwhite(term);			/* skip any leading whitespace */
/* -------------------------------------------------------------------------
First look for conditional of the form term?term:term:...
-------------------------------------------------------------------------- */
/* ---left-hand part of conditional is chars preceding "?" outside ()'s--- */
delim = strpspn(term,"?",token);	/* chars preceding ? outside () */
if ( *delim != '\000' ) {		/* found conditional expression */
  int ncolons = 0;			/* #colons we've found so far */
  if ( *token != '\000' )		/* evaluate "index" value on left */
    if ( (termval=evalterm(store,token)) /* evaluate left-hand term */
    == novalue ) goto end_of_job;	/* return error if failed */
  while ( *delim != '\000' ) {		/* still have chars in term */
    delim++; *token='\000';		/* initialize for next "value:" */
    if ( *delim == '\000' ) break;	/* no more values */
    delim = strpspn(delim,":",token);	/* chars preceding : outside () */
    if ( ncolons++ >= termval ) break;	/* have corresponding term */
    } /* --- end-of-while(*delim!='\000')) --- */
  if ( *token != '\000' )		/* have x:x:value:x:x on right */
    termval=evalterm(store,token);	/* so evaluate it */
  goto end_of_job;			/* return result to caller */
  } /* --- end-of-if(*delim!='\000')) --- */
/* -------------------------------------------------------------------------
evaluate a+b recursively
-------------------------------------------------------------------------- */
/* --- left-hand part of term is chars preceding "/+-*%" outside ()'s --- */
term = strpspn(term,"/+-*%",token);	/* chars preceding /+-*% outside ()*/
/* --- evaluate a+b, a-b, etc --- */
if ( *term != '\000' ) {		/* found arithmetic operation */
  int leftval=0, rightval=0;		/* init leftval for unary +a or -a */
  if ( *token != '\000' )		/* or eval for binary a+b or a-b */
    if ( (leftval=evalterm(store,token)) /* evaluate left-hand term */
    == novalue ) goto end_of_job;	/* return error if failed */
  if ( (rightval=evalterm(store,term+1)) /* evaluate right-hand term */
  == novalue ) goto end_of_job;		/* return error if failed */
  switch ( *term ) {			/* perform requested arithmetic */
    default: break;			/* internal error */
    case '+': termval = leftval+rightval;  break;  /* addition */
    case '-': termval = leftval-rightval;  break;  /* subtraction */
    case '*': termval = leftval*rightval;  break;  /* multiplication */
    case '/': if ( rightval != 0 )	/* guard against divide by zero */
                termval = leftval/rightval;  break; /* integer division */
    case '%': if ( rightval != 0 )	/* guard against divide by zero */
                termval = leftval%rightval;  break; /*left modulo right */
    } /* --- end-of-switch(*relation) --- */
  goto end_of_job;			/* return result to caller */
  } /* --- end-of-if(*term!='\000')) --- */
/* -------------------------------------------------------------------------
check for parenthesized expression or term of the form function(arg1,arg2,...)
-------------------------------------------------------------------------- */
if ( (delim = strchr(token,'(')) != NULL ) { /* token contains a ( */
  /* --- strip trailing paren (if there hopefully is one) --- */
  int  toklen = strlen(token);		/* total #chars in token */
  if ( token[toklen-1] == ')' )		/* found matching ) at end of token*/
    token[--toklen] = '\000';		/* remove trailing ) */
  /* --- handle parenthesized subexpression --- */
  if ( *token == '(' ) {		/* have parenthesized expression */
    strsqueeze(token,1);		/* so squeeze out leading ( */
    /* --- evaluate edited term --- */
    trimwhite(token);			/* trim leading/trailing whitespace*/
    termval = evalterm(store,token); }	/* evaluate token recursively */
  /* --- handle function(arg1,arg2,...) --- */
  else {				/* have function(arg1,arg2,...) */
    *delim = '\000';			/* separate function name and args */
    /*termval = evalfunc(store,token,delim+1);*/ } /* evaluate function */
  goto end_of_job; }			/* return result to caller */
/* -------------------------------------------------------------------------
evaluate constants directly, or recursively evaluate variables, etc
-------------------------------------------------------------------------- */
if ( *token != '\000' ) {		/* empty string */
  if ( isnumeric(token) )		/* have a constant */
    termval = atoi(token);		/* convert ascii-to-int */
  else {				/* variable or "stored proposition"*/
    termval = getstore(store,token); }	/* look up token */
  } /* --- end-of-if(*token!=0) --- */
/* -------------------------------------------------------------------------
back to caller with truth value of proposition
-------------------------------------------------------------------------- */
end_of_job:
  /* --- back to caller --- */
  if ( evaltermdepth > 0 ) evaltermdepth--;  /* pop recursion depth */
  return ( termval );			/* back to caller with value */
} /* --- end-of-function evalterm() --- */


/* ==========================================================================
 * Function:	getstore ( store, identifier )
 * Purpose:	finds identifier in store and returns corresponding value
 * --------------------------------------------------------------------------
 * Arguments:	store (I)	(STORE *)pointer to store containing
 *				the desired identifier
 *		identifier (I)	(char *)pointer to null-terminated string
 *				containing the identifier whose value
 *				is to be returned
 * --------------------------------------------------------------------------
 * Returns:	( int )		identifier's corresponding value,
 *				or 0 if identifier not found (or any error)
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	getstore ( STORE *store, char *identifier )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	value = 0;		/* store[istore].value for identifier */
int	istore=0;		/* store[] index containing identifier */
char	seek[512], hide[512];	/* identifier arg, identifier in store */
/* --- first check args --- */
if ( store==NULL || isempty(identifier)) goto end_of_job; /* missing arg */
strninit(seek,identifier,500);	/* local copy of caller's identifier */
trimwhite(seek);		/* remove leading/trailing whitespace */
/* --- loop over store --- */
for ( istore=0; istore<MAXSTORE; istore++ ) { /* until end-of-table */
  char *idstore = store[istore].identifier; /* ptr to identifier in store */
  if ( isempty(idstore) )	/* empty id signals eot */
    break;			/* de-reference any default/error value */
  strninit(hide,idstore,500);	/* local copy of store[] identifier */
  trimwhite(hide);		/* remove leading/trailing whitespace */
  if ( !strcmp(hide,seek) )	/* found match */
    break;			/* de-reference corresponding value */
  } /* --- end-of-for(istore) --- */
if ( store[istore].value != NULL ) /* address of int supplied */
  value = *(store[istore].value);  /* return de-referenced int */
end_of_job:
  return ( value );			/* store->values[istore] or NULL */
} /* --- end-of-function getstore() --- */


/* ==========================================================================
 * Function:	timestamp ( tzdelta, ifmt )
 * Purpose:	returns null-terminated character string containing
 *		current date:time stamp as ccyy-mm-dd:hh:mm:ss{am,pm}
 * --------------------------------------------------------------------------
 * Arguments:	tzdelta (I)	integer, positive or negative, containing
 *				containing number of hours to be added or
 *				subtracted from system time (to accommodate
 *				your desired time zone).
 *		ifmt (I)	integer containing 0 for default format
 * --------------------------------------------------------------------------
 * Returns:	( char * )	ptr to null-terminated buffer
 *				containing current date:time stamp
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*timestamp( int tzdelta, int ifmt )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static	char timebuff[256];		/* date:time buffer back to caller */
/*long	time_val = 0L;*/		/* binary value returned by time() */
time_t	time_val = (time_t)(0);		/* binary value returned by time() */
struct tm *tmstruct=(struct tm *)NULL, *localtime(); /* interpret time_val */
int	year=0, hour=0,ispm=1,		/* adjust year, and set am/pm hour */
	month=0, day=0,			/* adjust day and month for delta  */
	minute=0,second=0;		/* minute and second not adjusted  */
int	tzadjust();			/* time zone adjustment function */
int	daynumber();			/* #days since Jan 1, 1973 */
static	char *daynames[] = { "Monday", "Tuesday", "Wednesday",
	 "Thursday", "Friday", "Saturday", "Sunday" } ;
static	char *monthnames[] = { "?", "January", "February", "March", "April",
	 "May", "June", "July", "August", "September", "October",
	"November", "December", "?" } ;
/* -------------------------------------------------------------------------
get current date:time, adjust values, and and format stamp
-------------------------------------------------------------------------- */
/* --- first init returned timebuff in case of any error --- */
*timebuff = '\000';
/* --- get current date:time --- */
time((time_t *)(&time_val));		/* get date and time */
tmstruct = localtime((time_t *)(&time_val)); /* interpret time_val */
/* --- extract fields --- */
year  = (int)(tmstruct->tm_year);	/* local copy of year,  0=1900 */
month = (int)(tmstruct->tm_mon) + 1;	/* local copy of month, 1-12 */
day   = (int)(tmstruct->tm_mday);	/* local copy of day,   1-31 */
hour  = (int)(tmstruct->tm_hour);	/* local copy of hour,  0-23 */
minute= (int)(tmstruct->tm_min);	/* local copy of minute,0-59 */
second= (int)(tmstruct->tm_sec);	/* local copy of second,0-59 */
/* --- adjust year --- */
year += 1900;				/* set century in year */
/* --- adjust for timezone --- */
tzadjust(tzdelta,&year,&month,&day,&hour);
/* --- check params --- */
if ( hour<0  || hour>23
||   day<1   || day>31
||   month<1 || month>12
||   year<1973 ) goto end_of_job;
/* --- adjust hour for am/pm --- */
switch ( ifmt )
  {
  default:
  case 0:
    if ( hour < 12 )			/* am check */
     { ispm=0;				/* reset pm flag */
       if ( hour == 0 ) hour = 12; }	/* set 00hrs = 12am */
    if ( hour > 12 ) hour -= 12;	/* pm check sets 13hrs to 1pm, etc */
    break;
  case 4: break;			/* numeric result */
  } /* --- end-of-switch(ifmt) --- */
/* --- format date:time stamp --- */
switch ( ifmt )
  {
  default:
  case 0:  /* --- 2005-03-05:11:49:59am --- */
    sprintf(timebuff,"%04d-%02d-%02d:%02d:%02d:%02d%s",
    year,month,day,hour,minute,second,((ispm)?"pm":"am"));
    break;
  case 1:  /* --- Saturday, March 5, 2005 --- */
    sprintf(timebuff,"%s, %s %d, %d",
    daynames[daynumber(year,month,day)%7],monthnames[month],day,year);
    break;
  case 2: /* --- Saturday, March 5, 2005, 11:49:59am --- */
    sprintf(timebuff,"%s, %s %d, %d, %d:%02d:%02d%s",
    daynames[daynumber(year,month,day)%7],monthnames[month],day,year,
    hour,minute,second,((ispm)?"pm":"am"));
    break;
  case 3: /* --- 11:49:59am --- */
    sprintf(timebuff,"%d:%02d:%02d%s",
    hour,minute,second,((ispm)?"pm":"am"));
    break;
  case 4: /* --- 1231235959 (mmddhhmmss time as integer) --- */
    sprintf(timebuff,"%d%02d%02d%02d%02d",
    month,day,hour,minute,second);
    break;
  case 5:  /* --- 2005 --- */
    sprintf(timebuff,"%d",year);
    break;
  } /* --- end-of-switch(ifmt) --- */
end_of_job:
  return ( timebuff );			/* return stamp to caller */
} /* --- end-of-function timestamp() --- */


/* ==========================================================================
 * Function:	tzadjust ( tzdelta, year, month, day, hour )
 * Purpose:	Adjusts hour, and day,month,year if necessary,
 *		by delta increment to accommodate your time zone.
 * --------------------------------------------------------------------------
 * Arguments:	tzdelta (I)	integer, positive or negative, containing
 *				containing number of hours to be added or
 *				subtracted from given time (to accommodate
 *				your desired time zone).
 *		year (I)	addr of int containing        4-digit year
 *		month (I)	addr of int containing month  1=Jan - 12=Dec.
 *		day (I)		addr of int containing day    1-31 for Jan.
 *		hour (I)	addr of int containing hour   0-23
 * Returns:	( int )		1 for success, or 0 for error
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	tzadjust ( int tzdelta, int *year, int *month, int *day, int *hour )
{
/* --------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	yy = *year, mm = *month, dd = *day, hh = *hour; /*dereference args*/
/* --- calendar data --- */
static	int modays[] =
	{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };
/* --------------------------------------------------------------------------
check args
-------------------------------------------------------------------------- */
if ( mm<1 || mm>12 ) return(-1);	/* bad month */
if ( dd<1 || dd>modays[mm] ) return(-1); /* bad day */
if ( hh<0 || hh>23 ) return(-1);	/* bad hour */
if ( tzdelta>23 || tzdelta<(-23) ) return(-1); /* bad tzdelta */
/* --------------------------------------------------------------------------
make adjustments
-------------------------------------------------------------------------- */
/* --- adjust hour --- */
hh += tzdelta;				/* apply caller's delta */
/* --- adjust for feb 29 --- */
modays[2] = (yy%4==0?29:28);		/* Feb has 29 days in leap years */
/* --- adjust day --- */
if ( hh < 0 )				/* went to preceding day */
  { dd--;  hh += 24; }
if ( hh > 23 )				/* went to next day */
  { dd++;  hh -= 24; }
/* --- adjust month --- */
if ( dd < 1 )				/* went to preceding month */
  { mm--;  dd = modays[mm]; }
if ( dd > modays[mm] )			/* went to next month */
  { mm++;  dd = 1; }
/* --- adjust year --- */
if ( mm < 1 )				/* went to preceding year */
  { yy--;  mm = 12;  dd = modays[mm]; }
if ( mm > 12 )				/* went to next year */
  { yy++;  mm = 1;   dd = 1; }
/* --- back to caller --- */
*year=yy; *month=mm; *day=dd; *hour=hh;	/* reset adjusted args */
return ( 1 );
} /* --- end-of-function tzadjust() --- */


/* ==========================================================================
 * Function:	daynumber ( year, month, day )
 * Purpose:	Returns number of actual calendar days from Jan 1, 1973
 *		to the given date (e.g., bvdaynumber(1974,1,1)=365).
 * --------------------------------------------------------------------------
 * Arguments:	year (I)	int containing year -- may be either 1995 or
 *				95, or may be either 2010 or 110 for those
 *				years.
 *		month (I)	int containing month, 1=Jan thru 12=Dec.
 *		day (I)		int containing day of month, 1-31 for Jan, etc.
 * Returns:	( int )		Number of days from Jan 1, 1973 to given date,
 *				or -1 for error (e.g., year<1973).
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
int	daynumber ( int year, int month, int day )
{
/* --------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
/* --- returned value (note: returned as a default "int") --- */
int	ndays;				/* #days since jan 1, year0 */
/* --- initial conditions --- */
static	int year0 = 73, 		/* jan 1 was a monday, 72 was a leap */
	days4yrs = 1461,		/* #days in 4 yrs = 365*4 + 1 */
	days1yr  = 365;
/* --- table of accumulated days per month (last index not used) --- */
static	int modays[] =
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
/* --- variables for #days since day0 --- */
int	nyears, nfouryrs;		/*#years, #4-yr periods since year0*/
/* --------------------------------------------------------------------------
Check input
-------------------------------------------------------------------------- */
if ( month < 1 || month > 12 )		/*month used as index, so must be ok*/
	return ( -1 );			/* otherwise, forget it */
if ( year >= 1900 ) year -= 1900;	/*use two-digit years (3 after 2000)*/
/* --------------------------------------------------------------------------
Find #days since jan 1, 1973
-------------------------------------------------------------------------- */
/* --- figure #complete 4-year periods and #remaining yrs till current --- */
nyears = year - year0;			/* #years since year0 */
if ( nyears < 0 ) return ( -1 );	/* we're not working backwards */
nfouryrs = nyears/4;			/* #complete four-year periods */
nyears -= (4*nfouryrs); 		/* remainder excluding current year*/
/* --- #days from jan 1, year0 till jan 1, this year --- */
ndays = (days4yrs*nfouryrs)		/* #days in 4-yr periods */
      +  (days1yr*nyears);		/* +remaining days */
/*if ( year > 100 ) ndays--;*/		/* subtract leap year for 2000AD */
/* --- add #days within current year --- */
ndays += (modays[month-1] + (day-1));
/* --- may need an extra day if current year is a leap year --- */
if ( nyears == 3 )			/*three preceding yrs so this is 4th*/
    { if ( month > 2 )			/* past feb so need an extra day */
	/*if ( year != 100 )*/		/* unless it's 2000AD */
	  ndays++; }			/* so add it in */
return ( (int)(ndays) );		/* #days back to caller */
} /* --- end-of-function daynumber() --- */


/* ==========================================================================
 * Function:	calendar ( year, month, day )
 * Purpose:	returns null-terminated character string containing
 *		\begin{tabular}...\end{tabular} for the one-month calendar
 *		specified by year=1973...2099 and month=1...12.
 *		If either arg out-of-range, today's value is used.
 * --------------------------------------------------------------------------
 * Arguments:	year (I)	int containing 1973...2099 or 0 for current
 *				year
 *		month (I)	int containing 1...12 or 0 for current month
 *		day (I)		int containing day to emphasize or 0
 * --------------------------------------------------------------------------
 * Returns:	( char * )	char ptr to null-terminated buffer
 *				containing \begin{tabular}...\end{tabular}
 *				string that will render calendar for
 *				requested month, or NULL for any error.
 * --------------------------------------------------------------------------
 * Notes:     o
 * ======================================================================= */
/* --- entry point --- */
char	*calendar( int year, int month, int day )
{
/* -------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
static char calbuff[4096];		/* calendar returned to caller */
time_t	time_val = (time_t)(0);		/* binary value returned by time() */
struct tm *tmstruct=(struct tm *)NULL, *localtime(); /* interpret time_val */
int	yy=0, mm=0, dd=0;		/* today (emphasize today's dd) */
int	idd=1, iday=0, daynumber();	/* day-of-week for idd=1...31 */
char	aval[64];			/* ascii day or 4-digit year */
char	*small = sizedirectives[(fontsize>1?fontsize-1:1)], /* small chars */
	*smaller=sizedirectives[(fontsize>2?fontsize-2:1)]; /* smaller */
/* --- calendar data --- */
static	char *monthnames[] = { "?", "January", "February", "March", "April",
	 "May", "June", "July", "August", "September", "October",
	"November", "December", "?" } ;
static	int modays[] =
	{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };
/* -------------------------------------------------------------------------
initialization
-------------------------------------------------------------------------- */
/* --- get current date/time --- */
time((time_t *)(&time_val));		/* get date and time */
tmstruct = localtime((time_t *)(&time_val)); /* interpret time_val */
yy  =  1900 + (int)(tmstruct->tm_year);	/* current four-digit year */
mm  =  1 + (int)(tmstruct->tm_mon);	/* current month, 1-12 */
dd  =  (int)(tmstruct->tm_mday);	/* current day, 1-31 */
/* --- check args --- */
if ( year<1973 || year>2099 ) year  = yy; /* current year if out-of-bounds */
if ( month<1 || month>12 ) month = mm;	/* current month if out-of-bounds */
if ( month==mm && year==yy && day==0 )	/* current month and default day */
  day = dd;				/* emphasize current day */
modays[2] = (year%4==0?29:28);		/* Feb has 29 days in leap years */
/* --- initialize calendar string --- */
strcpy(calbuff,"{\\begin{center} {");	/* center `month year` above cal */
strcat(calbuff,small);			/* small font size */
strcat(calbuff,"\\text{");		/* month set in roman */
strcat(calbuff,monthnames[month]);	/* insert month name */
strcat(calbuff,"}\\ ");			/* add a space */
sprintf(aval,"%d",year);		/* convert year to ascii */
strcat(calbuff,aval);			/* add year */
strcat(calbuff,"}\\vspace*{1pt} \\\\");	/* end top row */
/* --- now begin calendar array --- */
strcat(calbuff,"\\begin{tabular}{|c|c|c|c|c|c|c|} \\hline" );
strcat(calbuff,"{");strcat(calbuff,smaller);strcat(calbuff,"\\text{Sun}} &");
strcat(calbuff,"{");strcat(calbuff,smaller);strcat(calbuff,"\\text{Mon}} &");
strcat(calbuff,"{");strcat(calbuff,smaller);strcat(calbuff,"\\text{Tue}} &");
strcat(calbuff,"{");strcat(calbuff,smaller);strcat(calbuff,"\\text{Wed}} &");
strcat(calbuff,"{");strcat(calbuff,smaller);strcat(calbuff,"\\text{Thu}} &");
strcat(calbuff,"{");strcat(calbuff,smaller);strcat(calbuff,"\\text{Fri}} &");
strcat(calbuff,"{");strcat(calbuff,smaller);strcat(calbuff,"\\text{Sat}}");
strcat(calbuff," \\\\ \\hline " );
/* -------------------------------------------------------------------------
generate calendar
-------------------------------------------------------------------------- */
for ( idd=1; idd<=modays[month]; idd++ ) /* run through days of month */
  {
  /* --- get day-of-week for this day --- */
  iday = 1 + (daynumber(year,month,idd)%7); /* 1=Monday...7=Sunday */
  if ( iday == 7 ) iday = 0;		/* now 0=Sunday...6=Saturday */
  /* --- may need empty cells at beginning of month --- */
  if ( idd == 1 )			/* first day of month */
   if ( iday > 0 )			/* need to skip cells */
    { strcpy(aval,"\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\"); /*cells to skip*/
      aval[3*iday] = '\000';		/*skip cells preceding 1st of month*/
      strcat(calbuff,aval); }		/* add skip string to buffer */
  /* --- add idd to current cell --- */
  sprintf(aval,"%d",idd);		/* convert idd to ascii */
  if ( idd == day			/* emphasize today's date */
  /*&&   month==mm && year==yy*/ )	/* only if this month's calendar */
   { strcat(calbuff,"{");		/* emphasize current day */
     strcat(calbuff,small);		/* small */
     strcat(calbuff,"$\\langle$");	/* enclose curent day in <> */
     strcat(calbuff,aval);		/* put in idd */
     strcat(calbuff,"$\\rangle$}"); }	/* finish emphasis */
  else					/* not today's date */
    strcat(calbuff,aval);		/* so just put in idd */
  /* --- terminate cell --- */
  if ( idd < modays[month] ) {		/* not yet end-of-month */
   if ( iday < 6 )			/* still have days left in week */
    strcat(calbuff,"&");		/* new cell in same week */
   else					/* reached end-of-week */
    strcat(calbuff,"\\\\ \\hline"); }	/* so start new week */
  else /* --- terminate last week of month --- */
   if ( iday < 6 ) {			/* only if there are empty cells */
    strcpy(aval,"&\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\ &\\ &");/*empty end cells*/
    aval[3*(6-iday)] = '\000';		/* cells after last of month */
    strcat(calbuff,aval); }		/* add end-of-week string to buffer*/
  } /* --- end-of-for(idd) --- */
strcat(calbuff,"\\\\ \\hline");		/* final underline at end-of-month */
/* --- return calendar to caller --- */
strcat(calbuff,"\\end{tabular}\\end{center}}"); /* terminate array */
if(0) strcpy(calbuff,"testing");
return ( calbuff );			/* back to caller with calendar */
} /* --- end-of-function calendar() --- */


/* ==========================================================================
 * Function:	emitembedded ( imagenum, isquery )
 * Purpose:	Emits embedded gif (or png) message to stdout
 * --------------------------------------------------------------------------
 * Arguments:	imagenum (I)	int containing image number desired
 *		isquery (I)	int containing 0 for command-line run,
 *				1 for query mode, 2 for query mode but
 *				suppress msgfp output even if it's enabled.
 * Returns:	int		#bytes emitted (returned by emitcache()),
 *				or 0 for failure
 * --------------------------------------------------------------------------
 * Notes:     o	If mathTeX fails, these images contain
 *		error messages that can be displayed
 * ======================================================================= */
/* --- entry point --- */
int	emitembedded ( int imagenum, int isquery )
{
/* --------------------------------------------------------------------------
Allocations and Declarations
-------------------------------------------------------------------------- */
int	nbytes = 0,			/* #bytes in embedded image */
	imgtype = 0;			/* image type 1=gif,2=png */
unsigned char *eimage=NULL, *embeddedimages(); /* retrieve embedded image */
int	emitcache();			/* emit embedded image to stdout */
int	maxage = MAXAGE;		/* maxage is typically 7200 secs */
/* --------------------------------------------------------------------------
Retrieve and emit embedded image
-------------------------------------------------------------------------- */
/* --- retrieve image --- */
if ( imagenum<1 || imagenum>MAXEMBEDDED ) imagenum=1; /*out-of-bounds check*/
eimage = embeddedimages(imagenum,&nbytes,&imgtype); /*retrieve image*/
/* --- display message image on stdout --- */
if ( isquery )				/* don't emit error to shell stdout*/
  if ( eimage != NULL ) {		/* have embedded image */
    imagetype = imgtype;		/* set embedded image type */
    nbytes = emitcache((char *)eimage,maxage,nbytes); } /* and emit image */
/* --- show message text on msgfp --- */
if ( isquery != 2 )			/* msgfp output enabled */
  if ( msgfp!=NULL && msglevel>=1 )	/* display on msgfp, too */
    fprintf(msgfp,"\nmathTeX> message#%d %s:\n         %s\n",
    imagenum,(nbytes>0?"succeeded":"failed"),
    strwrap(embeddedtext[imagenum],64,-9));
/* --- back to caller --- */
return ( nbytes );			/* return #bytes emitted, 0=error */
} /* --- end-of-function emitembedded() --- */


/* ==========================================================================
 * Function:	embeddedimages ( imagenum, nbytes, imgtype )
 * Purpose:	Returns unsigned char ptr to embedded gif or png message
 * --------------------------------------------------------------------------
 * Arguments:	imagenum (I)	int containing image number desired
 *		nbytes (O)	ptr to int returning #bytes in image,
 *				or returning 0 for error
 *		imgtype (O)	ptr to int returning 1=gif, 2=png,
 *				or unchanged for error
 * Returns:	( unsigned char * ) embedded gif or png, or NULL for error
 * --------------------------------------------------------------------------
 * Notes:     o	If mathTeX fails, these binary images of gifs/pngs
 *		supply error messages that can be displayed
 *	      o	String initializations for these binary images were
 *		created by my utility file2hex.c, and the important
 *		thing to notice is that the images are broken into
 *		one or more 500-byte strings.  If you change that
 *		length, change variable stringsz correspondingly.
 * ======================================================================= */
/* --- entry point --- */
unsigned char *embeddedimages ( int imagenum, int *nbytes, int *imgtype )
{
/* --------------------------------------------------------------------------
Embedded Image Allocations and Declarations
-------------------------------------------------------------------------- */
/* ---
 * Each image broken up into 500-byte strings.  See Notes above.
 * ------------------------------------------------------------- */
static	int stringsz = 500;		/* sizeof each image string */
/* ---
 * image1[] contains a 1885-byte gif rendering of expression
 * \usepackage{color}\color{blue}\footnotesize\fparbox{
 * (1) mathTeX test message:\\cgi program running okay.\\
 * See mathtex.html\#message1}
 * ----------------------------------------------------------- */
#define	IMAGETYPE1 1			/* 1=gif, 2=png */
#define	IMAGESIZE1 1885			/* #bytes in image1[] */
static	unsigned char *image1[] = {
  (unsigned char *) "GIF89a\xc6\x0\x38\x0\x84\x0\x0\xff\xff\xff\x0\x0\xff" 
  "\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb\xdb" 
  "\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff==\xff\x16\x16\xff\x1\x1\xff" 
  "\x9f\x9f\xff\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9" 
  "\x4\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xc6\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh" 
  "\xaa\xael\xeb\xbep,\xcf)`\xdfx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f:\x11r" 
  "\xc9l:\x9f\xd0(O)\xadZ\xaf\xd8,\x90\xaa\xedz\xbf`!7L.\x9b\xaf\x63\xde" 
  "\x80\xb0\x43\x38\xb2\bE\xe1L\xaf\xe7\xd2\xba\x82\xe1\xa6\x1f\xdc\x6\vUs" 
  "\x0\x5\t{8\n\x10\x6\x6\a\x8a\x8c\x87\x37\n\v\x1\as\n\x1\t\f;\x83\x42\x9c" 
  "vYx9\a\b6\f\f\r\n8\v~Q\x90\x8b\x39\x4\xac\x8c\x36n;\xe\x10\xa4\x5\x9a<" 
  "\x90\x41\xbd\x9fh\x1>\x3\t\xa2\xa9\x7f\xfQ\xe\xae\xbf\xae\a7\xbf\x37\t" 
  "\xc5\xd1\x37\xcb\x43\xd7\xc0\xa0\xc2=\xaf\x38\a\xc7\x37\xdc\xb0\x10\v\x4" 
  "\n\a\x3\n\xe8\x83\f\x4\x4\x87\f\x10\a\x6\xa9\x6\xe6\xec\x39\xb3\x37\f" 
  "\xe6\v\x9c\b(\x91\xda!\x8f^\xaa~\x4\xfe!\xa8G Y\xc1z:\b\x94;\x97n]\xa5R" 
  "\xfe\n,T\xd0\x10\xc0\xc6\x8e\x0\xdc\xc1\xe3\xb7\xe0\x1e\x83\x62\b\xfe" 
  "\xff\x1\xf0v$\xd4\rp\xc6\x44\xb1\xd2\xf7leMo\t\xfc,x\xb3\xd2\x99\r\x96\?" 
  "k\x2\x90hc\x80P\x0\xe\x2\xcc\xd4\xe1\x8d(\x0\xa3\x6\xd8 \xfdY-\xa8\xd5" 
  "\x9e\x43!\x14\xa5\'\xf5MT\x1b<s\x2\xd8i#\x0\xa9\x5\x9a\x9c\x1a\xc5\x8a" 
  "\xc4\xa5\r\x98\xdf\xc2\xbd\x95\v\xed\x90\x37\x96\x1c\x1f\xd8\x65\x46\x95" 
  "\xa6\x34p\xeb\xc6\x15p\xb0\xa0\xc1@\xa6\x87\x12\x0\xbe\x34`\x9e\x83\x81",
  (unsigned char *) "@\xf5\xed\xed\xabx\x1d\x63\xc7\xa4\x1a\x1fx\x1c\xa9" 
  "\xe1!n\x6\xdeT\xe\xec\xc4-\x0\xb8/\xe9\xa2\xc6qw2\x80\x4\xf6&\xc7\xee" 
  "\xcbZ(\xbd\x1d\x87\x1a\x4\xc2\xbd\a]\xaf\x2\xba\x1e\xdc\xec\xcd\x9b\xf6" 
  "\xab\xdb|\x82\?\x3\xce@\xb8\r\xd8l\xcd\x8d<]\xb5\xed\xb8\x1dhc\xe2\xc0U" 
  "\x9c\xed+t6\xf4\xeey5\xde\x35\x34\xa1\x4\x1aX\x3;\xb0@\x0\xa9\x92W\xa6" 
  "\xb7\x66`\x90z\xac\xd5Z\xf7\x9d\xf\xb6\xbe\r\xf5\xfe\x1\xa0\x1ex\x0\x88" 
  "\x17\x12,\xf7M\xfe\xf5\xce\x12\xa6%\xc4\x8f\x1\x98@\x82@\x82\x88\x1c\x80" 
  "\x8a\x2\r\\\x98\xa1\x2\bTtJ \x5\x34\xe0\xc0\x39\x1b\x62\x88\x8a\r\n\x18" 
  "\xd0@\x0\x10\xad\xf4\x0\x61\xa6\x1c\x10\xc0\x31\x89\xb0\xe8\t!\"\xb2\x61" 
  "\xc0\x8bh\xdd\xb3\x6O!\x8e\xa8\x3:\x1a\x16i\xf\x8f\f\xf8H\x80W\xab,\xe9" 
  "\x91\x87\xba\x15x\xc0\x94\v\x90\xb2#\x8c\xaf\x15\x63\xdd\xfG\x11\xb4\xcb" 
  "\x11\x87\xe1\x10\x66\x11\x37\xe\x11\xe6 \xa4\x8c\xe9\x11\x13h\xaeI\x8b" 
  "\x9b\x37\x84\xb9\x0l\x1c%\x90\x8c\ren\xe9\x3\x1_\xea\x80\x80\x96\xda\x4" 
  "\xaa\x85\x39\x37\x38X\x85i6\xc4\xc2\x3g\x82\x36\x8a\x85\x65\xeb\x4\x13" 
  "\x44\x9e\b\xa8\xe9\xe8\xa5\x98\xdeq]\xa6\x9cv\xca\xe0\xa6\x9e\x86*\xea" 
  "\xf\x88\x8ej\xea\xfq\xe4\x19\x46\xa9\?$\xc0\xd3\xa9Q\xb8\x1a\x45!\xd5}" 
  "\xc1\xea\x9eK\xc1\xea\x84\xa2\xad\xd4\xea\xc5\xad\xba\x6\xcb\x16\x1d\xa6" 
  "\x11\x63RH%\xa1\x5[\x3\xd1H\xe4\xaa\x1\x62\r\xa0\x1b\x1\x1\xfe\xcc\x1" 
  "\x8f\x2\xee`dRN\xd3V\x1b\xd2;\x87H\x84OE\xf9\xfc\xd1m\x1\xd2\xee\x91\xee" 
  "P\x1\x8ckm\xbb\x14Y\xc2,\xbb\xeej\xab,,\xf5\x64\x8b\x15\x1\xb7\x89\xb4" 
  "\xc7Iw\xda\xa9\xaa\x9e\x9bx\xeb@1f\x8d\xb5Kd\?\xed\xe6\x9e\x95\xea=\x96" 
  "\xc8\r\xf\xa4\x92pv+EL\x8aXd\xd9tUd*\"e\xa5k\xfb\xdc\x35\x9cq\'\x3",
  (unsigned char *) "pq\x9f\x0L\x1c\xdel\v\x1c\xc3\xf1\x1b\xcd\xfd\x64)\x13" 
  "n9\xd0%h<1<\xec\x66\xc3&\xb0\xdbJ\b\xff\xd4\xf3/y\x91l\x1em\xc6)\xdd\xf4" 
  "\xd3l\xf1\x9c\x83\xd0\xd0P\xb3\xc0jI{\xc4\xdd\xabQ\xb8\xb5\xcf\r\xd2\xf1" 
  "\x85\xd8K\xe5\xbd\x34\xf4,a\xd7\x85\x3t\xfay\xd7L/m\xc7\xed\xb4\xdb\x36" 
  "\xa4-\xca\xd9\xcf@\x8b\xe1@l\x1fR\x12\x3\x37\xe3\f*\x1f\t\xfu`|L9\xec-K" 
  "\xa7P\xac\t\xcb\xc3\x12h\xa0\xdc\x88\xd3\xa6\x17\xd4\x94\xd3}8A\t>\xa0" 
  "\xc9+\xfe\xe$#\xf9+\x5(R(|M\x98\xc6\xc0\x66\x6\xf8!\x1c\x95ql\xc8\xd4\x1" 
  "\xe7$\xc0\x86\x89_^\xe9\xcd\xeb\aT\x89;-P\xc6l\xe1:%\xca\x8e\x62\x86_" 
  "\x16\xa2N\x2\x46\x9aH|\xf3\xc5\x1b)%\xec\xfa\xbc\xf8]\x3\x80\x61\x12\x41" 
  "\xf0\xcf\xcd\x34\r\x14\xa5\xe\x34\xe7:\x4\xd8\xe9\x3\xe9@p2~\x9dw\xfa)E" 
  "\x9aV\xac_~\xfb\x38\f\x1c\xa7\xe\\K\x1,\xa1\x89\xe\xdd\x1d\x11\xfc\x1b" 
  "\x8a\xfft\x15@C1\xc1U\xea\xc0\x2\xb0Z\x6)\x1f\xa4\xa8\x45\x44\x80\x14]" 
  "\x82%\xc1\'\xac\x61\x82]\x1b\x9c\xb0\x36\xd8\xa8\x5r\xf0\x83\xab\xd2 " 
  "\x1f\x14\xe0\a\xfb\x61!UQ@\xe1\x12T(\xa6\x81\x4.T\xaa\xd3\x84\x1e\xba" 
  "\xa4\x85\x41\xd0j\x13G\xb8\x61\x1e:a\b\x1d\xa8\x64\x41\x8e\xea\xc3\x16" 
  "\x44X\x0\xa1t(\fb\xd3\a\x12|\xe6+\xd6\x44\x43K\xfx\xe1\x19Lq\"R\x89\x30" 
  "\x11\x87i\"\x14\xb2\x31,\xfa,1\x1a\\\x4\xfe\x42\x64\xf4\x10\x9eL\xad\x66" 
  "\n\"<\x8d\"\xe4\x92\x92\x39\xb4\x91\x1c\xe3RG\xb9\xfcU\x8ay@\xe4\x1e\xf1" 
  "\n\x89\x1d\xf\x92\x91\x93\x14\x83_\x3\xc4\x63>\x1e\xc2\x91\x89\xa0\x43" 
  "\x8e\x17\xe9\"R\xfc\x30\x80W\x89\x8b\"\x88lGF>\x92\fJ\x96\x2\\$\xd9\x16" 
  "\xb2\x12\xc2\x89\x33\xee\x0Q*\x12\x1\x1b\xd4\x42;\xad<\x85",
  (unsigned char *) "\x86%K\xd9\xcc\x98V2\xd7\x90\xf2)\x86\xc1\xd6\xec\xa0" 
  "\xe6\xb1Z\xeaG\xf\x8b\xa0\xc7#L\xf6\xb1=\x90\xf2+S\x1\x66Xt\xc2\x93\x95" 
  "\x65\x65+\xa9\x19\x62\x10\xdc`\x96\xd1\\\xc2\x99\xa0\xca\\\xcb<\xc3\xca" 
  "\xb9\x41\x33Q\r@\x9d\xda\x34\xa7\xb9\xb6\xd1\"`\x88\xd3\xf\x34\x35\xc3" 
  "\x19r\x1e&k*3\xdak\x16\x33\xeO&\xe1\x8arA\x5r\xdeR\xab\xcc\xf5\xad/\xb3" 
  "\xa1\x9bo\xfct\x0\xfamS\?\xf9\xa4\\\x1c\x14\xb0\x8cu\xd0\xe9\x18\x94\x9b" 
  "\'!\x94\xb3\xd0\xe6\xd4\xe4\x9e\x63\xc1$uv\xe0NM\xf5 E80\f\xe0\x7f\x90" 
  "\xb2\xd1\xfcu\xb3\x37\x35\x99\xdcx>*\x9f\x4\xbd\x61!\x5\xe2\t\x10)W6\x92" 
  "\xe\x14\x30;\xb1\f\xe6|i\xd2\x0\x1\xc8>-\v\xe9+X\xd6\xd1\x64Z\xf1\xa2" 
  "\x86X\x87\xe7~\x82$\x17\x61\xa9\x42\xd0\x43\x45\x87\xd4\xf1!\x1c\x8d\xc8" 
  "y\xces\xaa\x8e\x90$\x8f@P\xeb\x1f\xdf\x83\xaa\xec\x82\x44\x91\xa4>/\x1cw" 
  "\xf2\'\x91\xbe\x1a\xd5+\xf5\xa8IL\xfa\xd1\x93\x98\x1a%\xdeU\xc9\xa8\xd9I" 
  "R\x84~\xca\x83J1pL72\xa1\xf,%E8\x11\xc2\t}\xdd\x41#\xcb\x98\xc3\x37\x1d" 
  "\x6~bR\x5\x9d\xe6\xc7\x87\x96\xa4\x11\x84_p\x0#=\xea\x85\x2\xe\xd0\b\x1e" 
  "\x84l\x15h\xa4\xd7+T0u\x8f\xd5\xach\x7f\x15\xda\xd1\x9aVR\xa7M-\x18\x32" 
  "\xab\xda\xd6\x16\x81\xb5\xae\x8d\xed\x10i@\xdb\xda\xda\xf6\xb6\xb8\xcd" 
  "\xadnS\x10\x2\x0;",
  NULL } ; /* --- end-of-image1[] --- */
/* ---
 * image2[] contains a 2296-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (2) mathTeX failed: probably due to\\bad paths, permissions,
 * or installation.\\See mathtex.html\#message2}
 * ------------------------------------------------------------ */
#define	IMAGETYPE2 1			/* 1=gif, 2=png */
#define	IMAGESIZE2 2296			/* #bytes in image2[] */
static	unsigned char *image2[] = {
  (unsigned char *) "GIF89a\x1\x1\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x16\x16\xff==\xff\x3\x3\xff\x1\x1" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\x1\x1\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcftm\x97@\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8\xa4r" 
  "\xc9l\xe6\x44\xce\xa8tJ\xadZ\xaf\xd8!4\xcb\xedz\xbf\xe0\x30p+.\x9b\xcf" 
  "\xe8\xf4\x8e\xacn\xbb\xdfp$\x1b\x38 \xf8\x10\x8e,BQ\x10\xf3\x83{}\x80" 
  "\x7fz\x84\x83\x82:\x81G\x8aps>\x5\x6\x39\x90\x6\v\b9\x3\vU\x82\x5\t\x91;" 
  "\n\x10\x6\x6\a\xa1\xa3\x9e\x9f\v\x1\a}\n\x1\t\f=\x9c\xad>\ty\x9c\xa7\?" 
  "\xb7\\\xba\x42\xbc:\xbe\x44\xc0I\x88Q\x8e=\a\x96\b\r9\xe\xf:\v\x3T\xa7" 
  "\xa2<\x4\xd1\x0\xa3\x39x\?\xe\x10\x96\x5\xb0>\xd4\xc4:\xd6\x39\xd4\x42" 
  "\xe8Y\xea\x41\xec\xe7\xb8\x44\xeeH\xf0M\xc6;\x3\t9\xb3O\n\x97\xcbR\xe" 
  "\xa6\xc1\x9bv@\a=\x1d\t\xf2\x1d|\x17o\xe1\xeyU \xf6\x90\'\x11H\xc5!\x1" 
  "\xa9\xd8\x33x\xeaZ\x81\x0\x88\x2\xf4 \x0\x61\x1\x1\x5\a\xfe\x6(@)\x88\x1" 
  "\x1\x2\x9e\x18@8`\xa0\x1f\xa5\x93\xabxd\xdb\xc1\xc0\xe4\x2\x44\bTY\xf2" 
  "\x81\x92&\x0\x97\x30\xf5=\xf0\x84\xae\'\x81\x9f\x0\x30)`\xd0\x89\xa1\xf" 
  "\x92\xb5\f$\x88\x36\xe0\x81I\x90\x0`N\xb5\x83-\xc1Kh9\x90N3\xfb\xf4\x1a" 
  "\x35\xaa\xff\x12\x34 &\x95j\xa4\xaew\x97\xa6\xf5IN&\xcd~b]*\xd9\x98\xe3@" 
  "\?\x1e\v2\xe9H9\xb1 6\xc7",
  (unsigned char *) "\xe8\xb6\x2X\x90\a\x1b\x41\xab\x6\x1d\xe7 yI3\x0\a\x1" 
  "\xae\x89\xf3$\x99\xb2Uj\x9c\xa3\"\v0\xd4\xa8\x65\x8b\x8a\?Z2\xe0\xcc\xc1" 
  "\x1e\b:\x1a\xd8\x8c\xcd\x1a@\xe9\xca\x94$\xf5n\xfa\xcf\xc0Pm\xbd\x1\xb8" 
  "\x46\x87\x1a\xb7\xea\x89\x9e@\xe5>|\x84\xb0r\xeai\xff-\xc6~\x9a\xe9)\x5" 
  "\x4\x1ax\xe7\x88\xf9\x9c\xe7\x4\x86W\x8a\x94\xe4`\xc1\x83\xe3:\xbf\x87" 
  "\x1f\xff\x1a\xfd\xcaV\xe\x34\xbbNw\xea@\x1eu\t(V\xd6k\x85U\x6\x9ex\x98" 
  "\xf9\xfeG \x2\xde|\xc6\x43~\x8b\xd1G\x8d}\xea\x41\x97\x43\x80\x6\xe5#" 
  "\xc7z@\x18vO$\x5 \xe2\x61|\xdd]h\xd3x\'\x96\xf7\xd8\xe\xfb=\x94\x83W\?D" 
  "\x96\"\x81\xd4\xb4\xb8\x93r\xe=\xd4_$\xea\x1c `6=F\x92\xc0\x8c\x41\x12" 
  "\x38\x99\x1\f\xc0\xb7\x62\x61\x12\x46\xd2\xa2\x85(\xfd\xe8Y\x11\xd6-\x10" 
  "NTd\x11\x80H\x83$\xd2\x18\tJ9\x88\xc7#\x8f^\xea\xe4\x19\x1\xce\x30\xc3" 
  "\xccP\x1f\x91\x45\"\x98\x0\x88Y&\x9a:8P\x80s8n\xe6\xe6\x44\xbc\xf5\xa1" 
  "\xe\x3i\xc6\tKp\x0\x4U\x0\x9cr\x12j\xa8\x91w\xd2sg\x84V!H\xa7\x9a$\"\x19" 
  "h\x3W\x1a\x61\xddS\xc2\x91\x90H\xa0:\xa0\xf4\xc0J\xf\x8c\xaa@\xa9\n \x90" 
  "\xd2T0\x16\xf0\x80\x3\'\xa1z\xea\xa8\xfa\xd0\x16@M\x6\x35\xd0\x1e\x3\f" 
  "\x1c\x10\xc0\x61\xa0\xdcJ\xcc\xacI\xae\n\xe8\x2\xb3\x92J\xab\x1\xbaZy" 
  "\x14\xb2$y\x98\x90\x45\a\xfe\x9c\x64\x16\x0\xc4\xe6\x1a\x10S\xe(0@\x3v" 
  "\xa8\xaa\xd2\xb1\xd8t\xfb\xad\x1d\xc9\"$\x1aO\xd0\xce\xd4\xf\'*%\xb0l" 
  "\xb3\x99J\xf2\xaa\x1d\xccn;\x18\x87\x1d\n\xc1\xeb\x12Jj\xd3\x4\x39I\x4\f" 
  "\x4\x39\xc9\x14\x31\xce\x10\x4\x3\x80p.AT\xf6\x43\xc2;P\xfc\v ;4L%\xbf\?" 
  "\x10Po\xc5\x1a\xc6!\xf2\x10\x17yQ\x8bJ#k",
  (unsigned char *) "\xc1q\xc7\xeb\xee`[\xca\x30\x13%\nwb\xd4\x41s\xcck\xac" 
  "\fq\xf\b\x18\x8c\xf3\xcf@\xa7l]\xd0\x44\x17m\xf4\x13:\x1f\xad\xf4\xd2" 
  "\x38\x13\xc6H\x11O7\x11\x35\xd3T\x1fR\xa8!a\x10&L/U1\xb1I\xd7\"\xd7\x42" 
  "\x84\xd8n\x90m\x84\xc6\xeW\xb5u\xc6\bI\\O\xd2\xe4\x1dQr:q\x8bl\xce\x10w" 
  "\xb7\x91\xb7\xc2\xed\xd0\x17\x63\x39-/a\xdd\xdc]*\x91\x91\x8aU\x1f}x\x8c" 
  "~\xf7\xb0\xb8\x46p\x9f\xc3\x16ZG\xbd\xd4\x11\xb2vU\x3\x41V\x92U\x9e\xd4Q" 
  "\xfe\x33\xe1z\x13K\x85\xd6\x14^\x10X\x5\xd4\xb9S\?u\xf5U\x1$\x99\x84\x92" 
  "J\xa4\x9f\x1a\t\x2\xa6/\x83\xfb\x81\xb6oV\x93`\x4\x4 {N\xbb\x9f\xde\xf1" 
  "\xefv\xb8\x1e<9\xbd\a\?|\x1f\xc5/\xe3\x17\xaej\xc5\xdd\x94\xe5i\x85N\xaa" 
  "\'\x81\xd9\xe1<Nh\x3\x30\x38o\x96\xfcVhrObC\xbeo\xd1\x98\x66\xe4N\xd4" 
  "\x18@\x96\xdb\xe2\x90\x9f\xda\x0\x5\xd1\xf6\xd9l\x90\x41\x36\x9e\xfcj\x2" 
  "\xa0\x83\xa8!\x9d\x30\xd9\xc4\x7f\xd8\x98\xdf\xf\n\x18\'\x9b\xd4&`\xcc" 
  "\x41\xa0\x0\x81s\n\xf3\x95\x89}\x93\xa1 y\x18\xa8\x9b%]dp\xfd\x31\xd0|>" 
  "\xa3\x1fz\xf4H\x84\bz\xdfx\x6\x30\x93\x97\xf5m1\xe\xa0\xd0,\xd8\xc1\x9c&" 
  "E\xa5\x85\x96`\xa1\x7f\x66#\xa4\x1f)d\x85\x38\xfc\x1\x86$g$\v\x95I\x87.d" 
  "\xc7\x81l\xa8\x8f\x11\xaa\xd0\x37>\xbc\xe0\xf@\b\xa9!\xbd\xe6\x46\xe9+" 
  "\x92\x15k\x88\xad\x32\x85\x88\x1\r\x98\xfe\x92\x11q\xf4$\x1a\x32q\x1c" 
  "\xe0\b\xa3\xc3\xd2\x98\?\'I\xc9\x8bl\xfc\x81\x8f\x32S\xc4.q\xf1\x8bj|M\?" 
  "\xb6\xc8\x44>2$Es4\x8f\x14}0>\xe4\x1c\xca\x31\x62z\x14\x93\xf8$\x9c\x43" 
  "\x86)~d\xe2\xa2\x1\x4\x91\xa6\xbd\x19\xa4O\x93\x1a .\xb8X\xa6I\xbe\b\x1b" 
  "\x94|\r\xa0r3\xa8\xff\x85\x32",
  (unsigned char *) ",\xeb\x1a\x65\x98Ji\x11\x1bz\x12\x0i\x8a\x1f\xa2 \xf9G" 
  "D\xd2rLG\xb9\x14+\t\xf4\x92\x1e\f\xce\\\xe0*\x94\xb1`\xd4\x13\xf0\xb8" 
  "\xcbL\xd6\n\xd7\x30\x33\xe1*X%+Y\x94\xa8\x43\x65\xd0#\x8ej)\xe0Z\xd8\xa0" 
  "W\xb6\xb0u\x0S\xc9\xea\x9b\xa3\x8a&\x1\xfe\x3\x8dq\xa6+_\x4\x4\xa7\x2" 
  "\xc4\x39\xcd)\xa1\xf3K\xa5\xfa\x18\xb6\xd4\t\xcdrV\xa6\x99\x4\x10\x17" 
  "\xab\x90\xa5N}\x92\v\x9f\xe7\xd4U:\xbdI\xabi\xf1`h\x92\xe0\x81\xc1,\xa6#" 
  "\x87)\xf4\xe@`h\x17-D\xb0\xf0\x41\xadP\x89\xc0\xe8\xfe\xce\"\xaa\xd1P=b" 
  "\n\x12\xed\xa8\xcfx\x6\xd1\x8f\xee+\f\x84S\xc2\x1e\x46\x93\xb8\xab\xb5" 
  "\xf4\n\b\x8d\xc2:qu\x85\xc0\x61kf\x89\xb3\xe9K\x9d\x10\xd3\x9d\xfa\xf4" 
  "\xa7o\x3\xaaP\x87\n9\x86y\xcb\xa1\x66\x98\x9a\xd4\xb0V0\xa6\x16j(#%j\x19" 
  "\xac\xf3/H\x88q\x17\x92\x0\x1b\xdb\x86\xa1\xd5\x8b\x5\xa3\xab\x93\xe9" 
  "\x43/G6\x89J$Nk\x9aQ\x95\x18\x4\"\x8e%@$G\x85\x43H\x98\xa2z\x6\x65\x30" 
  "\x3TK#\f(\x8e\x3W*,N\x1e\x8f\x93\x1b=\x2\xfb\xc2\x8cyB;q\xd8\x87\xf8nV4" 
  "\xeb\x90\x82\xa6{yJ\x1fXG\x8e\xd8\xe1\x84v9\xf1\\L\xb4\xa7>\xf0\x81\xee/" 
  "\xcf\x92,U\xf2\x41\x80@^\xd2\xb3\xd3\x33\xe6\xf0\x30+\bw8\x80+\x12\xb3" 
  "\xec\xecV\x92Y\xca\x46\xaft\aJ\v\xf6\xf6\x42\t\xaa\x84\x16*\x1e\x1\v\xd5" 
  "\x86\x66\xab\x0x\xcf\x39\xf8\xbb\x9f\x18\xe1\x87@\f\xbaO\x1d\xf0\xa3\x8f" 
  "r\xfe\xa3\xf2\x9e\xa9\x34\xa6\x44\x82\xf4\xe0x&a\x8a\xee\x32\xa5\x7f\fQ" 
  "\xee\x4\x13H)\v\xf6\xc6Y\xd3}\x86\x80\x98\x16S<\xb0\x46\x86\xaeH\x8f\x62" 
  "\xad\xe7\xb7%b\x86\x93\x91\x91\xefz\xd0\xb4\'\xfa\xde\xf7\x8c\xb8\xb0" 
  "\xabo\xec\x38\x1e\xf8\"1\x87\x41lb\n\xd7\x63\x80<\xc0W\a`l",
  (unsigned char *) "\xa9^\xb1\x33\xaa\x16\xa5\xaf\xa1\x17\xf4\xa3\x1e/H" 
  "\xc0\xf4\xa9\n\xb1\xfe\xdd\x30~\xc7\xb3\a\x5\x4\x64%CZ\tvk\x84\v<\x16" 
  "\xc4\xc5&\"\x90I>\x97\xbe\x1\x80\xc8\xa2\?\xd3+.\xde\x93IX\x5\x8a~\x1c" 
  "\xfe\x92-q\x89\xcbN\xf6\xb8tq\xaa\xccXG\x8c\x8d 3\xa4\xc4\xe9\xa1\xcc}V" 
  "\x1c\x89\x1e\xbf\xd2\x19W\xe6\xe6##Q\xaf#\xd7\x61\x33\x38nZ\xe4\xaeY\x13" 
  "\x5`\xea\x1c\xf4\xca\xe6\xaex *e\xb9\xd9\x9f\xad\xba\xd7\x33Q\xe5\xb0{" 
  "\xa9\xd9J2\xc9\x84\xf3\x38\x91\x8f\x39\xd3\n\xa0\xddt\xb3\x9f\xa9\xf3\xf" 
  "\x10\xb7y\xd0wf\x0;\xd5}\'Ma\x8e\vFa<\x80\x8f\x66\x43\xaf\x8fx\xaajN\xb3" 
  "\xc4J\x94\x44\x8e\x30O\xac\xa4\x18\xf3\xaa\xd4\x94\x30\x80\xca\x80\xf8l" 
  "\x2;\x8e\x44\xe1\xb3\x80\x14\x13@.\xa2\xfeiO\xa5:\x85\xd7\x46\x5\xc8_0I9" 
  "\xd6;\xd4Y\xd3Z\xa6\xfa\xf0\xb4\x15\xee\xa3\xe2_\xfb\xfa\xd7\xc8\x6\xea" 
  "\xb1\x93\xcdl\tG\xae\xd9\xd0\x96\xf5\xb3\xa3M\xed\xb3N\xbb\xda\xd8V\xda" 
  "\r\xb6\xcd\xedn{\xfb\xdb\xe0\xe\xb7\xb8]\x0\x80\x10\x0\x0;",
  NULL } ; /* --- end-of-image2[] --- */
/* ---
 * image3[] contains a 2235-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (3) Can't mkdir cgi-bin/mathtex/\\cache directory: check
 * permissions.\\See mathtex.html\#message3}
 * --------------------------------------------------------- */
#define	IMAGETYPE3 1			/* 1=gif, 2=png */
#define	IMAGESIZE3 2235			/* #bytes in image3[] */
static	unsigned char *image3[] = {
  (unsigned char *) "GIF89a\xe9\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff==\xff\x1\x1\xff\x16\x16" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xe9\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcft=\x2x\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8\xa4r)Z" 
  ":\x9f\xd0\xa8tJ\xad\xf2\x9a\xd6\xacv\xcb\xedv\xb1\xde\xb0xL\x1e\x83\xcb" 
  "\xe8\xb4z=<\xfb\x6\x84\x1e\xc2\x31,(\x6\xb8\x82\xb1\x80x\"\x14zr\x80\x62" 
  "\x7f\x81J\xex~\x83R\x88m\x1\x41\x5\x6y\n\n\x6\x89\x3\v@\x3\t\xe\n\x4\v\v" 
  "\x92\x45\a\t9\x86{\t\xa2<\x5\xa9\x38\x9cR\xaf;\xac\xaa\x39\xb1\x42\xa5K" 
  "\x81\xb3\xabJ\xb8\x42n<\a}\x0\rz\n\x8f\x38\v\x89<\x3\r\xc3\xae\xb4\x42" 
  "\x5\f9\xd1\x45\x6\xd6\xd5\xa2\x4\xcbO\xdc=\xd8<\xdf\x42\x3tK\xaa\xe1;" 
  "\xd9\xe4\xe6\xbf\xc8o\xbe\xe}\xc7\x39\x3\x10>\a\xd4:\x3\xeb\?\n8\xe\xfa" 
  "\x5I\aN`\x15\x82H \x9c:\x12P\xdb\x8e\x86\t\x17\xfe\x0\xa6\x3\xa1+}8\xde" 
  "\xe9(\x10\xe0Y\x8e\x7f\f\b\x10\xd8\x16`\x1\x1\x5\a\xfe\x2)\xe8\xc3\xe0" 
  "\xc1\x1\x3\xfft\x10x`\x12\xe5\x0J)q\x84#\xf0\x12\x0&\x5\fZ)h \xa9\x99" 
  "\xc9\x0\x12\x37\x19X@\x8d\x41(\xa6\t\x86\x66\x33\x90@\xa4\x32\x0R\x1\x10" 
  "(y2\xa7\x8e{Ziv\xbd\x89\x32P\xc8\x91\x38Z\xbe\xfc\xb7\xb4\xeb\x34\x97" 
  "\x30\x1\x38\xfd\x34-A)\x9e\x99r\x80u\x4\xe4@L\x1c\xe5\x12\x9c:\xd0\r\a=" 
  "\x1f\t\xf0,0g\xe0\x80Nk",
  (unsigned char *) "\x16\xab\x39\x6\xd0\xf8\xb1\xe5\x5\xff\x10t\xc4\xd1" 
  "\x93\xb2(\x3\r\x0\xc8\x93\x85Tt\xa9\xcdL-\xf3X\x9ags\xba\xca\x9eu\x84" 
  "\x94\xacsr\xba\xc4\x0\x16\xab\xa6l[R\xba\x99\x80\x1d\x37\xfb\x83Qn\x1c\"" 
  "\x14s\xf8\xe5\x81\xe0\x81\xa1\xe5\xa4=\xe6\x31L\x0\xc2\xe7\xeb\xab\xd7" 
  "\xbd\xc6Ny\x1t\a\x93\x1t\xde\xee\x3\xfc\xe\x64\x6\x18\x17\xa5\x14\x93" 
  "\xe0\x1:\xe4\x63\xd7\xaa\xc8\x9d\xa0\'\xeb\xbb\xc9\xdf\xf6K\t\x19\x81\x6" 
  "\xc7\xcdWDr\x9c\xc5\x84\x0\x46\x9d\x89\xfe\xf7\x97\xe\xf9\xec\x30\a\x0Q" 
  "\xc9\x17\x1f}Xe\xa7\xdakQ9\xc3\x9br\xf5i\x17^2\"\xa1#\t\x3\xd8p\'\x9eo" 
  "\x1d\x9a\x92\xd7\x85\xdc\x45H\x1e[\xd8\x85\x83\x92\x35\b\x1c\xb0\x17\x0" 
  "\x5\xac\xc8\xd7\xf\xa9\xc9\xf5@\xe\r`\xf4\x80t\x80iX\xcd\x1f\x93Y\x87" 
  "\xe2n\xe\x41\x96\x62l\xe\xdcS\xc0\x8f\x9c=\xb9\xca\x66Z\xc9\x65\xe1jyi" 
  "\xa6G|\x4\x19p\n\x98\x92\xa0\x84\x83\x92\xb1-\x99\xa6o\xff\xe5@\a\x2\x92" 
  "@\xd0\x8e\x98\x46\x10\xa8U\x97\xe\xe8\xc1\xc0^\b\x84\x6O\'\x9f\xe0Q\xe3M" 
  "\f4\x80Y\x3\rP\x82\xe8\x82\x38\x36\xe0@\x80\x86\x1d\x90\xe8P\x93.:\x14" 
  "\x7f\x1\b\xe6\x94\'.)Zi\x90>0\xf0\x9e%\x0@p\xc0\xa9\v\xfc\xb1\xe8j\x9d" 
  "\xd8\x13\a\xa5\x9e\xc6\xea\x8a\xe(}\x1a\xeb\xa0@\x19\xda\xe8\xa3\xb0\xc2" 
  "Z\x80\xa3q\x18 \'S-e\xb2\xd5\x2z\xf8\x82\x9c\x46=|x\xd3)\f\x14\xd7\x83" 
  "\x1d\x85\xfe\x11\x39\x84\xb5K\xf4\x81\xad\x10\xc3,\x10\x95\'\t\xdc\xf8" 
  "\x83\x44@\x10 \xed\xb5r\x4\xe1\x11\xb9\x39\x98{\x84\x9dZ\x9d\x9b\x3\x2" 
  "\xca\xb2\xb1\x86I\xed\xea(E\xaa\xf6\xf6\xc0o\x9d\xcc\xf6\x30\xe\xf\xa3" 
  "\xf5k/{\x94X\x81\x80\xb8\x6/\x8c\x4\xbc\xd3\x31\xb7\xad\xc1\x14\x43\x81" 
  "\xc0\xc4j\\\xfcp\xc0\x15w\xec\xf1\xc7\x3r\f\xf2\xc8$\x8f\f\xf1\x11\x85$",
  (unsigned char *) "\x91r\xc9\xcc-R\xc4\xca,Sq2*\x6\xf9\xb0\v\x84\xed \xc1" 
  "n\x16\x37\x17\xd1\xf3\x18\xb6\x4\x11t\x14\x33\x1b\x11\x19\x11\xbf\x15" 
  "\x66\x34\x19G\xfT\xb3\x15\x3\a\x11\x35\xd1\";\xd1\xb4\x10W\x17\x1\x91" 
  "\x18Y\xfb\xd0u\xcc\x1bk\x92Jjg\xa9\xa2T\x8fm\x95\x95\x96I\xc8\xee\xf0SPe" 
  "\x12\xe5\x93\xa1[\xe9\xc9\xb6\x1egG\v\xd7\?#\x1\x15\x87Qu\a\x5V\xb8\x91" 
  "\xf4\x93wwn\xadM\x97OcS\xb3S\x82\x38\xcc\xc4\tU\x89\xcc\x85,\xe0H\xcdT" 
  "\x13\x61\x38\x19#7\x9c\xfe\xf7\x1\x0zuX\xc9\xad\x15L\xb3\x1d\x9b\xf8\xe8" 
  "\fgT5Gz8P\nn\xba\xc1nZm\x96\x1\xe7\xd3\x87\x9a\r3\xdeg\xa1\xc9\xa3\xfb" 
  "\x0)\x95&\xbb|\nPY*[\xc1\xb3\x4\x96\x1}4m\xfb\xf1\xb0\xed\x44%\xf1\xd3" 
  "\x97\x12\xe\x66\xe0\xe4\xc5\x11\x2\xc3;\x6\x9ah\xd1\xdb\xd6\x9b\x65\x6" 
  "\x1c\a\x9f\xfa\x96%\xaf\x17[\xe7\xa7\xf\x90\xf\x4\x9a\xb7\xc3}\x92\xd8Oa" 
  "l\t\xf0w\x18@\xe1\xf9\xddn\xfa\xc7\x9e\x0\xe8O5\tX\x11U\xe4\x83\x83\xe6" 
  "\xf4!g\xe5\xf9\x90\x84$A\xc0\xfe\x1c\xb0;\xd0\xd9\x92xd\xe7\xbfG \x84L" 
  "\x96\x19\x80KF#\xc2\xf7\f#\x1c\t\xac\x86\xf6\xb0S\xc2\x82]\xa1j\xb0\xa9" 
  "\x5\x8c\x36\xa4\xe\xec@\xae\"\x1\xb4\x12\xe4\x62\xd8\xbe\x3(P|\xd1\b\x5" 
  "\x3\x30\x46\x43\x87\xc4\x66\x87\x12\xa4\xcaP\xac\xe5\x9e\xc6\x44\xe3\x83" 
  "O\nG\x1\xa6\x61*\x1cQ\xf1|>\xa4\ry\xa6\xb8\'\t\xe6\x80@\xdf\x8b\xfe\x9c" 
  "\x99J\x85\x8d\x30\x66i\x82Z\xf1\x93h6\xa2<\x1\x32\xb0M\x0\x31\x63\xb0|S(" 
  "\xbd\x38.\x1aSR\x85Hl\x86\xa5\x39\xe6N\x8dy\xea#\x94Z\xc7\x1a\x1c\x65\xe" 
  "\x90\fD#\x1a\xe9\x44\f\xca\x4\xc2O\xe1\xa8\xe3\x99\xee\x88\xbeGj\x5R\x0" 
  "\xb0\x93\xa8\x2\x32\x0\\\x15*\x13\x9b$U\xafV%,\a\xf4(",
  (unsigned char *) "\a\x9b\x9a\t\x7f,\x85(\x8c\x94\x92l\xa3\xc2\xc3\xaf" 
  "\x1e\x45\x19\x39\xc9\xa8\x95;\xc0\x8d+\xea%\x9bX\x8e\x32Q\xb5\x34\x65S|" 
  "\xd9\x0L\tF\x1d\a8IUt2,\x6PJ\x1f\xb5\x8a\xd5/+\xa1\f\x2\xc0\xa7\x9at\x80" 
  "U0oi\xab\xa5\xc0\x81\xev9O\xd5\xe6\xa5\x3k\x11\x31\x62\xe9:\':\x1b\xe8" 
  "\xa0\x8d\b\x1\x82\xdc\xaa\x43;\x91&\t\x89\xec\xec\x65\xa2#\xe7\xc4\xee" 
  "\x99\xcf\x89\x15\rlI\xe0\xc4M<\xf6\x35\x80~q\x9c\x6u\x2\x1c\x18\xd5\xafJ" 
  "\xc4%\xa1\x41\xf8\'D\'J\xd1w!\xb4\xa2\x18\xcd\xe8\xfe\x8e\xc6u\a\x1c\xa1" 
  "\x1\x66\x16s\x99\x12@\xda\xc0\x61\xa8S\xa3\x61\xfb\x41\xb4p\xc4\xc3.\xe8" 
  "\xa2\x15\xbc\xd0\x19Lc*\x8d\x99\xe6\xa0m{\xec\x98\x1d*\xa1\x34\x33\xbc.<" 
  "5\x1a\x83\x88\xc0\xa1\x84\xa6=\x8dI\xb3*\xd5I\xc5P\f\xac\\t\v\x4J\xde" 
  "\x33\x8e\n\x85\xad\x45\x66kG8\x1aV\x9d&\vQ\xb4\xce^\xa3\xf9\x1f\x1a\xect" 
  "\x80\a<Tqm\xb3\xdc\x42\x34\x37\x96\xce\xa5%Di\xd9\x1b\xe2\xd4\xa6\x96" 
  "\xb8\xa8\x35(w\xc9\x62\x45\x36g\x96\xbd\xb1\xd5&nM\xa4h\xf0P\x8ev\x89\x5" 
  "\xb0j\x93\xcb\xddX\':\x98\x90\xce\x38hY\xdbR\x82\xa2\xd8\xc5\xd5\x42^b" 
  "\x80\x17hD\x10\x87\xf0\x85o5\xe6\x43\xa0\x62\xd4\x43\x1b\x45~v8@\x1m~" 
  "\xd0\x11Z4\x16\xce\x89\xb0\xfdLk\xad\x17\x1c\xf9\xad\xd1\xb6\xe6\xa0\x9d" 
  "\x39PC\x8d\xcf\xfa\x44v\xfc\xe4\xc2\xc9\xe6\xd0\x91\n\x1e\xc3\xb8\x1\x3!" 
  "u\xf0\xc3@\xe5\"7r\x0*\xc8j\xfe\x8d\xa8\xdc\x6\xe\xae\x86,\x82P\a[\xd8\a" 
  "\xee~\xa4:\xa2@\xf\x38;XN\xe7\xa8!\xaa\vJ\x14\xe4n\x88]E\xba\b;3D\xe3" 
  "\x8c\x98\x63#\xe9\x36\xb7L\x8a$\xcf\x1f\x14\x10\x10J|\xab=)\x82\x1c\x17" 
  "\xab\x38\xe0\xc9\xbc\x17\x44\x91\x85\xdc\x81\x38t\xde\xaaUB\a\xce\x80",
  (unsigned char *) "\xa3h$\fO\x10\x8e\x11M\xe1P\x93~(\xdc\xd8R\x99#\xa7" 
  "\x13\xcap~\xb1\xb3_\xfe,\x86=\xd9\x8d\r\x85\x19\x19\x1a\x16\x63%I\x91" 
  "\xdc\x1\x85[\x2$\xccz!\xaa\xa9\xa0\x4\x4\xf4\xf1J\xc7\x35\xf3~\x92\x92" 
  "\xe6\xa2<\xa9\xabY\x9e\x84\x95\xc0\x34r0\x89\x45\x13\xad\x94\x84\x15\xa5" 
  "\x98\x66\x1e\x80\x15\xcdij\xf3LX\xa6U\x90\xa5\xdc\x63oZ\xb3;\xdf\x14\x1d" 
  "\xe7>Y\xaaS\xf9\x30z\xcd\x9c\x83\x9e\xbe\x1a\x6\x2i\x8c\x12\xd2YHp\x7f`" 
  "\xcex\x9a\"\nK\xad\x87\x39\xd8\f\tv>C[\xf3L\xc6\xb7\b\x10\xae;\xd7\x43" 
  "\xa4\x63}*J\xbb@\xd0\x88\xc2\x96\x1_\x91\xd3W\xc7$\xbah+\xc4\x4\xd1\x62@" 
  "\x18\x43\'\xad\xe8J{\xba\xa2\x94\xfe\xb4\xa8\x63\x16\xeaQ\x9b\x1a\x64" 
  "\xa5>\xb5\xaa)\x96\xeaU\xbbz\r6\x88\xb5\xacgM\xebZ\xdb\xfa\xd6\xb8\xe\x1" 
  "\x0;",
  NULL } ; /* --- end-of-image3[] --- */
/* ---
 * image4[] contains a 2238-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (4) Can't mkdir cgi-bin/tempnam/\\work directory: check
 * permissions.\\See mathtex.html\#message4}
 * --------------------------------------------------------- */
#define	IMAGETYPE4 1			/* 1=gif, 2=png */
#define	IMAGESIZE4 2238			/* #bytes in image4[] */
static	unsigned char *image4[] = {
  (unsigned char *) "GIF89a\xe9\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x1\x1\xff==\xff\x16\x16\xff\x3\x3" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xe9\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcft=\x2x\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8\xa4r)Z" 
  ":\x9f\xd0\xa8tJ\xad\xf2\x9a\xd6\xacv\xcb\xedv\xb1\xde\xb0xL\x1e\x83\xcb" 
  "\xe8\xb4z=<\xfb\x6\x84\x1e\xc2\x31,(\x6\xb8\x82\xb1\x80x\"\x14zr\x80\x61" 
  "\x7f\x81J\xexlF\x88;n<\x5\x6;\v\n8\x3\v@\x3\t\xe\n\x4\v\v\x91\x45\a\t9" 
  "\x86{\t\xa0\x8f\xa7\x38\x9aQ\xac;\x5\xaa;\xae\x42\xa3y\x8a\x43\xb5:\x8e;" 
  "\a}9\x3\r\x94\x38\v\x89<\x3\x10\xbe\xab\xa8u\f9\xcb\x46\x6\xcf:\xd1\x38" 
  "\x4\xc5N\xd6=\xd4;\xd9\x42\x3t8\xd2\xb7;\xdfW\x1\x98\xb9\x0\b\f\a\xc2\x0" 
  "\x3\xf>\a\xcd:\x3\xe2@\xc2\xe\xf7\x42\xdb\xda\xfbU\xfd\x90<\b\xa4o\x1c" 
  "\x90\x81\xe6\x80\x4\xa4\xd3NW\x8f\x2\x1\x92\xe5\xa0\xc4\x80\x0\x1P\x4\x2" 
  ", \xa0\xe0@ \x5}\x18\x34\x38`\xc0]\xb5\x6\x1b\xfe;\xeP\xd0\x31\x10\x35" 
  "\x2$\xdfMb\xa0J\x1\x84H\xc7\x36\x6(\xf5\xee\xd4\x82\x66\f>\xfdL`S\x9c" 
  "\x81\x4\x16\x89\x1(\n cJ\x8f;\xe2\x1\x10I\x92\xe2\xc6\x5\x5\b\xa0\xe4x`e" 
  "\xcb\xa6\r4\x1d\xc5\xe3\x94k\xa0\x8a\x17O>\xf5\nU\x87\xd4\x46\xe7~4\xac" 
  "\xd6g.\x8e\xae<\x14\xc4\xed\x91\x0\xcf\x2p\x6\xe\x84\x93\x16p\x9a`\x0" 
  "\x81\a+\x9e\xa4.\xe2]P",
  (unsigned char *) "\xdb\f@\x0\xe0@\"D=\xe\x46\x39\xfe\xa9\x98\x87\x81K\x0" 
  " \xf6\x89|8`Eg\x18\x1bT\x12\x9c\x18qi\xc8\xa0\x45\xbb\xee\xdc\x17\xc0" 
  "\xdfp\xa5_\xeb\x38\x9dP\xae\x30;wM\xda%\xe5\xf8\x15\xeN\xf +\xf7|/\xf2r" 
  "\xc4\v\xe6:8\f &\xe2\xe7;\xa6\x37\n\a\x18\'Ka\xfd\xe\xd0q\xdey\xd5\x34P" 
  "\t\xda\xb1<G\xfezy\xf1\xee\xe3/%\x90\xbcs{\xf3=v\xe5\x98{1Z\x83\a\xf4Tg" 
  "\xd2~\x1\xe2\x30\a\x0\x44\xc5\xfew\x1fjK1g\x1f\x64\x44!3\xdb\x63\xfjC" 
  "\xdd\x30\x16\xa1\x42\r\x3\xd1`\x17S{\xfd\x14\x0\x9a\x62\x1d=\x3\xa2\x87" 
  "\x91\xb4\x97\xe0\x89\xe5\x89\xe8\x83~\xc3\x14\x18\x9c\xe\rH\xf4\x8b\x84" 
  "\xce\xfcqXr)b\xc7 a\xcf\x91\xe7@<\x5\xa8\x46\xa1\x82\xe2\xc8\xd6\xd4T\xe" 
  "z\x6\x1a\x2;!Y\x9e\x1\xa5Pc\x0\x1\x93\xe1\x30^\x90\xb0\xe5\x11%y\x1d\xe1" 
  "\xc0\xa3\x94\xf2Q\xf9\xe2^=t\xa2\x3$\x1$@\xf\x2Y\xf6\x90\xc9&\x9d\xe0" 
  "\x81@W\n0\x0\xc1$\x10@\xc0R\x9f\x3\x16\x0\x81\x3q\xe8\xd0\x91\x9f\x36!\n" 
  "\xa8M\xea\xb5Y@P\x9c\x8c\xf4\xa7\xa2\x10\xc8\x88\x3;\xfa\xe0\xf1\xc0\x1" 
  "\x9c.\xf0\a\xa0\x9em\x2O\x1c\x89Nj*~\xa4\f\x1a\x87\x1\xf\x38\xf0\xd3\xa1" 
  "\xa6\x96\xea\x1aGH-\xb5(\xa0w\xae\xa4\xe7$\aPz*\x82\?\xc0xW\x10\fX\xfa" 
  "\xca\x1d;\xd8H\x84\xb2N\xf4\xc1\xac\x10\xbe,@\x14\'\t\xfe\xbc\x5\x4OA" 
  "\x10`\xac\x65\x45P\x83\xad\x1cGh\x1b,\x9ai\x1ak`:\x6\x91\xb1Q\xejV\xe1)" 
  "\x15\x85Q\xf1\xee\x99\x42t\xc3\x43\x65\xe9\xaa\xf1\x1dKV `-\x14\nD3\xa0" 
  "\x14\xfe\x2!,)r<\x9b\xef\xc2H \xa0\x30\xc3\xe\x1bL.\xc3\x14Wl\xb1.\x13_" 
  "\xac\xf1\xc6\xb7\x1c\xbc\x44!C\x80\xcc\xb1\x81\x83\x18!\xf2\xc8Ux",
  (unsigned char *) "\xac\x4,\xff\x90\x12\xcb,H|\x9b\x5\xcbG\xd0\x8c\x6\xcc" 
  "\?\xe0L\x85\xcaJ\xc4\xeb\xc3\x36\xf6\"\xd1\x32\xbc\x43\x37YF\xd0\? \xbds" 
  "\xc6R\xf8\xec\xf\x14\x5\x89\xe1\xb4\x42\x45\xa3|\x4\x8c\xec`\xf5\x0Q\xb0" 
  "\xc4sQ\x9eq\xe4\x94\x91\x1e/YW\xcfL5\xdd\xf4\xce\x9e\x63Ou\x95\x1e\x99|V" 
  "\xecH%5U\xd2ib\xef\x44\x93T\xd5\x42rO\xdc\x9c}f\xd6\xa5o\xf7$\xb7{0=\xa3" 
  "\x95X\xb5\x11\xdeI\x1yg\xb5\x95J,A\xc5\x14\x2%\xd1\xa7N\xe6\xf\x30\x65" 
  "\x37\xd8\xfeMi48\xe6\xc8\t!,\x80\xb6\xe\xb3\x94\x91\x0t\x8e\xd8\x64\xf8R" 
  "\xc3X\xb2\xc5Y\x17\x19\xec\bh\xb5\xda\x65\x94\x8d\xb2\x8d\x2\xac\xbb." 
  "\x19\x65!Ie\xc0hIF\x99\xd9\x84/\x19\x39\x80G\xca\xfb\x1e\xc9\xec;|\xe6" 
  "\x65\x1f\xba\xbf\xc3\x1a\xee\xcc\xeb\xe6\xde\x95Z\"V\xe8\x96\xab\xe7 " 
  "\xbcn\xe0Sf:\xd3\xc7\xc1\x9e\x0\x1f\xcd$0\xe2Q\xf2\x41\x37\\\xe\xda\x1dy" 
  "_z\xdf\x5\x90\?\x83\b\x9a\x9f\xf4t\x80\x80\x1a\xa9\xf\b\xff\xab\x10\xff" 
  "\xd6\x93@\xfb\r\xec=tX\xa0^\xea\xc7\"\xf7\x0\x43<}\xb8`\xec\"!\?g\f\xd0" 
  "\x82#\xc1W\x10\xe\xd6\x80\x2\x38 (\f\xd0\xc3\x1\xe6\xc7\x9a\x65\x1c\xc5&" 
  "\xcajMu\xb8\xb4\x1f\x17^\xc8=\x1d\x61\x61\xfdlc\x0\x6<lB\x0\xa4\x86\xd9" 
  "\x80\x18\x8e\b1+<\x91\x18\"\x5i\xe8\x9e\x2<jS\xa1yb\v\xab\xa3\x43\xf2" 
  "\x38\x91\x1P\x1c!\xfbp\x83\x87\x41]*N\xa8\v\xfe\b5\x86\xf4\b\xd6\xd9\xe;" 
  "X\xc2\x9f\x92V\x95\"=\x99\xaf\x19\x85)\x12*,\xf2\x90\xe2\xb0Q1i\xd4\xd2" 
  "\x1a\xdd\x43\x46\'yI\xfyT\x9f\x18\x99h\xa5@L\xc6L\x0\x38\x64$\xdc(&8.\a" 
  "\x91Y\xa2\x63~\xb6\x18\x9a,=\xa0P\x88i\x15\x35\x12\x45\xf\x46\xad\xe7};" 
  "\x80\x94V\xd4s\xabJ9\xa3",
  (unsigned char *) "U\x9c\xc1\x94\x1\xf0 (BeR\x1f\x91\xe0\xa4,\x8a\x91\x0t" 
  "\xe5@\x95+)%%X\xe5*\xa0\x88g\x95\x9e\xd4\v(\rC+L\xf2\xf2U}\xea\x64\xaf" 
  "\x62\xa5\xcb\xcf\xc0\x61<\xc4 \xc0&@\xc5\xcbM63\x9a\xe0\xa8%\xbd\x82\xe0" 
  "\ve\xc9\xcc\x64\xea(\x2Ol\xc4\xad \x80\x63YuHV\xb7\"\xf1\xado\x16\xc1Y9" 
  "\x80\xa7\xf\xdc\x19\xce\x1f\xe6\x80gV#\x82&V\x92\xaf\xa9\xe5s\x92\xff" 
  "\x94\x2\x1c\x1e\xb8\x86\x80\xd5-\xa0\xeb\x43\xa8\x42\x17\n\x5|2\xf4\xa1" 
  "\x10=\x98\x1dX\x89\x86\x93\x41\xc1\xa2I\xc0\xa8\xfe:\xba\tQ+\b\xabX\xa1" 
  "\x91\xa1\x17\x2\x61\x33\xe3\xc4,\x16\x8f BIs\x80\x95\xa6`ra\xd4\x43\x3" 
  "\x8c\n@\x9d;\x8d\x41\x43\x46\xe9Ys\xd6\xc9\x83Z<\xc0\x9ei\x0\x6\x41\xc3" 
  "\x0#\xe0%\xa3jP\xc3\xe9\xbd\x90Z=qDm\bq\x4\xc5\xbf\f\xb2\x8e\xfb\x91\x41" 
  "X\ah\xc0\x41\x1d\xd7\xd2\xa0<\x8e\x1b\x93\xc3\xd3W\xa6\x92\xa1K\xd1m\x97" 
  "O\xd1\x3U\xea\xe6U\xac\xd0\x64\x14\x30\x19Q8\xd2j\xd6\xaahe-\x95s\xc9" 
  "\x33\x18Q\xe\xb5p\x85-gy\x1b\xe9\x34\x37X\xa9\xa0\x5\x15\x41\x91\xdb(" 
  "\xdaJ\x10\x1\xad\xe1`\x92\x11\x41\x1c\xb2\xf7<\xca\xde\xb0{\xb4\xf1Kw" 
  "\x9c\xe1=\xa0\x39O0\xc7\xf8\x83\xa5Z\xd3\x8f\xd2\xe6\xa6\x42~\vL4HB\x1a" 
  "\xdc\xe0\xf1\xb3\xe9\x1b\xcf\xf8V\xa1Y\x1cl\xa6\x19\x96mJ]\x86\xea\x5" 
  "\x95\xcd!\"\x12l\x93z&\xd8\xd4\n\x1d\x87>\xd8\xa9`p\xab\x1\x81\x97\x6Q" 
  "\xa9\xc6]P\xc1\xfe\x80u\x1e\x5\xe\x37\x0\x1a\xcc`\b\x93\x81\x1cP\xc4\xc5" 
  "\x0\x11\xbc.p\x1c\xab\x86\xa2\x9a\xc4OCTbq\xc9\xb4\xa2\xe5\xec\x92\x86%" 
  "\xe2\xc1\x9d\xa6J&\x1c\x92\x89<\x7fP\x80>X2-\xf0\xd0p\x88W\x84\x62\x80" 
  "\xf\xd3^\f\xa5\x65\x86:\xe8\x8f\x1\xfe\x63\xae\xde\xb2/`:@",
  (unsigned char *) "F \t\x15\xa7s>\x97\x44;\xb2R\x8a\xee\x9b\xa2\to\xaeu" 
  "\xe0\x90\xe4}\xac\xc4\x61\xc5\xe4W=\x7f\xf9Nt#1aH\"\xc6\x90K\xc9\xf0\"" 
  "\xb9Qa\x1dX\xd5\f\xf>\x5KP\xf7\xca\xc0\xa1RF\xb0*U\xa9r\x95\xa7=U\x92PB" 
  "\x6U+W\xf5\x63\x91\\\xc2)\xb0\x18\x45\x92\xfd\x94\aU\x5Y\x97\xb2\x12\x93" 
  "\x96\r\xb5\xcc)\xef\xf2\xc7\xce\x94&t\x9e\xa9\xe<\xed\xaau\x9cZ\xe1h~" 
  "\x9c\a\x3\xb4\xa9\xc1_`_\xc4Xb#\x9e\xd0\x13\b\xcc\xb2g9\xfd\x90\x84\xbe" 
  "\xd2\x37\x9d\xf5\x8cg8\t\xa8\x3i\xb1\x84\x0\xd5\x42X\xbe\x1c\x43\xdaQ+" 
  "\xf0\xd5\xc2\x62XW5\xe4\xba\x30\x46\x37\x9a\n\xc2(Y\x19\xf6\xc5\xdb\x8eQ" 
  "\xf2\xd2\xa0\xe\xa8\xa5\x43M\xea\x8a\x8d\xba\xd4\xa8N\xd7\xa9S\xcdj6\xac" 
  "\xba\xd5\xb0.\x83\rfM\xebZ\xdb\xfa\xd6\xb8\xce\xb5\xae\x43\x0\x0;",
  NULL } ; /* --- end-of-image4[] --- */
/* ---
 * image5[] contains a 2212-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (5) Can't cd cgi-bin/tempnam/\\work directory: check
 * permissions.\\See mathtex.html\#message5}
 * --------------------------------------------------------- */
#define	IMAGETYPE5 1			/* 1=gif, 2=png */
#define	IMAGESIZE5 2212			/* #bytes in image5[] */
static	unsigned char *image5[] = {
  (unsigned char *) "GIF89a\xe6\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x16\x16\xff==\xff\x1\x1\xff\x3\x3" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xe6\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcft\r\xdcx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8\xa4" 
  "\x31\x0`*\x9f\xd0\xa8tJ\xadZ\x9b\xd8\xabv\xcb\xedz\xb5L\xe7wL.\x9b\xbd" 
  "\xe1\xb3z\xcdn\a\xd3\xc0\x1\x81\x87p\b\v\x8a\xc1\xadP, \x94\b\n|T\x81" 
  "\x83I\xeznE\x88Pp>\x5\x6\x37\f\r\x6\v\f7\x3\v\?\x3\t\xe\n\x4\v\v\x91\x44" 
  "\a\t8\x86\x43\x5\t\xa3G\x9d;\xaa\xac\x38\xae\x42\xa6{\x8a\xb4Q\x8e=\a" 
  "\x7f\x0\x6\t\av8\v\x89;\x3\x10\xbd\x37\xab}\x97\x37\xb1\x43\x6\xcf\x44" 
  "\x4\xc5\x39\xd1;\xd4\x42\x3\xc2\xbe\xb7q\xdcJ\xba\xc6\xb5\xdd\xc6\r\xbb" 
  "\xcd\x38\x3\xd2@\n7\xe\xec@\xd7U\xf3H\r\x83\xf0\xde\?\xf7\xb9Y=\xf5\x6" 
  "\x14|\xd2!&G\x81\x0\xc9p\xb8\x63@\x80\xc0(\x2\x1\x16\x10Pp`\x90\x82\?\f" 
  "\x1e\x1c\b8\xae\xd2\xa5L\n\x18,\xebh\t\x0\x3Q\x96\xfe\x12(\x80\xc0\xeeWC" 
  "b\x0VF\x82(\x91\"*\x0\xe7Lj\xe4x\x12T\x1\x2\xfj\x1e\x18 \xb0\"\x0\xa0" 
  "\x9d~\xe9\xa1\x39\xd1\xa8\xc9\x86\xf\x83\x36%jSG\xce\x46\xfex\x1cp\'\x89" 
  "\x8f\x2M8\x86\xeePPPG\x2=\v\x84\x19\x38\xe0LZ=\x83\x1\xf8\x38H\x80\x0" 
  "\xe1\x8d\x8d\xaf\xe2\x2\x98\xdb\xe4OI_-\xc1\x1e\xfc\x33om\xdb\x1c\fq\xcc" 
  "\x3\x8a\x89\xad\x61_",
  (unsigned char *) "l\x1;\x13l\xf7\xf1\xbc\xb3\x0\xd2:\x8bl9V\xe2~eul\xe5" 
  "\xa1\xf7.W\xb8\tm\xc5$@\xe9\xb0\x64\x1doq8\x88\xfc\x8e\x36^\x1d\xb3\t:S" 
  "\x1b\x89*\xd7z\xc1^\x17\x8e\x45\xee\x35\x30\x81\x64\x85\x8f\x1a\x1e\xd6" 
  "\xe\xf3\x1b\x9fZ+w\xadl\x8a\x38\xd1\\U\xe1\bpz\xf4\x8e\x3\xe9n\xd4\x1" 
  "\xa0r\xfakk\x91No\xb6\x66\xdb-\xed\x1b\x12\x1d*\x8e\xc4 \xdarVx\x9f\xd7+" 
  "\x0\xf6\x30\xc5g\xfa\xdd\x17V$\xcf\x95\x17 u\xfcY\xfe\x97\xd5\xe\x7f\r" 
  "\x90S]\xc9<\x90\x1a&\xc8X\x13Hd\x94\x10( l\x4\xea\x30\xd8\r\x4\x14\xf0" 
  "\xc0\x80y\xf5\x32Gx\xd4Y\x3V]|\x1c\xe8\f*\xd7\x18@\x0\x4\xb2\x99W\x18" 
  "\x65-\nH\xd1\r\x19\xda\xc8\x8a\x1\x37\x85\xb3\xa0\xe\xa0\xc8\xc6\xc7_\x0" 
  " @c\xf\x9cx\x2\x8a\x1e\b\f\x15\x12\x4\v\xac\x4\x81@\x10\\\xe9!\x4\xe\xcc" 
  "\xa1\x3\x3\xc1\x18\xa0\xc7I\x9fh\xa4\x9e$a\xea\xd1\xc0\x1l.\x10H\x96g" 
  "\xfa\xe2\x89\x83sX\x89\xe5\x95vVg\x10\x97s\x18\xd0\x80\x3\x96P\x84\'\x9c" 
  "y\xae\x35Q\x2u\x12\ng\x94\x44\x31@\xa5\xa0wF\xaa\xa7\x14\xd7\x89\x96\x3Q" 
  "\t1\x80\xe2+y\xe8\x30\xa1\x10\x9f\x8a\xe7i\x92\?\xf4\xb2\x80J\x9f$p\xd5" 
  "\xf\x41\xfe@\xc0\xa6\xa4\x9e\x42\xc4\x35\xad\xee\x10\xaa\xf\xafRQi\xe" 
  "\xb9\xf6\x80@q\xfa\x1c!\x11\xe\x45Z\xe1&=\xf1Hq\xac\x82\xa1\x61SM\xe\xe" 
  "\xdc\x1a\xac\x10\xc8\xfe\tt\x5\x2\xab\x46\xa1@4qJ\x81m\x15\xbbzH\x87\xb4" 
  "\xd3*\x82\x0\xb9\xd3\x9e\v\xee\x90\xe5\xb6\xeb\xee\xbb\xdb\xb1\v\xef\xbc" 
  "\xf4\xba\x11.\x14\x85\x80*H\xbd\x39\xe4[\x84\xbf\xfc\x32\xbb\x5,w\x8c" 
  "\x34\v\x12\xb5^Ap\x1f#\x99q0\x10\xfS*\xaf\x14\xb1\xf9\xb0\xd8\xb3\x45${E" 
  "\xc5\x41p\xec\x45\x36\x42\x80\xac\xeb\xc4Qx\xbc\x83\xc9\x44\xe4",
  (unsigned char *) "\x43\x6\xca\xffh\x1c\xf0\x12\x13\x83\xb9@\x1\r\xa8\xa4" 
  "\xca\x39\xe\x85\x34\xc7\x31\x12\xe9u\r\x1\xb7]Z\xa5H\xe9\xb1\x4\x0\xcf" 
  "\x10\xf1\xd1\xd3\xccG\xaf\x62IF\x1b\xb9\x93sbH\xc7%RN\xaa\x42\x12\xf\'" 
  "\x1e\xf9\"\xd4 K\xf3\xc1uI\?\a\r\xe2\x3Iaf\x92\x44\x33W\xfd\x93T\x14Qe" 
  "\x94LI\x6\xc4Z\xdd\xd1\xc5\x64\xf4Q\x1%\xc6TU\b\xd8\x9dm\xbc\xcd\xe2\xd0" 
  "\xc0%+\xc1\x17\xd3\x88<\xba\x63\x0\x8d\xd1JV\xa5\xadv\x1p[a\x90#\xc0\xd8" 
  "\xd1\x15\xe9\xc5\xd7<\xfe\n0\x8e\x93\xe3\x99\x9b\x94\x93\x1\x84\xb1s\x90" 
  "\\\xa6tv\x14\xe3\x3t\xce\xba\xe4\xddV\xb2\aB\x9b\xc7\xeeK\xe9\x96q&\xa0" 
  "\x8c\xef\xf8\xe2%o\x8b\x1b\xee\xb8\xef\xc2\aO\x1a\xc9\x89\xcf\xe5\xc7%\t" 
  "\xf4\xf7\xcby^{\x87[{\xae\xd5s\x9c@\x1\xe4\x86\x9e\x32\xd2\xb7\x1e\v\x2" 
  "\x12\xee\x5\x84\xf7\xf3\x1d\xb6=Y\xe8Of=\x87\xcd\xad\xcf\xc4[.^3\x80\x46" 
  "\x91\xdf\x1fL/\xd7\x44\xaf\x98\xf8\x87\xd1_\xe4\x96W8\x1c<\xa0\x0\xe\x38" 
  "\t\x3\xf8p\x0\xe9\x39&\x16\xbfX\xc9\x84\x1e\x63\xb9\r\x1\a\x82\xef\x91\f" 
  "E\x1cH\xbd\xcc\x18\x80\x1\xe4\xa2`\xf6\"a6\x11\xfa\x42%\x15:\x19~\xd6\x2" 
  " \bZP@\x5(\xc0$\xd8\x12\xc3\x19\x1e\xa6\x81\x8ay\xe0\x61j\xb8\xa6\x1e" 
  "\xdc\v2z\xe0\x92$\x96\x84\x93K\xd0/\x12\xe\x18\x9c\x88Hd\x9e\xa3\x10\xd1" 
  "\x1\x1f:\x8a\x64>H\xc4\xc3uPD\xach\b\xf\xa2\xd8\'\x1\xcd\xc8H\x95\xeb" 
  "\xfe\xe2^\x6\xe7\xb5\xdb\xfd\xe4\x89\xd4\xab\x1f\x81\x6\x41# \xdd`I\xd7p" 
  "\x94\xe1\x8c\xf8;6\x1e\xc5K\x84\x3\x42\x1\x96\xd4\x0<\xfa\t\x1e\x45\x83@" 
  "3V\xb2\x15\xb2$\xe0&d\x2J!\x15%H\xc5\xfc\xa9$`\x82\x87\x1e\xf6\xd8%_\xfc" 
  "\xe9\x1aVB\x91\xda\xc8\x3,\x1c\x44RLv*\xd4#/\xf1I\xa2@\xa0",
  (unsigned char *) "\x90\x1\x38$l\xep(/\xfd\x31PY\x1a\xe4\x1\x6\x45\xcbHUB" 
  "\xe\xce!\x6\x1\xec J@\xea\xad\x96\xb7\xdc%\'uS@\x1c\xf4\x62\x42\t#\xc2" 
  "\x1f\xd0%\xab~\x8d\xaa\x99\?\x0G\x10\x98y\x13\x66\xee\xf0\x15QX\xa6\x31" 
  "\x63\xd5\x83\x64\x92JZ\?|Y+\x10\xd1\xad\x36\xb0L\x9c\x37\b\':\x89 \x87r" 
  "\xb2\x61[\x1cY\xa7\xf\xd4)\xcfz\xda\xf3\a\xf4\xbc\xa7>\xf7\xf9\x43<LR\r" 
  "\x0\xcb\xe6\xbe\x9e\x10P\xf1\x1cs\x9f#+\xa6\xa6\x0\x0\x89\fva\x10\v\x13" 
  "\xd7\x11\"*\xd1 P\x14>|\xd0\xe2\xfe\xb4&\xd1\x35\x33\x84\xab\x0\xb4\x89R" 
  "\x19~\xd4\x92$x\xcc\x65\xdf\x33\v\x8f\xac\x39R`Hs\f\xe1\n]2P:\x5\x95up/4" 
  "\xe5\x1\xc7l\xda\xb1g@\x82G\xed\xca)\x18Hv\x80\a\xc4S\x12lSZR\x89\x4\x37" 
  ")U\xe5)\xf2\xd1I\xd4\xbc\xd6\x14\xa5\xed\x64!I\x15\x89)\x80\xd6\x9f\xc9T" 
  "U\x12W\x5\x8aP\xe4\x36\x88\xd8\x30\x62\x1b\xc4j*Y\x91\xea\x93\xc0\xe5" 
  "\xcd\xadw\x83*+N\xe2\x11S\x84\xcd\x17\x2\xc1\x63\x19~\xf8\x38\x11\xcc!w" 
  "\xac\x84\x9d\x43{\xe7\x1a\xcch\xe6<\xae{\r`\x8f\x86\x8c\x90\x9c\fy\xd4!" 
  "\xect\xb4\xc6\xc2\xca.\x87\x33\xc8\x3,\xf0\xcc\xb7Ya\x18V\x18v)\xc9\x62" 
  "\x17\x18\x93\xae\xc2\x94\x64\xc6t\x0\x42\xe4\'\?\x15\x8eP!\xac\xd9\x90" 
  "\x8f\x94Q\xc8\xe4\x38Q\xaf\xe9\x43,I_K\xbdo\x91\a~\xd3\x91\x9f\x0\xff" 
  "\x30\\\xd8J\xc7\t\x6\xb0\x43ksP\x1a\x32\xc4T=W2\x9b\xd9\xfe\\k\x1e\x3" 
  "\xe9h\xb6\xff\xb1\xd5\x1\xc8\xf8\x1c\r\xceV2\x81P\x0<\x4\x82\xaa\xdf\xbc" 
  "\xb0\x82\x6\x91\x61\xfy\x18\x19\xeb\xc2\a*w\x89\x85v\xd2\xe9\xce\xa1\x16" 
  "p[9@\xc6\x17\xdf\xb1_\xf3\x1\x97\x39;\xc2I\x8c:t \x19\xa1\x31p8\x11\x86" 
  "\x46\xbb\x1b\xa3\xef^#\xbc\x85L",
  (unsigned char *) "\vrx\xfb\xb3\'\xba\x11\x0m\xb4\x63\x80{\x84\xa2\xfe\"" 
  "\xe2\x41\x95s\xee\xc4\x14\xb0\n\x81X\xd1\x92\x80\x32\xe2(u\x0\xa9P.JJ" 
  "\x8e\xd2\x4%\'\xc2Hw\xcc\x18\xc5O\v\xcaQ\"\xa2\nS\xb8XK\f\xe5S\x8bk\x9c" 
  "\'\xa0\xe2\x84\xc5\xb3\x8cT/-\x11\xcc\\\xe2\x32I0\xa6\x12N\xd8\xd4@\xc2" 
  "\xac\x18\x8a\x99\x81U\x17\xc2\xa5.\x81\xa4\xe6&\xde,\x15\x1d\xa6i\x10o!" 
  "\x1\xadGF\x98\xa8\x92\xa1Mg\xe\x3U\x4P\x15\x34\x8fv\x11\x35\xe4\x13\xa1[" 
  "8\xebK\xbf\x30,\x10\x99\xf6\x16w\xc6\xf3\x15\xb8\x32\xd0\x33T\xab\xbel\b" 
  "\xb4\xa0\x17)M/E3\xfa\xd1\xedr4\xa4\'\xed\rIS\xfa\xd2\x89\x46-\xa6\x37]" 
  "\xe9&\xd4\xe0\xd3\xa0\xe\xb5\xa8GM\xeaR\x9b\x9a\x4!\x0\x0;",
  NULL } ; /* --- end-of-image5[] --- */
/* ---
 * image6[] contains a 1954-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (6) Can't fopen("latex.tex") file:\\check permissions.\\
 * See mathtex.html\#message6}
 * --------------------------------------------------------- */
#define	IMAGETYPE6 1			/* 1=gif, 2=png */
#define	IMAGESIZE6 1954			/* #bytes in image6[] */
static	unsigned char *image6[] = {
  (unsigned char *) "GIF89a\xd9\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x1\x1\xff\x3\x3\xff==\xff\x16\x16" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xd9\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcf\x34\n\xdcx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8" 
  "\xa4N\xa4l:\x9f\xd0\xa8t\x8a\x64R\xaf\xd8\xacv\v\xb5r\xbf\xe0\xb0x\xea" 
  "\x1d\x9b\xcf\xe8\xf4\xad\xdc\x1b\x10x\x88\x87\xb0\xa0\x18\xdc\n\xc5\x2" 
  "\x62\xa8\xc0\x63\xdd\bxz7\b}j:\x81\x0\x83\x0qA\x3\x86\x8c\x90\x43l;\x5" 
  "\x6\x38\x3\f\n\f7\x3\v\?\x3\t\xf\n\x4\v\v\x97\x44\a\t8~\x95\t\n\x1X\x96" 
  "\x0\x6\xb1\xaaw\t\xa8=\xadH\xbc\x41\xb5\x0\xb7\x9d\x9f=\b\xaf\x0\xeo\x5" 
  "\xb9\x45\x94:\a{\x0\f\x9f\b\x1\xd1\vv<\x3\xe\xd1\x37\xcc\x44\x5\x9c\x37" 
  "\xba:\x6\x97\xbeQ\xd0\x8cr\xe1\x38\xe6\?\xe4G\xf1@\x8d\xed\x38\xd8=\xf" 
  "\xa8\x4\xd9\xef\x44\xce\x98V1\xb2\xd6\t\x13\x84\x1e\a\xc4\x61\x9a\aD\xc1" 
  "\r}=\xfc]\t\x85\xc3!\x0\x8b\xb4\x18>\xd4(\x4\x62\x11\x8b\x18\x1\f8\xc8" 
  "\x43\xa2;\x8e<\xfe\x0\x8e\x43\xc5\xe0\xc0\xa3\x37\x39\x62U\"\xa8\xc3!\x3" 
  "\x2\x4\xf6\x5X@@\xc1\x1\?\n\xf6\x30hp\xc0@\xc8\x8b\a\x8a\x2\xc8\xb9\xe9" 
  "\r\x81\x6\xa2\f$\xc8&\x8d\xe7\x82\x2\x4v\xf6\xfc\xb9\xf2\xce\x1e\xaa\x19" 
  "o\xdc\xcc)\x96\xa8\xd1\xaa\xa5\xc2%XE\xe0\x0\x31\x1c\x43\x8b\xda\xb4\xaa" 
  "\x96\xad[E_w\xc8\xac\x99\xd4\xdc\x37\x7f\xd3\xd2\x86]\xb2\x97\xc7\x1\x8b" 
  "\x6\x1a\xd8\xd9\x96\xc3\xe5\xeX",
  (unsigned char *) ">\xa6\x2X \x87\xd6\x81\xae\xe5\x34\xbeS\xd0\x0\a\x4" 
  "\x87\x6\x88\x15 \xf8\xb4\xd3\x65\x3\x97\a\x1f\x86\x87J2e\xcc\xa5\x45^" 
  "\xdeVHa\xe\x7f\xb1\a\xcc\xe6\xb6)\x88\xe3\x1d\xfe\x82_\xca}z\x9e\xca`" 
  "\x88\x5\x2\b\xa0p\xb5\x8e\xd1\xddX\xdd \x5\x1\x95p\xe0\x9a/%x+up09\t\xe+" 
  "\x80|\x1d\x39\xeb\x8a\x4\xaa\x63\xe\?\x1e\xf2\xd2\x64\x11[\x8bwO\x0\xbe" 
  "\xef\xa3\x98\xaf\xb3\x1f_\x98\xf0\xf\xe7\x4(\x17\x80.\xce=c\xdb:\x0\x1c" 
  "\xfeS\x9eI]\xe1\xf7\xce]\xe3\x14\x87\x83RJ\xdd\x66\x1d*\x5\x96\xd4\x1ah" 
  "\x17\"5\xf\x2\a\x90\x84\x9d\x87;\x80(\xe2\x7f\xf8\rv]\x85@\x1c\xb7\x80" 
  "\x38\b8\x80\x43\x0\x18\x35\x10\x1d&\xdc\xdcVHj\xd5]R\x9e\x85\xb4\x8c\xc8" 
  "\x80\x8c\x37@\xc0Ihw\x4\x80G}8\xc8Q\xde\x8b\xe7\xf9T\xe4;T2\xf9\x10#\x97" 
  "@P\xd9R0UI$\x0r \x90\xe5\x96>\xd8\xa8\x61~\xc3})\aNz\xf5\xa7\x43)p9\xf9V" 
  "\x8c>\x84\x32J)v\x80\xf8\xc8\x90\v(\xe0\x80\x3\xe3\xfdyT\x1\xe<\x0SE\x7f" 
  "\x1e\xa9\x65p\a\xf4\x94\xc0\xa1\x6h\xf9\xa2\x9f\x80R\xea\x10\x9c=X\xaa" 
  "\xe7&\xe|B\xa8\xa1\xb4H\xca\xc0P\x9f\x64uU\x82\x2}\xfa\x46\xa4\xf\xbcH" 
  "\xeaR;\xa1\x83\x3\x9d\x8f\t\xea\xa7x\x82\x86\xda*\'k\xb5\tDj\x84H\"\xcd" 
  "\x81\x95\xd4\x81\b\x12\x37\xee\x2\xa4\xac\xb2>CD\xb2\x8c\xe4\xd0\xac\xf" 
  "\xd1\xfeM\xfb\xc3\xa8MX\xbb\x9c\x9bo\x12;\xabr\x87\x9c\xa7\x4\x1\xde\x86" 
  "\xfb,\xb8X\x1cw\x3\?\xf9@knE\xe6\xa4H\x4\xbb\xef\x1e\xf1\x80\xbbR\xa8{\a" 
  "\x1c\xf8\xd6+\x85\xb6\xfe\"\xd2o\x14\xfa\x6l\xf0\xc1i\x14\x8c\xf0\xc2\fo" 
  "\xa1p$\x0#\"l\xc3\x14/\xfc\xf0\x32(\xf1\x80\x31\x1a\xa2\b\xd1q\xc5\x16s" 
  "\xeb\x3\x83\x42\x90\f\x6\xbd@\xa0",
  (unsigned char *) "\f\xb2\xc1\xf{G\x84\xc9+\xc7\xac\xc6q\xa1\x84v$O>\xf9" 
  "\x11\xd8\xa9\x35\x43\xf9N[\xf1<\x15\x95\x64\x62\xd1\xb5\rOJ>\x85\xb3K" 
  "\xe3q\xe5\xe7%bR\x87%uO\xafk\xd4M\xb0.\x8dG\xd4\xe9\xc9\x9c\x85J\xa3\xe1" 
  "\xf1\xc0*\xa8uE\\\xd8`\x92}I\x9f%\x89\x46Zg\xb2\xd1\"\xe3\xbd\x96\x45" 
  "\x88&-09\x99wX\x9cy\x6ZjU^\xe9\xf5\x15*=\x0\xec\xdd\xfb\xc1\x62x9\vd\b" 
  "\xe4\r\a\x80\x37_,&-\xd8\xe1\x0\x44\xd1\x8dy\xe4{\xbc\xb3\x9d;\xfejw\xb5" 
  "\x39\xdd\x83S\xa1R\xd9\x8f\?H\xe\xea\xa0\xfb\t\xad\x44\x14\xc6Sy\x87*" 
  "\x9e\x13\xe\x4\x97\x15p\xbb\x84\x10\xd6}\x9d\xee\f\xe0^\xba\xe9\"+B\xd3" 
  "\xaa\x1dZ\t&t\xeb\x86\xf5\xc0\x89\xee\xb8\xbd\xa4\x9a.[\xde\x95\x1~\xc8" 
  "\x88\xfd\r\xda_2\xa4gGv\xb8=2\xeb\x1e:\xbc\x12\xc7\xb5\xa4\xcf#\xb6\xe6" 
  "\xca*\x94\xea\x1b\xc0\xbex\x1$\xe0\vj\x8e\x42**\xa5\xe2\xf8Ti\xfb\xff\x3" 
  "Th\xdc \'\x2ZJW\x9b\x1\xa0\x2\x6H\x80\xca\xf4\xea|M\xd0\xd7\xc0\xf6\x95" 
  "\x83\t\x9e\x44\x11\x1a{\xc2\x1e\xba\xb1\x41\x1fh\xab\x83\x10\xccW\xf1" 
  "\xc4\x0\xb3\x10\x9ap\x12#\x4\xc3\x2\xcfr\xc2\x16\x36#\x85.\x8c!\xcb`(" 
  "\xc3\x1a\x9aK]t\xb0\x43\xc4\xa8P\x88\x1d\n\xa1\x87N\x0\xa2\xc0\ba\xc3" 
  "\x1e\xa4\x8f\x13\x96\x38\x1c\x17\xfc\xb0\xb1J\xf4\xe2\x1bN\x9c\x3\x14" 
  "\xef\xb1$\xf3\xd5+\x13\xbd!C\xf1\n\x0,\x10\x91\xf0\x82\xc0I\xfe\x2\xcc" 
  "\x32\xf6\xb8\x1c\b\x4\x2\x16\x1c\xc3\x34\x6\x92\x46\x14\xfa\x80\x33\xdd " 
  "c\x14<\xe2\xb2\x8d\x88\x91!t\xfc\x45<f\x1\x0\xe8\x99\xab\x1a\xd1\x0K\x17" 
  "\x46x\x80\x6\xb0\xb0h\x82\xd9\x99/\x94\xb6\x95G\xe4L,8a\x89Y@\xa3\x35iL" 
  "\x12-Wa@\xaf\xda\xf2\x96qT\xd2\x92ra\xa4O\x1c\xc9\x95:>`",
  (unsigned char *) "1[\x12%\xd3\x1e\x89I=\x18\xa5k\\#\xc9Xt1\r\x9b\xad\x42" 
  "\x91-y\t\xe1Rh\x0\a\x88\xc0)p\xd3\rq\x80\x3\x38\xc0\xb5\xc6\xe\xaf\xf1" 
  "\xe\xea\xca\x33L\xda\x64\xf1\x36\xc6\x4\x63\xdd\xa6\xf9;s\xa0\xc6\x1c" 
  "\x45\x61\x94\xdd\xc2\x42\x1c\x3\xec\xcd\x9b\x82sMe\b\x2\xa5n*F$_\"\x18\r" 
  "\xe3`\x8d\xc4\xd5or#\xaa\x1dz\xd4\xa3L\xda\xf9\xc3\x9d\xeb\xb2Of0S\xcf" 
  "\xbb\xb9\f\x1$A\x97\xf5\xdc\x39\xba\xaf\x64\xae\x1b\xd4\x41\x85L\f \xb9" 
  "\xf6\fH@\xe5J\x82J\xc6\x93\x3@\xb1(\x18(\xb1^\xfe\x82\x38\xd4 y\xf2\xed" 
  "\xa2\x84\b\xd1\x99\xfa\xc9Q\x8f\xbe\xa3\x10\n\xd0\xc7x^AQ\x93^\xe2\xa2" 
  "\xc0\x13^LS\xa3 T\xf0\x84,\x18\xcd\x41\x80\x66$\xc7\x7f\x14o\x81\x15\x45" 
  "\x80\xf2\fE\xbd}\xcaSJ}\xa4\x92\x8f\\\x8a\x37\xea\x89\xa9\x8f\x95\x61" 
  "\x93G\xc3\xe2#{v\b\xa5\xe2\xa1L{\xfc\xf9\x33\x35\x8dO{\xd9\x43\xca\x94" 
  "\xbc\xf7&5\xd1j9\xf2\x92\xe8Os1\x1e#\x8d\x43T\xba\x82R\xe\xfc\x17\xa8\x0" 
  "\x6\x85i|RD\xa1z\xa2@\xbd\x82\xea}\xa3j@\xa9\x62\xd5+K\x1d\xd0\xaf[\xb1" 
  "\xab\x61sU\xa4\xc6\xce\xf5\x0\x8au\x9f\xa8\x18X\xc0\x6\x32\x2\xaf\x9d" 
  "\xea\x63R\xdc\xd2\x39\xb8\x32@N\xc4\xa3\xd6\x1e\xc6s#_\xf8\xb0\x44p\bB" 
  "\xb5\xa0\xd0\xc6\x1c\f\xa0\x32~\x1c\x82\x1f@\x18\xad\x1b\x45g\x1,\r\x90" 
  "\x88x!D-\x16q\f\xa7\x14\t\x99\xc2\xc0\x13\x1c`J\f-\xfb\xad\x13@rZ)\xb4" 
  "\xa7\xa5\x63H\xaer&\xa7\xab\x5\xe9R\xf7\xba\xbe\xc5\xaev\xcd`\xdd\xedz" 
  "\xb7\n4\xfc\xaex\xd1W\x83\xf2\x9a\xf7\xbc\xe8M\xafz\xd7[\x83\x10\x0\x0;",
  NULL } ; /* --- end-of-image6[] --- */
/* ---
 * image7[] contains a 2110-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (7) Can't run latex program:\\check -DLATEX=$\backslash$
 * "path$\backslash$", etc.\\See mathtex.html\#message7}
 * --------------------------------------------------------- */
#define	IMAGETYPE7 1			/* 1=gif, 2=png */
#define	IMAGESIZE7 2110			/* #bytes in image7[] */
static	unsigned char *image7[] = {
  (unsigned char *) "GIF89a\xd9\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x16\x16\xff==\xff\x1\x1\xff\x3\x3" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xd9\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcf\x34\n\xdcx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8" 
  "\xa4N\xa4l:\x9f\xd0\xa8t\x8a\x64R\xaf\xd8\xacv\v\xb5r\xbf\xe0\xb0x\xea" 
  "\x1d\x9b\xcf\xe8\xf4\xad\xec\x1b\x10x\bG\xb0\xa0\x18\xdc\n\xc5\x2\"\x8cP" 
  "\xe0\xd5\x80Ml<\x5\x6\x37\a\r\x6\x8a\f\x0\x3\vm\t\xe\n\x4\v\v\x86\x44\a" 
  "\t8\x7fY\x5\t\x97=\x9c\x81\xa3K\x1\x41\a{\x0\a\x8a\x6\t\n7\vv;\x3\x10" 
  "\xa9\x37\x9fy\x8c\x37\xa0Y\x8a\?\xbc\xa4\xc1\x83:\x3\x9a\x0\x5\xb2q8\x3" 
  "\r<\a\xba\xcb\xc0\x41\xaf\x0\xe\xd2T\xbe=\xd6\xc1\xdck\xa6\xbf\xd2\xc0" 
  "\xdf\x39\x5\x1\xb6\x38\xaf\f\x4\x4\x97\x4\x1\v\x4\n\a\x7f\n{\f\xf\xab" 
  "\xd4\x39\xb4\xf0\x1\xc8\x10\x86h\x19r\aO\x9e\xa8]\x97\xd4\xb1\xbbq/\x1f" 
  "\x0\x6\xf0\x16\x14`\x90@\x13\x81\x3\x8fr\x10x\x10\xa9\x95\x1d~\xee\xf0" 
  "\xb0S\xa0\x8e\xa1\xa5\x5\x14\xfe\x41\xfa{\xb8\xae\xdd\x83\x82\a\x6(0\x88" 
  "P\xca\xb0\x1c\a\xf4\x31\xd4\xa9J\x16\xbaq;\x12\xd8Y \a\x80\x81\x3\x35ud" 
  "\xe3\x61\x0\x42\xb5=\xd9\xa2\"5\x1a\xee\x92P\x0\x44\x93n\xbc\x31\x0)\xad" 
  ">\xd0r\x18\xc8X\xe\xaaS\a}\x1e\xe0h\xf0\xca\x1cV]M\x9f\x2\xb8\x9a\xd5" 
  "\xe8\xd4\xa3I\x97>\xb9\x89#\xa7\xe\xb5:\xfc\x92s\xab\x83\x1e\x81\x44y" 
  "\xab\xfa\xd0\x1b\xf5R\xe3\x1d",
  (unsigned char *) "K\'!\xa6z+\xe7\xcco\x4 \xbc\x61\n\xea\x80\x9c\xa5\t2" 
  "\x1a\xd5\xf4\xcd@Q\xbd\x0$;^\x9d\x94\n\xdf\x43:\xbb\xee\x10\x8c\x33,\x0" 
  "\x65\xae(\xeb\x46M\x99g\xeb\xdd\xacu\x8b\xb5\xfa\xea\xb1<i\b\x10\xf5X" 
  "\xbaJ8F\x1cx\xe1-\xfc\x9d\xfb\x31p,\xaf\xdf\xe6p05\xc7\x83s\\k\x89\xed" 
  "\x33\x35\x91!\xeb\xc3\x8drN\xf`2z\xe8\x86\xe4\xdd\x30O\xd9@f\x1cr\x10" 
  "\x18jPT)\xd9\x95K1\xe0\xd4|\x8c\xd8&\x9c|\xed\xf9\xfe\x62]6\xeb\xd8\x4" 
  "\x14\xf\x94\x88\xd5\xdd\r\b\f8K$\x93\xc4r[L$A\xb0\x80\x2\x10@0S\x88<\x15" 
  "\x0\x81\x3\x9b\xe5\x0\"\x4\xd0x\"S\x2\"\xae\x38\xa2\x88*\x92\x98\x9cL\x2" 
  ">b\"\x8a\x46\xf1\x87\xd2=\x8f\x10t\xd0Q\xf1$\xf0\xc6\x8a\xd0\x18\xc0_6\r" 
  "\x1c\xe0\xe4\x2}\x84\xa8\xcb\x8d\x1d~x@\x8c$\xca\x38\x97\x31]<\xe8L\xe\b" 
  "\x1c\xc4\x80\x81\xe4\xd4\xa1\x3xC\xa0\xf9\xc3\x1ej\n\xa1\xe6\x39\a\x81s" 
  "\xcc\x1c\x38,\xe0\xca$\t4\x3\a \xd9\x1@\x0\x99\x14r\xd9\xcd\xa0\xbc\x5" 
  "\x1\xf\xe\x11\xe\n@\x9f~\xfa\xa4\x3Z\x8a\xe\xaa\x80\"\xbeM3\xd3L\x91." 
  "\xea\x65(p\xb4\x99\xe9\xa7\xa0\x6\xc1h\xa8\xa4\x96*\xc8\xa6\xa6\xa6\xaaj" 
  "\x15\xa8\xf6\xd0G\x9c@\xbc*F\x8a\x66\f\xd0\x1f\x85x\xe8\xd1\x84\xac\xfb" 
  "\xdcz[\xae\x9e\xba\xd6*!\xb8\xe4Q\xec\x34\x6\x4\xf0\xc9\x2\x18\xa5\xa2" 
  "\xc0\x2\xca\x2\xbaUj\xf\xfe(r@\xb5G\x19\xf0l\x0\xe6\x19Pmj\xd0\xce\x93Z" 
  "\xb4\x42\bj\x94)\x99 \xf1\x87\'\xd2\x98\x9b\xac*\xe6\xe\x1+\x10\xa3\xc2w" 
  "D\xa1>\x4\xa0\xf\x88\xb6\xe8\xbbX\x0o\x10 \v^\xb7\x15\xe5/\x85\xa2\x39" 
  "\xf0\xdd\x31\x80\xf2\xe0\x80\xa3\xcaL\x84\x4(\xbc=\f\xa6\x1c\x12\xdf{D" 
  "\xbd\xbf\r\x81o\xf\a\xcf\aJ\xc8g2\xd0\x80&\x14w",
  (unsigned char *) "w\xc9\xc1\xe9\xe4P\x91zD \xa0\':7T\n\xc4\x36\x1d\xdf" 
  "\x36s\xcd<\x1b\x81s\x11\xd9\x15\x33\x16#c\xc5#\xee\x43\x11\xe1!4J\x94]t" 
  "\xcd\x12:M\xbb\xa8\xcd\f 0\x0\x61\xbbL\xb8\xc6+\x3 \x17\x0*F@\xb9\xc9" 
  "\x1e>m\xd4\xd1U,M\xd7\x90\xb6\x46\xc1\xc4\x89\xd8w\x90-\x4\x44\x94L\x84" 
  "\xf\xdbK7\xac\xe9\xf\xe5\xe0\xe1\x0\xcaw\r\x4XW}W\x83\xb2!\x1f\nA\xb2\x2" 
  "\xe3\x90,\xd6\r\xf\xdcJp\xe\xdc\xb6r\x8d\x3\x1\x38Z\b+\x9c\x1b\xf0\xc7" 
  "\x9f\xfe@\x8cu\x87[t\x9d\x96rk\xa0\x1f\x31\xadl\xd9\x14\xfe\xf7\xf\xafq" 
  "\xa7\x14k\tX\xc6\xb8\xec\x62\x31[\xa9L\x98n\xad\"`So\xf2\x87\xd0GYh\xd7" 
  "\xe\a\x1bR\x0\'\x5\x38\xb0\x80xE\xec\xbc\xde!E\xa9\xd6\xda{\xd2\xfQ\xfb" 
  "\xa5\xa6\x64\x83\xbb\xa8\xc3N\x9e\x98*\xbc\x88\?\x1a\xbf;0\xc0\n\xe$[" 
  "\xc2\xbe>\x2\xef\xb2I\xe6\xd0i\x1d<\xeUCw\x83\x87\x38\xf4q\xe9\xff\xf5" 
  "\x80\xce\xbcZ\xd3\x9c\xea\xb0\xa6\x38\xc1\xd9\xc5\x0}\xd0\x9c\xf4L\xca~ " 
  "\x1bVYn\xf0\x86\xc7\xdc\xe7\x6\xe\x98\xa0\x9f(\xe3\x80\xec\xf1 d]\xe9" 
  "\x97>\x8c\xa1\x1f\x97\x89\xc6|\xf7\x9b\xf~RQ\x8e\xcd\xf8\xf\x80\xbd\xbb" 
  "\x83h\x98\xf2\x1f:\x94GA\xe7\xb9\xce&f\xb8\x83\xf8\xe9\xe0\x82\xd5\xa8" 
  "\x8f\x6\x3\x46+\xf6\r\xeb!\x9e\x31\x80L\xb2\x44\xa2\x1e\x39\xaf@I\\\xa2" 
  "\x65\x94\x35\xc0mu\vnVT\x4\x4L!\xf\b\xd0\x63\x8b\xdaj\xfeJ\x0\xd8\x6\xae" 
  "\xca)\x2\x1fHd\xd9\x3\xc6\xb8@\x1dx\xb0~Ez\x3\x95rt\x8c\x13\xc5\x83\x89" 
  "\x34Z\xcb\xfj\xc7\x14\x1f\x31\x62Go`@\x14\xb7\xf4\xc1#RH\b\a\t\xd6\xaa" 
  "\x8eP\x92\xe5(\xefL{\nB#{`\xb3\x39\x81\xe9\x92\xf4\x32\xe4\"\x83\xf1\x46" 
  "\xe1H\xa1\x93}\xd0\x2\xc7\x36\xa9\x86\x6\xa8i",
  (unsigned char *) "Rd\xfc\x64\x9b\x1cu\x85Q\x92\x12\r\bP$\x15\x62\x89\x6W" 
  "\xbe\xf2\x96\xa0\xea\x13\x1d\xec\xd0\x46,\xf0\n\n\xbf\x44\x42\x30)\x94\n" 
  "Y\xe2\x92rG\x1c\xd3\x31P\xa8\x85u\x1d\x8b\x1c\xeaz\xe6&\xe4%M\xac\x88" 
  "\xa4\x88\xa3@\x4+\xf4\xb6\x31\tv\'9b\xa0\xd8\x35\x9e\x16\x84\x42\x91\x13" 
  "\x32\xed\x9a\x8f\x31\xc7\xb0\nE\xe4\xc6\x41\?P\xc0\xc2\xe4\a\x86\x9fU" 
  "\xec\x9crz\x14>\xd9\xb3\x89Kt2\r\xc8\xa0\x90\xaf\xf6r\xc4k\xa5\x12iuC" 
  "\xa8\x44~\xf8\x12\xa3\xf1\xeeh\nI\xc8\xdd\x8a\xe3\xb6\x87LT\xa1\x13y\xd9" 
  "\xfe\x45xX4\x9a\xacm\x12\r\x95\xc7\x43\xff\x80\x1a\x8b\xd9\nQ!\xe5\x10M0" 
  "\xaa\x1f\xc9\xdc\x46[\x87\x61HK\xf0w\x12\x8a`\xf4q\xc2*\xe7\x16\x1\xe6" 
  "\xa7\xc1\x1d`u\xf6#\x98P\xad\x32\x14\xd3\xd5o|=\xe5\x8aWjA\x12\xc8\x4" 
  "\xeezwy*e6\x97\xad\xaa:&\xaa\x35\x1\xaa}0h\x94\xcd\x14\xa5tk\xd8\x3\xd3" 
  "\x80\x8a\xbfJ*aTq0\xc7\xf6.\xb3V\xc6\xa1\x13\xa9\xa9\x39L\x2\x17\x64\x15" 
  "\xdb\x61\x46\x33\xd3\x43\xf]\x7f#\xb3[\xccn|m\xcd\x1c> 5\x80\xc1\xda\xc2" 
  "z\x8b\xda\x85\x1c\x2\x8b\x3\xe0\x91\x61X1\x4\x80\x88\x1ax\x88s\xeeu.\b" 
  "\xacIf\xafs\x9c\x1d$\xc7\x83{\xdd\xec^\xfdg\x8d\x99\xdc\x89\x1a{\xa5\xec" 
  "\x31&\xd2\xa4\xd5\x9al*\x6\x84\xc5L\xc9\x37\v\b\x12\x34\x9e\xbc\xa8\x5" 
  "\x10Qd\xa1\x81\x86\xf6\x86\xe7\xc9\xe1^\xed\xd3\xdb\x97\xb6\xa7(\r\xd2" 
  "\xa1\x82\x94\xcb\x1a\xffY\x86(\x97\x42\xaa/\xc2v\xeb\xb9\xfd\x19\xe5\xfN" 
  "A\x10}l\xb3[\f\xda\xd6\t\xafQ\xc0\'f\xd2\x0\xb8\xf8\xd1\x89LS\xd1\x95" 
  "\x66\xc4\xde\x39\xf2\xf\x90\x32\xd2\x12 \xd1;\xa6\x97\xf8\xe9\x1d\x9e" 
  "\xd0\x44|\x9b\b\xdf\xf5\xee\x17Kyl\x8f\n\xd5\v`\xf6\xd2w,n\xf8L,\b\x90" 
  "\x1f\xe\xd1\xb1IO\x82",
  (unsigned char *) "\xcay\x8f\xd7\xca\x61\xd1r&\xe0\x39H/}\xf0\xa6 \xc0" 
  "\xe9\t\xeb$FQ\xfe\xc9\xb7\x43\xda\x82M\x90\xac\xd3\x9d\b\x90\xa7i\xf6o" 
  "\xc3\xdd<\xe6(L:\xd0/\x1c\x8a\x82<\xfc\x82-e<\x5j\xf8\x1\r\x0<\xc3\x8eyL" 
  "d>i\xb2\xc8H&\xc5\x90\x93\xcc\x64\x30,\xb9\xc9P\xce\xc2\x93\xa3Le\a\xd5" 
  "\xe0\xcaX\xce\xb2\x96\xb7\xcc\xe5.\xd3 \x4\x0;",
  NULL } ; /* --- end-of-image7[] --- */
/* ---
 * image8[] contains a 1896-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (8) latex ran but failed:\\check your input expression.\\
 * See mathtex.html\#message8}
 * --------------------------------------------------------- */
#define	IMAGETYPE8 1			/* 1=gif, 2=png */
#define	IMAGESIZE8 1896			/* #bytes in image8[] */
static	unsigned char *image8[] = {
  (unsigned char *) "GIF89a\xc6\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x16\x16\xff\x1\x1\xff==\xff\x3\x3" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xc6\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcf)`\xdfx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f:\x11r\xc9" 
  "l:\x9f\xd0(O)\xadZ\xaf\xd8,\x90\xaa\xedz\xbf`!7L.\x9b\xaf\x63\xde\x80" 
  "\xb0\x43<z\bE\xe1,\x8c\xcf\x9fr\x9f\xfd\xb7\xa7\x3\xd2:\x5\x6\x36\b\x4\n" 
  "\fw\x3\v<\x5\t\x83=wt\x8d\x8f\x41\x91\x39\x8d\n\x1;\to\x93\?\x9e~\x80" 
  "\x39\a\b6\r7\a7\v\x3<\x6\x94;\xafg\xae\x43\xb1\x37\xb3\x96\x37\x4\xac\x0" 
  "\xb3\?\xbdt\xa2\x37\x3\t7\xe\xa5\x0\xa9\x36\x3\xa7\xb0\xb5\x37\xf\xce" 
  "\x64\xbf\?\xd0\xad\xd1\x39\xd3\xd6~\x7f\x9a=\xbf\r\x1\f\xf\f8\xdd:\xbd\f" 
  "\x4\x4\x8f\f\xe\a\x6\n\x0\f\v\x4\v\x5\f\t\xc4\x4\a\x8b\x39\x3\x10\xf4\x2" 
  "\xccI\xb7\xce\x6\x81\x0\xf4\x14\x1c\xc0\xc5+\x81\xbaU\x0\xfe\r\x92(\xcf" 
  "\x1d\xbc\x1c\n\xdf\xc9S\xf7H\x1\x84G\xe8\xe8\xd9\x8b\xb8\xe0\x90#\x1b" 
  "\xd9\xfe\xbe\x4\xb3q \xde\r\b\x1 X:\xb0\v\xdb\xa3\x4\xac\x16\xbc\xe1\xf5" 
  "\x88\x80\x3\x65\xa9\xfe\xc5!\a\v\x2\x80\a\xa5p\x2\xd0\x89\x32YJ\x3\xfd\n" 
  "\x4(\xd5\xabj\xb4^J\x99\xf2\x44\x39\xc8\'P\x4SY\x82\xbc\x96\x65%2\x97\b" 
  "\x1a h$\x10\x95K\x9b\x37\x14\x10h0\xd6\x46\x82\x96\n2\x19\x84\xc0\x46" 
  "\x1b\xe\xb9t\xb9\n\x86\xcb\xf2\x8d\xd5\xc1\xe7(\x1\xae\xbb\xf5n\xdeL",
  (unsigned char *) "\xf\x92!#\xeb\xc5lK\x1b\xf\xfa\x2\x38\xc9\xf2-\xe\xac" 
  "\xf1\xe+\xac\x85\xe0\x0\xb3\x66\x38\x12\x84\x66\xfc\x94\x92\xc6\xc3[=\xf" 
  "V\xbd\xb5\xf6,\x8d\xb6$\xe3\x6\x66n\xc7\x2\xa2\x99m\x11\x5`\f\xb5\x42S" 
  "\xb3\x92\x13\x30\x8a\x19\x0\x82\x41\rv&\x8e\x9b\x8c\xee \xd8\x9f\xfb\x81" 
  "\x9d\xd3+\xf0\xd6Z\xb3\x8e\x3\xb0n\xbb+\xf3\xa3\x5~\x8a\x35\xa8\x39\x8c" 
  "\xd9z\xaa\xe2)\xe8\xe7\xfc\xfc_\b\x10\x14\x94\x1ep\b \x80\x2\x10\x4g@t" 
  "\xbf\xb5\xb3\xc8\x41#\xdd\xfe\a\x1Q\xfb\xf5W\x12~yA\x88\xcd\x3\n,\xd3W#" 
  "\xfc%\x90\xdf\x7f\x1\xb6\a\x80G\v6\xc8\x0@ F\xb8\xe1\x80\xf\xfc&OI>]\x96" 
  "\x8f\x19\x66!\x83\x43\x1y\xdc\xc0\xc0pz\xb4\x31\x63\x13\xc7\x18\xc1P)=" 
  "\x12\x2\x44\x90\x9f\xe4\x0\xe4\x36\xdc\xfc@\x0\x8e\x46\x12\x83\xe4\x93PF" 
  "\x11#\x0\xba\xf0\x80T\x94Xfy\xc4\x94\xff\xb5\x41\xa4\x96`\x86\x99\x44ob" 
  "\x96i\xe6\x12\\\x9e\xa9\xe6\x9a\x37\xa4\xe9\\\x8d\x43\xf4\xc1\xe6\x9ch" 
  "\x90\xe9\x3(C\xe0\x19\x6\'Q0\x84$\x9fV\xb8Y\x1b\x11)yQ%\x14\x94\x99qh" 
  "\x15\x82\x16\xeaK\xa2\x66VC\'\x9av\xe2\x30\fT\xe4@e\xc8\x42\x36\xccS\xcf" 
  "\x1c\x97\xaa\x38\xcb>\xb1\xe0\xc3L\x2\r\x14\xb0\xce!lP\xe4*@\aY\xe2QW\bm" 
  ":\x87O\x9c\x18\xa0\xd4\xab\x83\xb4\xf3\x8el+~\x8a\x8f>\xfc\xe0\n\xcd\xae" 
  "\xb0\xb6\xe5\xa9=\xcf\x1\xe6\x1c<s}\xf8\x91\x41\xf0\xfe\xa4\x43\x65\xad" 
  "\n\xf9\xd9\xc3JR\xcd\xf1\x0\x31\x6\x38\xd5\x95z\x3,\xd4\xd6\xb7[\x95\xb4" 
  "\x3\x3\xcc\x18\x10\x87z\xe3\xad\x36\x98\x1\x46]\xf9\x19H\xe2\xa2\x14UX" 
  "\xb0\x15\xeaUDAA0\x94\xbe\x36HEU\xbd\x85\x90\xfbN_\x86\x31,\x98\x2\xf0" 
  "\x36\x10Z\xbe\x43\xac\x14\x19\x61\xb3\x38\x96W\x0\x17gwY\x1b\xc5\xbd\x91" 
  "\x0}\xba\x96\x87\x18\xc6\xac\xb9",
  (unsigned char *) "\x66X\xca\x9b\xe0\xa5\x17\x95|\xd9\xa2\xf2\xa0\x1ag2" 
  "\x80;W\xde|\x80\xbd\x19\x93\f.cbT\xda\x14\xca\x93}&\x19J\xaa\t\xcc\xc3" 
  "\x2\x6\x30P\n\?\xb9\x99<(\xd1R\x17\xdd\xef \xb2\xedvCi\xa7\xfd\xf2\x9ak" 
  "\xaf\x14pO\x3\xa9\x88\xcdn\xbeP\xf\x8d]\x10\xdc\x86\x45\xa5\xc9\xcb=c" 
  "\xb0\x41[=p\xda%\xe\xb0s^\x3\x99>\xe2\x9d\xa3W\x13\xfc_[\xdd\xd5\x15K" 
  "\xdc\xcd=7\xdeNP\x15L8%\x88\x1f\x65\xc0\x1d\x46Mn\x3s\xb3\x8cx\x3\xdfR" 
  "\xab\xb3\x85\xd0\xfe\x9d\xeel\x0\x7f\x10\x96\xc8\v\x81\xe4\x30 :\xe9x\x5" 
  "\x90\x80\x9fJ\xa1\x14]/\x18*\xa0\xa1\x89LJ\x9b_\x89\xa6\x87k\x88\x43\x5" 
  "\xe3\x64\xfb\x86\x0\x6\x87\r\xea\x6^k\x8f\xef\xb6\xb3\x1\xe2p(\xfe\x6" 
  "\xd5\x1a\x86\xad\x42\xc0\x1b\xbd\xcf\x8eu\xe9\x12\xbe\xf8\xc3\x94_\xee" 
  "\x80K\xf8>Hw\xc3\x8f\xce=q\x8b\xeG\xe2@~\x97\xde\f\xa2\xed\x8e\xe9\xf7" 
  "\xd8>#P\bz\x5\'\xfc\x95\xe1h\x15\xff\x3\x93\xfe\xac\xb0\x6`uA\x1\xae\x30" 
  "\xa0\x14\x10x\x91\x35\rpR\x10\x44\xd2\x3#HA\x18\x81\xae`\x15\x82\x1f\x19" 
  "\xe4\xe4\x4\xe\x1a\xc1\x83\xce\x39\xc6\xfb\xd4\x64\x96\x1b\xfd\'\\a\xb8" 
  "\x83\x9e\xcew\x84\x15\x16L\b.\x1c\x89\xe7\xb2T\x88\x43\xcc\xcf\x6\xdc" 
  "\x92Li\xc2@\x89\x42\x41\n5\xe7\xa0\x45-\x9c\xa4\x16-\x9d\xe6h;X\t\xc4z" 
  "\xf4\x43\'Hjj\x98i\"ap\xf0\x44 dC\x10\xa6\x0Sqd\xe4\x3\xcb\xe4\xcd\x33" 
  "\xcb\x1a\xfe\x88Hp\xe1\x93\x84\xd0$/\x9c\xdaHA*\xf2+^\x98q \x16\x89G\x18" 
  "\x87\x45\xa5\xb4\xd9\xe2\x8dl\x84\x96\x3\xcc\xc8\x9fl\x9d\xec(\xac\x18" 
  "\x80t\xca\xb8\xa9>\xa6\x31\x8c\xcd\x8aV\"\x99\x41\x10J\xcc\x3S\xc4\b\xe3" 
  "x\xc2\x31\x8e\xef]\x90\x17\x30\t\x0\x1b\xfeU.N\"\x91\x17N\xa1XV\x18\xd7" 
  "\x43\x8a\xf5\xc2",
  (unsigned char *) "\x93\x11\x11\xd8!\xce\x61\xcaR\xe\xcd\x64\x82p\xc5;" 
  "\xc2\xe5\n|\xbdrT\n3\x80\xc3t\xd9\x9c\xcd\xe4\x64\'aQ\x11*\x1\x0\x13\x99" 
  "X2\bn\x98J\xcd\\\xe7\x32;\x5\xce\x6\x8b\x41\xcc\x33\x97\xb9\x17\xf\xcd\v" 
  "h\xcf\x9c\x46Z\xec\x42\xb5\x8c\x35Sg9\xc3Y\x8f\xa2\xf9\a\x94\x88\xac\x99" 
  "iYK\x2\xda\xb2-\xd0\xe5\x5\a\xf9\xd1\x9a\xd6\xa6#5\xda\x88\x46j\xe1\x99" 
  "\xa7sL\x3\xc4{fs\x10qP\x0\x34\xf2\xa2\x9aw\xe2s\x10Z3\x1b\xd9\xfe\x33" 
  "\xb6\x64\xd8\xf3\x11\xf4X\xa3\xd6\x8c\xb7\x19\xb2(\xf1\x15\x2\x8b\\\xeb" 
  "\x66\xce\x63\xbek>\xac:\xc9\xb9\xce\x41y\xa1\xd1g-\x8e=\x1e\xfd\xceH\xb7" 
  "\x12P\xbc\xe8\xe4\x31)\x1d\x15G-GL^P\xee\x43 \xedU\xe\x34J\xd1\xa6u\xd1" 
  "\x9d\x8e\xc8\v\xe7N\x97\xa2L\xa1\xe#\a\xd8\x1d\xf7\xf2#\"\xff\x14\xcf" 
  "\x10K\x8d\xc7S\x89Z\xa0=*\xaf\x11\xc4\xe0\x9d\x84\x9e\xaa\x10\xa5z\xd5" 
  "\x44oa\xc6\xddp\xfa\xd5\xec\x15uz\xd7s#\xf5\xf6\xc9\x9f\x11-\x82l\a\xe0" 
  "\aU\x8e\xaa\xae\xf9\x1c\x93\a\b(E^\x88\x84\x8b\x1b\xf2\x41G\x7f\x65!\x8f" 
  "\x8e \xc8,\xb6PH\xf6K\x9f\xfbp\xb0\x80\x82\x12\x0U\x82\xc5\xa0_\x93TA(= " 
  "\x90\x1d\xfd\x2=rA\x1f$L\xb0\xb2Up\t\x9c\xc8\xf0\x18\x83\x32\xe1\xb3\xa0" 
  "M-\x16P\xab\xda\xd6J\xe9\x92\xae\x8d\xad\x16X+\xdb\xda\x1a\x81\xb6\xb6" 
  "\xcd-\xdbh\xc0\xdb\xde\xfa\xf6\xb7\xc0\r\xaepU\x10\x2\x0;",
  NULL } ; /* --- end-of-image8[] --- */
/* ---
 * image9[] contains a 2215-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (9) Can't run dvipng program:\\check -DDVIPNG=$\backslash$
 * "path$\backslash$", etc.\\See mathtex.html\#message9}
 * --------------------------------------------------------- */
#define	IMAGETYPE9 1			/* 1=gif, 2=png */
#define	IMAGESIZE9 2215			/* #bytes in image9[] */
static	unsigned char *image9[] = {
  (unsigned char *) "GIF89a\xe1\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff==\xff\x1\x1\xff\x3\x3\xff\x16\x16" 
  "\xff[[\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4\x1" 
  "\x0\x0\x0\x0,\x0\x0\x0\x0\xe1\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcf\x34\f\xdcx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8" 
  "\xa4P\xa4l:\x9f\xd0\xa8t*eR\xaf\xd8\xacv\xab\xb5r\xbf\xe0\xb0\xf8\xeb" 
  "\x1d\x9b\xcf\xe8\xf4\xaf\xdc\x1b\x10x\x88\x86\xb0\xa0\x18\xdc\n\xc5\x2" 
  "\xc2\x8cP\xe0\xd5\x80U\x1@\x5\x6\x37q\n\r{\x0\x3\v\?\x3\t\r\n\x4\v\v\x86" 
  "\x44\a\t8\x7f\\\x5\t\x97<\x91\x81\xa3K\x83\?\a{\b\x1x\b\a8\vv<\x3\xf\x8b" 
  "\x37\x9fy\f8\xa0\\\x6\xbb:\x4\xb1\xa4\xc2<l:\x90\x37\r\xae\x37\a\xb1\x3" 
  "\x10=\a\xb9\x39\x3\xbe\x41\n\xc8\xd5Y\xbd\xc3\xdc\x45\xc5\x39\xdb\x0\x6" 
  "\xca\x0\a\xa0\xa6:\x5\x1\xb5\x38\xd7\f\x4\x4\x97\x4\x1\v\x4\n\a\x7f\n{\f" 
  "\xe\xe6\xd7\xd3\xf\xea\xad\x9a\x65\x88 \x80y\xf5\xeeq\xc2\xd1H\x1\x83[\f" 
  "\xe<\x3\x90\x0\x42\x84\a\x86\b8\x88\x64 \x81\x1d\x84\xf6\xf0\xdd``i\xc1" 
  "\xc3_\x1b\xfe\x1bt\xb4\x33K \x9ex\xe\xdf\x0 i\xc0\xa4\xc7\x80\xf3\xf0" 
  "\xbc\x8bwCcBf\n\x14\xde\b\xa7\xe4\x1b\x8e\x3\xff\xd4-r\x0\x8a\xd9\xe\x5" 
  "\xe8vx\x4\xb0@\x8e\x38\x65\x44ue\x13\xf7\x0\x80\"q\x97\xc2\x8d\x1b\xeaK" 
  "\xd5\"sC\x1d\x89\xdb#V\xad\xd2\xab\x64o\xac\xa3*MG\xcd;s\rtU\xa4\xc0\x1" 
  "\xe\b\xd7\xe6\x9a\x1c\xbaw\xcf\xd4\xaa\x43\xb1\x62\r\xbb\x95\x88\xd1\x65",
  (unsigned char *) "\xff\xe.\b\x8a\x16\xf2\x8e\xb7\xe9nL\x82\xc0\x38\xae" 
  "\xdd\xadY\xc5v\x6\xab#\xd9\xd1Ko\xa5\x89>*g5\x0S\x6\xac~fM\xda\x96Zq\x9a" 
  "`[\xcd\n`\xf3h\xd7P\x1e\x97\x8b\x9c\xe3\x41\x30\xa4<\xa2\xe9\x88\x43\xf1" 
  "\x9ak\xde\xa4\x89{\xf6\xfc\xdc\xd7\xd8\x65\xa0 0`\xf0\a8Z\xd7\xf5x\xf2 " 
  "\xfa\xbd\xe9\xed\xb1\xe1\x41\x65M\xe0\xfc\xf7\xe8\xe0Qw\f\x6\xd0\nO\x1r" 
  "\x0\x1c\xb0\x63H\v\\\x1f\x65\x9c\x19\x2\x9cV\xe2\xec\x10\xda%\x9cQ\xfe" 
  "\xe7K\x1~a\xc7\x90\x3u\xb5\x85\x17\x1e\xae\xd5\xd5\xc3]\x0\xa8\x43!(\ftu" 
  "\x83v3\x19\b\xca=\x1fnS]O2\x15\x15\xdf/\xb7\xb9\x82\x0\x4\xb5 \xe0\xa1," 
  "\x91L\x2\v}@u8\xd9\x3\xf\x4\xc5\xa3t\x5<\xd0@\x8a\x9a\xf1X\x97\'\x3(\x90" 
  "@\x8f\n\xfc\xd8\x64\x8f:\x90\x34I\?\x91!\xd7\xdb\x8fW\xd9\x93\xc0\x1bO" 
  "\xfa\b%\x4\a\x84\xb9\xc0~Y*\xc9\xa5\x91\xba@\xa0\x12\x82\x61\x1e\x30\xe6" 
  "\x93\xd2\xb4\x92\xa4\x8e\xf7\x30\xe9$\x96\th\xd2\x84p\xe5\xe0\xd0G0#Y" 
  "\xc8\x3\x1d\x80\xd2\x87\x4\x99>\xa4\x62\x84\xa2\x39 J\xda\x42<,\xc0\xde$" 
  "\x15\x89\x98\x61\x10\x9cH\x1a\x14\x1\x95\xc2q\x6\x9f\x4\b\xda\xa8\x9e" 
  "\xdd\x84\x1]\xa4)Rb\xa9\x11\xf5\xe0\xa0\xaa\x30|\x1eT(\xe_\x95\n\x86\x2" 
  "\xbdH\xd7\x43P\xbc>\x95\xeb\x11\xbc\x6\xc5M\xac\x97\xee\x80\x80\xa3\xb6&" 
  "\xab\xec\x13\xc4.\xeb\xec\xb3]\xac\b\xed\xb4\xd4\xfe\x62\xd1,}~,\x9am" 
  "\xb5\x84\xd4qG\x10\x3\xc8v\x88}\xc8\n\xd1\a\xa4\xe1.G.\x18\xd7z\xd2X\xf" 
  "\xee\xe\x81k\x0\x9f,\xe0\xe6\"\xf3\xd6{\xefL\xf\x4\x30\xe2\x3\x10\xd0" 
  "\xb1@\x0\x80\xcd\x1b`\x82\xbd\x35\xe0\xef\x1d\x3\xdf\x6\x89$\x94X\"\x4" 
  "\xa9\x43\r\x92\t\x12\x7f\xc4\xab\x3\xc5\xe2X\xcc\xf1\x1cG\\[[\x11\xa7" 
  "\xfa\x10@dM\xd6",
  (unsigned char *) "rr;\xfdi\bN\xe+\xcb\x15\xd9-7\f\x1c\x19(\xb3\xb0\x43" 
  "\x33\x10\r\x0\xca\\\x1\xa2\n\xa1^5=7*\a\xd0G\xbc\xdb\x83\xc8%\xff\xd0" 
  "\x34\x31\xc4!\xfcZ\xd4\x97$p[\x1\x44\xc6<\xb5.\xe4\x18\x0\xd5\"M\tZ\x88" 
  "\xb9\x13\xb5\xa3\xd9\x11*\x11\xb8\\\xd9g\xf7\x66\x44\xda\x46\b\aIM\xb9" 
  "\xd4\x14\xd2\x1f$Q\x82\xc7\xdc\x83mC@ek\x10\xa7\x11\xeZ\x1f\xd4\xe0<\xb4" 
  "\xea\xa0\xb5\xd6\a\xa4h\x88\xa4\x43\xe1\x45\xa6\xae=\x8c\xb9\xc9\x1e\xc1" 
  "h\xc4\xd1T3\xc1s\t\?\xfe\xfe\x88\xf3S\xa6\xb5\xe8\xc1H\x10y/\x0t\?^3Ro" 
  "\xd0\x84K;\xa1W\x9a\\\xe7w\x83\x3\xe0\xb3\n\xed\xa4M\xb6\x4qP\x11\xe<:" 
  "\x10\xd2\'\xee\xd6rI\xe2\x10q\x86\xc8\xd8\x9a\x66\xb2g\xd8\xcb\xf4\xd4" 
  "\xff\x11*\x10\x18\xbeu\xd8n\xea-\xe6*\xec\?\f\xce\x88+\xe1\xb8\xdc\xc0" 
  "\xc7\x30Go\x1a\x38\x9d%\x80TP\x1\xac\xaf\x8b\xbd\x94\')\xac\xcc\x39\xf4%" 
  "|\xfe\rr5\x13\xa4\x8b\xa3\\\x81\x18\x11\x0jH\xae\x1d\xe3\x0\xcc\x10\xd8" 
  "\x36\x9e\xa6X\xc5\x37\xd3\x19\x10\x3\'\xf6\xbe\xe0\x85\x43~@x\xccu\xd4" 
  "\xb6\r\xc0\xc1\x5\x1c\xec\xe9O\x94\xa6\xb7\xbfW\x9c\x83\x38\x12\xc3K\xba" 
  "\x14\x17\x99\xc2\xbd\f\x19\x1\x39\x8a\xa0\xd0\xd2\x87`\x5\xab\x16\x6\x80" 
  "\xd4l\x1c\xc4\x9e\xdal\xa3=\xd3\x11\x87\xe\x81\xe0\xc1\xe8l0\x83\xd1\xc3" 
  "\xcc\x1bVC\x80\x19\x35@\x89\xa4i\xc0\x4\xa1\xc6\x10T\x94p|\xec\x80\x0" 
  "\x94XxE\xbb\xe4 L\xf\x62\xfe\aF\xb0\x65\xc3^m\xe2\x36\xe3q\xcb*H\x4\x80" 
  "\x0\x91\x46@\xef\xc9\x10\x1aw\x0\x8c_8\xf1\x8dP\x84\a1\xa2\x37\x93\x3" 
  "\xa8$Iw\x82\x92\x1\xd4\x34\x98\x88\xfc\xb1I\xef\xa3\xd7\x10\x35\x33\xb0" 
  "\x0\xd5\x4_\x8d\x9c\x9e\xe5\xf8\xa7\x3\x5\flR\x8d\x94\x8e\x92\x2p$\x8a" 
  "\xcd-(\xbd\x38\x1e\x10\xa6\xc8\x35-\xbd\x41",
  (unsigned char *) "N\xe\x89\x61\x90\x86\xd4\xa5.\xe5\x80\x94\xb6\xc0\xcfP" 
  "\b\x99\x8bU\xbe\xc1\x90\x6\xb0\x43\x9e\xf6\x18\x84rmb9jX\xa4\x14\xe8 @ " 
  "\xbc\xc3\a\xdb\x80\x14\xa2|9\x12\"=\x5^\xc0\xf4S\x10\x44\xc6\xad\x64\xc1" 
  "\xf2iI\x80\x65\x1f\xb2@\xcdj\x96\nF<\xc0U\xeb\xa6\x0NYD\xcb\x9b\xe8<\x4" 
  "\x33\xa7p\xacO\xf1\x31\x9d\xf0\x8cg\xec\xba\x65\aaj\xe1\\R\xc0g\x13\xf4" 
  "\xd9\xa8\x45\xacS\x9ek\xe0\xe3v\xa4\'\xcb-dlg9\xb0\xa7\xf\x34v\x99!0\xf4" 
  "\x15/q\xe6(\x10Q+.<\xe6>~*\xa8\xfe\x16\x86\x36\x9e$\x94Li\xec\x93\xca" 
  "\x87\xfe\x19\x6U\xb0\x42\xa3WxL_p\x18\x6\xb8\x8d\x8cV \xbdP6\\\x1a\x4" 
  "\xde\x8c\xad\x8d\xdd\x90\x9fS,\xca\xc7\x3\x30\x85\x38\xa9\xd3I=T\x87\x92" 
  "\x9f\xd8O$\x9d\x13\xf\xe8Zg7\xa1,\xd5\x1d\x43\x5\xda.\xff\x36\xc7\xa6\"" 
  "\xf5\xa9>\t\xc9Q\xbb\xe3\x8b\xa2\xad\xd0pF\xa5\f\xde\xa2\x8a\x0\xaf\x11" 
  "\xe0\x19\x65\xddL3\xc5\x33\x93\x92\x9c$\xa8\x1b,\xa2\xb5\xde\xa9\x17\x11" 
  "\xbc\x41|\xb9\xc3kAm\xe7=\x8a\xd8\x1\x31#\xb3\xddh\xf4\xca\bZ8\xc4@}\r" 
  "\xac\x62\x14$\xbd^\x98\x63\x1c$\xfc\xa0`\rw\x83\xdc\x19@&\xad\xc1\xac-" 
  "\xfej\x15\xc1\xe4\x42\xaf\x98\x61\n\x19\xdey\b\x85!\xc0}\xbc\xa2W\x5Wt\"" 
  "\xcd\x9c\xf5=\xad\x45-\xfcz\xf2\x0\x89\xfa\x90\xa3\x8c\xbdm\xa3&\xc2\xb1" 
  "\x13\xc9\x16*\x3\xe8\xc7W\x82\xeb\xc7Z@\xf0\x35\x43\x91\xc3o\aA\t\xca" 
  "\xc4\xd4\t*%N\x8f\xfe<(\xd7\x90\xea\xb6\x39\xba\x5\x62v\xe5\xda\x8a)\x2" 
  "\xe7\x87\xd7]M\rU\x12\x94I\xfd\xe3\x44\x1e,\x0\xd0\xc0\x94\xa1\xf5*\xa3" 
  "\x87\xe1H\x8f\x83z`\x9c\xd1\xfa\x0W\xc5\x41@\x13i\xb5_d\xacj5lt\xa3\x89" 
  "\xc2+\xa0\xfez\x85>\x86P\x13\x8ar\xfb\x46\x2\xc7\xa5\x86\xef\xab",
  (unsigned char *) "\x8a\x19Od`\x95\xfc\xa1+9\xbc\x41W\x2\xbc\rAU\xb8>\x19" 
  "\x42)\x15T\xfa\x89\xa0\x80H\x1c\xb4\x44q\x3\xe6\xd3\x8e\x3\xd8\xe9\xc5" 
  "\xfa\xc8\x91*\x85\x64\x8f@^\xc3\x96*6\t\?\x1c\x81\x10Oh\xa2\x95X\xca\x10" 
  "\x8d\xeb\xe4\xa5\"\xbb\xf2\x43H\xce\x9f\x8b\x8d\x8c\xa5\x41\xae\x98\x1" 
  "\x35qCk`A\x0\x39\xa0RGml\x93\xe5\x9c<\x18\x17\x95\x93\xa7\x89\xda\x43P" 
  "\xf6\x3)\x85\xfe`\x99\xbdL(\x14H:\r\xab\xc0\x92\x10\x87\x30T\x9c\xf7\xc3" 
  "\xeMQ\xaal\x9c\xf8\x93\x18\xba\tP*xU\x94_hUO\xe6\x98\x6>\xf7Y\n\xff\xd8" 
  "\x96\x19l,\b+\xd2\x1e\xfa\xd1\xcf\x32\x34\xa4\'=\nIS\xfa\xd2h\xb0\x34" 
  "\xa6\x37\x1d\x6Ms\xfa\xd3\xd1\xaa\x81\xa8GM\xeaR\x9b\xfa\xd4\xa8\x46u\b" 
  "\x0\x0;",
  NULL } ; /* --- end-of-image9[] --- */
/* ---
 * image10[] contains a 1465-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (10) dvipng ran but failed:\\See mathtex.html\#message10}
 * ---------------------------------------------------------- */
#define	IMAGETYPE10 1			/* 1=gif, 2=png */
#define	IMAGESIZE10 1465		/* #bytes in image10[] */
static	unsigned char *image10[] = {
  (unsigned char *) "GIF89a\xcd\x0(\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0\xff" 
  "\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb\xdb" 
  "\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff==\xff\x1\x1\xff\x16\x16\xff[[" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4\x1\x0\x0" 
  "\x0\x0,\x0\x0\x0\x0\xcd\x0(\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa\xael\xeb" 
  "\xbep,\xcf\x30`\xdfx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\x41\x11r\xc9l:" 
  "\x9f\xd0\xe8P)\xadZ\xaf\xd8\xec\x91\xaa\xedz\xbf\xe0%7L.\x9b\xc1\x63\xde" 
  "\x80 D8\x82\bE\xe1\xcc\x8c\xcf\x9fr\x9f\xfd\xb7\xcf\xa6u\x5\x6\x37\x81" 
  "\x3\x37n\n\xe\b6\x3\v@\x5\t\x82<\tot=\x8f\x91\x41w\x80\t\n\x1;\x93\x0" 
  "\x97\?\xa2~\x9e>\a\x8a\x0\f\f\r\n7\xa8\x0\b\a7\v\x85\?\x6\x98:\x4\xb6" 
  "\x95<\xb8\x43\xb9\x38\xbf\x9a\x38\xbb\x36\xbf\xb7\xc1U\x7f\x38\x3\t9\a" 
  "\xae\x0\x3\xb3\x36\xd1\x8b\x10@\xc8\xbd\x46\xdb\?\xe\xca\xc7\xe1:\xde" 
  "\xbe\xe3P\xcc\x37\xe5\xd7\x0\x6\xd5\x0\a\x98\xa6:\x8c\n\f\x90\xaa\a\xd9" 
  "\x0\t\x10\x11\r\x82\b<\x98\x64 A!\x2\x1\x16\x10Pp\xe0\xe\x83\x5\x6\x16" 
  "\xdc\xcb\x31\xa0\x81\xc2\x0s\x18\x10 \x10\t\xa1\x42\x86\xc4\x8e%\xd8Xk" 
  "\x1a@\x93\x82\xfe\x18<\x88\'\xed\x6\xc3x\xaa\x36\x46Rp\xb2]\xa4\x87\x4" 
  "\x16\xcc\xa9w/R\xb9\'\xe9\xac\xb5\x14z\xec\x1dLk\xbc\f\x5Hu4\xe2\x31\x45" 
  "\xc8\x9c\x86Z\xda\xae\x1a\x32\xaa\x12w\x18h\x0 \x11\xbf\x42\v(\xb9\x13G" 
  "\xae\xd1T\xa8>\xd3\x9a\xb3\x61\x10@X\xb2\x36\x1\b\\\x84\x8a*<\xb5\xcb" 
  "\xe6\xed`\xf7J\xdaXk\x98\xf8\xdep`4R\x1\xaa\f\xe0n;\xf0&j$S",
  (unsigned char *) "\x6(\xe5(\xa7\x80\x0\x4\xbc\?\x17\x37\xc6<n[\xe5\xcb" 
  "\x8a\x5%\x88\xa6\xa0\x13\xe1W\xe7\x80\xea\xd5!\x18\x9e\x34\x6\x46\x13" 
  "\x13\x9d\\\xf8\x6\x84Uw\x1c\xa3\x8e\xcb[!\xc7\xb5\x37\x38\xf1\xd6=9\xb0 " 
  "\xe2\xbf\x86\xc2\x15\x8e\\\xd0\xd1\xa2\xbbK\xf9\xc8\x8a\x83\x1d\x82\a7" 
  "\x1ehz\x90\nG\x1\xec\x80o\fx {\xb8\xd9\xc3s\x88\x97\a\xe\x80\xa1\x8d\xcb" 
  "\xc7\xf1\n3\x8b\x0#o\xd0q\x95%\xaf\x6\x7fx@\xae\x36\x38\xf0]t\x1bY\x11" 
  "\x94\\f\xa9\xfe\x62@\x0\xf8\xa8\xf2\xc6*\x86\x0\xa8\xc3\x43\x95\xad\xd4" 
  "\x12;4\xb5R\xd5\x42#\xb5\xd7@+\x19\xba\x2\xc1\x1$.\xd0\x9d\r\x19\xca&" 
  "\xcb\x0\xf6X\x14\x62\x88\x93\x39\xa0\xc0\x0\x10\xb0\x11\x8a\x41\n$\xa0" 
  "\x61\x1\r8`\xa3K\x1f\x32\xb0\x62\x8b\v\xbc\xf8\xa1+\x6@\xe0@V\x14\ntM\x2" 
  "\xcf\xe4\xf5\xc3;<\x14\x90\xc7\r\x10\xf6\xa0\xc8\x89\xb1\xac\x15\x92\xe" 
  "\vpRY\?p \xf1\xe5\x96\x39p\xb9\x83\x9a\x96\xa4\xd9\xa5\x16\a\xca\xb5\x1e" 
  "\x10\bD\t\xc5O;(tCN\xdc\xf4\x19\x45\x9cr%\xf5\r\x9bH(\x80\x8br<\x94\xa6" 
  "\xa8\x9f\x8c:\x1h(m\x10\xda\xe8\xa4\x94\xf6\xf0h\xa5\x98\x66\xaa\x9a\xa6" 
  "\x9cv*\xa5\xfV\x16\xf2\xa5\x17}<Q*\x12\xa7\xda\x80@*\x92z\xaa\xc3\x81" 
  "\x10\x6\x42\x65\x17w\x90\x92\xc3\xa8\x8e\x34\b\x88\x10\xb6\xd2\x32G\x81" 
  "\x94\x12\x62\x88\x8c^Y\xba\x9aw\xef\xc8\x12\x6&x\xa6\xe6\x3\x9e\xed\x0" 
  "\xa3L\x94\x10\xfe\xb4z\xc6*\x1aZ\xb3\xe5\xac\x39\x4\xa5\x0w\xea\x80\x1" 
  "\x8e:\xca\x8c{\x4\x9e\xe6\x6QN \xef\x65\xca\xe\x35}\x19{\xca\x3\x6\f\x85" 
  "\x93N\xaa(\x84o1\xf|t\x0\x8b \xd9\xa0\xd1o\xaa\xacTo;\xfe\x66\x64\xb0+" 
  "\xf7\x16p\xcf\x33\x4\x1c\x90\xe0\x31\t\v\xbc\xb0@\xfe\x2\xdc\x10\\\x83" 
  "\x15\x32\x80\x64\x18/\xf4oi\x1b\xe7\x9bS\x1\b\xd4",
  (unsigned char *) "kY,*\xef\x33\x30&\xf\x45\x34Q\xc3\xf1V\x15\xdd\xeqn%" 
  "\x2\x1bsMs@\xcf\xf0\xd2\x6\x1dom\xbd\xc5\x9b\xcd\xfe\xd9\x0\xf4,\x15\xc5" 
  "\x31\'\xd2\xdb\x44m\x95Uj\x5\x82K<\xee\xe0\xe2\xd3\xd4\x64-m\x80\x8d\x8d" 
  "\x81\xcd\x16X\x94`\x95\xd8\xd2\x35\xffu\x97\xbct:\xb0\xd4h\x8a\x32HZ\'" 
  "\xe4p\xe6\x92\x65\xf2\x35\xc7\xd6\xdc\xa6\x10\xd0\xc0\x8f\xc2\xe4\xcd" 
  "\xac\xddG\xc7\xb2\x8f\x9d\xa1\x91\x5wi\x1\x8c\xc7\x98\"\x8e\x17\xdb\x1e" 
  "\xde\x36@\xf6\xc6\xe2t\xcf\xa6\xf6s\xaf\x1e\xeb\xd2P\xfe\xadp\xcey\xdd" 
  "\x89\xf3\x83\xa4Z\xa7\'\xce\x90\x32\xb2\xecS\x1c\xc7\xc9%\x9dt\x1c\x88" 
  "\xd4\x9b\xe3\xa2\xb2\xff\xc2y\x1\xe\x8f\x18J\xef\xd5\x30\x17\x89o\x91" 
  "\x8c>\x1bl\xaf<}\x83\xb7\xb9\x34\x80\x80\xdf\x83\x41\x1f \xe9I\xbb\a@" 
  "\x7f\xbf\xc4\x97\xf8\xd7\x12\xbe\x91\xf2\xf5\x94\x0K\\~\xb9\xab\x45;ia" 
  "\xe1\xae\xb7\xf4]\x19p\aW\xee\xdb\xc0\x95\xf5\xfd\xcd\xc9\xbe\x64\xd6" 
  "\x81\a\x80vl\xef\x90\xa3\xed\xb7\x39\x86\x92\xb2\x92\xa4%\xcd\x89! :R" 
  "\x88\x86\xc4\x8a\x46\xf0\xc8GF\xdaQ\x8f\xd8P@\x89\xa8\xa4\x11\x1ey\xc4" 
  "\x33\"(\x8d\a\x8a,\x81 ,\xcd\x91l\xd3.\x1c P\x84!l\xc7\x0\x19\x10\x91" 
  "\x35\x34\xa6\x16\x4\xf0\xde\xc8\x1ax=\x12I\f*+T\x10\x83n\xf2 \xe5-\xcfs" 
  "\xaaRDi\xb8\x14\x12\\\x95I\a\xd6z\x93\r\x8cH\x84$\xee\xe0\x63%4\xc2\x1d" 
  "\xd0\xa4*%V\x91\x16\x62\"\x0\x99\x96\x38\x8a+\xf5\xcfU\x99r;\x80\xc7$" 
  "\x13\x6=)mb\x9b\x2\xa3\xa6\xa4\xe1\x45\x32(\xaa\x34\x6\x2\xa2\x1a\xe7H" 
  "\xc7\x1f\xd6\xf1\x8ex\xac\x9c\x1c\xf3\xc8GL]\xaa\x8f\x80\xec\x5\r\x6I" 
  "\xc8\x42\x1a\xf2\x90\x88L\xa4\fB\x0\x0;",
  NULL } ; /* --- end-of-image10[] --- */
/* ---
 * image11[] contains a 2134-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (11) Can't run dvips program:\\check -DDVIPS=$\backslash$
 * "path$\backslash$", etc.\\See mathtex.html\#message11}
 * ---------------------------------------------------------- */
#define	IMAGETYPE11 1			/* 1=gif, 2=png */
#define	IMAGESIZE11 2134		/* #bytes in image11[] */
static	unsigned char *image11[] = {
  (unsigned char *) "GIF89a\xd3\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff==\xff\x16\x16\xff\x1\x1" 
  "\xff[[\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4\x1" 
  "\x0\x0\x0\x0,\x0\x0\x0\x0\xd3\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcf\xb4\b\xdcx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8fH\xa4-" 
  "\xc9l:\x9f\xd0\xa8\x34\xb9\x9cZ\xaf\xd8\xac\xf6Y\xddz\xbf\xe0p\xb4+.\x9b" 
  "\xcfh2o@\x18\"\x1c\xc2\x82\x62p+\x14\v\b3BaG\xfb\x7fj:\x5\x6\x38\x83t" 
  "\x85\x6\x87\x0\x3\v\?\x3\t\xe\n\x4\v\v\x84\x44\a\t\x85`\x5\t\x96<\x90" 
  "\x7fg\x81\x39\ay\x0\f\f\r\n8\xa8\xaa\x39\v\x8a:\x3\r\xa6\x37\x9dw\f8\x9e" 
  "^\x6\xbb:\x4\xb1\xa1`\xa3\x37\x8f:\a\xab\xa4\xc9\xc5\xf=\a\xb9\x39\x3" 
  "\xbe\x41\xc9\xe\xd3Y\xbd\xc2\xda\x38\xc4\x0\xd9\xca\xc7\xcb\x37\x1<\x5" 
  "\x1\xb5\x38\xab\f\x4\x4\x96\x4\x1\v\x4\n\a}\ny\f\x10\a\x6\xe2\x8b\r\xf1" 
  "\x1\x5\x66\x11\x12\b\xe0]\xbcy}\xa2-P\xc0\xe0\x16\x83\x3\xcd\x0$x\x10" 
  "\xa1\x1!\x2\x10 \x19H@\a\xc1>\x2\x11q`\xd4\xc8\xb1\xdf\?;\xed\xfe\x18" 
  "\xb6\x39UiA\xc3Y\'O\xb1s\a\xe1\xe0\x81\x1\n\x10\xde\xf8\x6\x88\x9c\xf" 
  "\x64\xe1\x82\x92\n\x6@\x81\xcfOt\x16\xc0\xf1v`\xe7\x34\x9e:\f4\x0\xe0 " 
  "\xcf\xb7\xabM\xbd\xf9\x42p\xee\x86\xbe\x9d\x8d\xbcY\xb5\x64 \xac\x39\x8f" 
  "+\x97\xe6([\xa7\xabT\xaa{ \xe0x\xb0\xaa\xab\xcb\x9dS\xabJL\xba\xd4@V\xbf" 
  "N\x3\x3\xe9\x6\x14\x9c\x61\x1c\x85s\x9c\xddQ\xf$\xd9\xc7",
  (unsigned char *) ";\xa0\xae\xddu\x15\xb2V\x1d\xe\xb2\x2\xf8\n`1\xb4\xca" 
  "\x88\x1d\f\xc8\xa7\x37\xaa\xa7\x3px&\b\xeb-\x93O\x3}}Iz`\x19\x34\x11\xc2" 
  "\xfc\x12{\xcd\xcd\xcf+4\x1co$\xae\xb2-\xf9ro\xa8\xc4-\xeb\xd2\xcc\x19" 
  "\xc0\x3T}lo6P\xa0\x0\x83\a\x9au\x9d&\xc4\xf3\x0k\xc0\xf1\xdai\xcf\x91`xm" 
  "\xe5\x42\xba\xdd=\xbc[\a\x4t8f\xa1\x33\xb0\'+m\xee\xe8\x3\?\xa5l\x89\xb6" 
  "`\xc9\x5\xc8\xe5\x95\'\xa3\xfdv\x95Y\x0Q\xfew\xc3TQ!h\aO\xa9\xcc\x95\xcb" 
  "o\xe3\xdd\x30\xcf\r\xf7]\xa6\xe1\x37\xec\x4\xd1\xcd$\xac\x18\x10\xc0-" 
  "\xa7\x88H\"\x0\b0\xb8\x6$\x92\xc0\x82\xe2M\f\xf9\xa3@\x3\xaa\xcc\xe8\x8a" 
  "\x62\r8\xb0R:4\xfe\xc6\tN\t\xd4HcNC\xea\xc0\xc0\x42\x18%V\x98\x8d\xc3" 
  "\x1d O\x2m\x94\xc5\x86Z\xcb=\xd9\x86\x8d\x6>`M\x7f\at\xb9\xc0\x1e=\xde" 
  "\x80\x0\x8c\xa9,t\x80\x90h\xae\x92@&\x83\x1d\xe5\xcc\x11\xa8\x0!G0\xf0" 
  "\xb9\x41\x44\x1eu\x2\x81\xa7\xey^\x96\xd0\x9e<d\x93\x90\xf\t-P\x9e$\x13" 
  "\xf5\xd0\xa7\x13\xdd\x14\x44\x61\x10\b\xb0\xb9\xcd\x19\xc5\xfdP)\x10\xf1" 
  "\x88\xc4\xda\x30n\xf6\x0\f\x11\xa5MZ\x86\x2\xbd\xf4\xd6\x3\xa9\xfb\x1c" 
  "\x91\xd3\xaa\x66\x34Z\x87\x1b\x8b\x8a*\xeb\xac\x43\xb8J\xeb\xad\xb8\x66" 
  "\x61k\xae\xbc\xf6\xda\xc4\xae(\xf2\x61\xc4\x1e\x83\xf6:ggA\f@%\x8av\xe0" 
  "\xd1\x4\xb1\xb2,\x8b@\xb3\xb1\xfeJ\x1,\'\xd7\xfc\x80\xed\x10\xa4\x8eX" 
  "\x96w\xa6t\xdb\xc9\x2\xe0\x9e\xd2@\x0\x9e\xcc\xf8\x80\x1c\v\x4@W\xb7\xf7" 
  "\xf9wC\x9c\x83\x64\xe7\x83\xa4;\x91\x83\t\x12}l\xab\x3\xbe\xde\xe8\vp" 
  "\x1cG\x0\xab\x61\x11\x97\xf6\x10\xc0\x32\x33\xd6\xb2\x30\x8fy\x98\x33" 
  "\xe8.\xf\x8f\xb3\xcc-\x5h6\xa6\x10\xa2\xe5\x10\x9cuHxR\\\xc7\xc0\xc1\x1" 
  "\xb2\x11\xd9\xf6",
  (unsigned char *) "\x84p\xca>$\xcc\x43\xc5\x18z\x2\xb3s\x96\xacV\xc8\x8e" 
  "\x0\xc0\f3`\n\xbc\xa7\x8b\x10\b\x84\x64\xe1\xd0\x46XS\xa1\xc7\x42\x17" 
  "\x45\x34\xa8,+\xdc\xa9,\xe3\xe6R\x96<\xf4\xcc\x1b\xcf\x2v<R\x96\xd4\x17" 
  "\x35\xe7\xc3\xcc\x18q#N\xd8\x5\x1d\xb5\xac\xce\xcb\x38\xe9\x15\x4\xa9" 
  "\x12\xf1\x65!y(2\x92\x35%\xc9$\xde)\xf9\xa4:\xb5N7\xbc]G\xdc\x41\x1c\x39" 
  "\x89uy\xaf\xa2\xf5zN\x13\n\x10U\x99\x0v\x19\xd9\x3\xd0\xb3\xb8\x3\x8d" 
  "\x13\xb2Pz\xe2\x18%v\xe\x9a\xdf\x0\x41.\xfe\xc1\xe5\xa0s$\f\x89#\x95\bm" 
  "\f\xd2\xcb\xea\xab\xf7\x41\xc0\xa3\x91!\x98GIJ\t\xc6\x94\xed\xaf\x1f\x1y" 
  "S\xdfH\xcc\xb8\xca=dfZ`\t \x93S\x0\xc2\xaf\x45\xae\xa9\x38\xe5\xb4y:\x2" 
  "\xe6\x9cy\xf4o1P,\xda\?\xbc\xd1\x95\x10IGv\xdaR\xb3)\'\x9dsG\x14\xbf*9" 
  "\xdf$\xef\xe1\xd3\xcb\r\x7f\x99\xd7\x8e\xebR\x1e-;0\xb0\xfa\xf3}\xcb,N%" 
  "\x85\x4\xa0\xec\xe\xd8\xd3\x81\xf3p\xa0\x8a=\xac\xea\x80\xf6\xd0\x45\xb1" 
  "\xdc\x37\x1d\xe1l\x88\x10\xe6\xb1\x9d\x37\x16\b\x4\xaf\x19\'~mR\x9c)\xa2" 
  "\x4\x19\x2\xa8\xc8\x1\x8b)\xc8\x65\x1c\xd0\xbd\x97-#r\xe;a)r\xf0\x80\x1b" 
  "\x89n\x19\x33\xb3\xd0.ha@\x4\xe\xb0\xe\x9b\x8a][\xe4`\x9fl\xf8\xf0\x81" 
  "\x8a\xc9\xe1/\x82\xe1\x41\x1c\xa4\x86;!lC\x87\x0\xc8>V\xa0&\x11Lb\x92" 
  "\x37\xb4t\x97\x87X\x3\'\r0\xde\x88(h\xa1v\xdd\xa7,\xe1\xf2\xfe\xe2\xea" 
  "\xfc\x6=\x1\xb6\xebP^\xe4\x87\x2:\x91\x93\xe7\x14\xa1\x84UZc\x1b\xc6\x84" 
  "\x93\x32u&G\xf2\x18\x92\x14\xe7\xf2\x83\xe2\x45\x86\x8a\xb9(\x0\x1eO\xf1" 
  "\x44:\xac\xe9\x65M\xf4\x18\xc1\x14\xe9\a.B*\xf\x39\xa9V\xfdp6\xbc\x62" 
  "\xe5I\x92\x32\xf9\x81\xa9\x90\xc5H1\xad\xcfW\xa0\xc4P\xcb\x9a\x86\x4\x38" 
  "\x6K",
  (unsigned char *) "W\x89\f\xa5\xac\x1e\x90\'Tm\x92\t\xac\xec\x1Q\xac\x95J" 
  "UN\n\x1\x98\x94\x2.EQK[\xfa\xf2\x97\xdc\xa8\xe5\xb1\x1c\xa9\x5hE\xc1\x98" 
  "L@&pL\x91K`v\x83^\x18\xf4\x42\xbfN\xa4\x18~Q\xb3\x9aq\xb8&\x0\xb0V\x10Jn" 
  "\xc3\x10\x8aI\x84\x11\x88\x91\x31\xe0\xd8k\v\"\xbb\x6){p\xa9uJ\xd0\x16" 
  "\x18jf\x18Z\xb1\fz\x8e\x33\x91=\xab\x85;\x9f`\xb4wRe\x9f\f\xcc\x41\?\x85" 
  "\x0\xa0\xfe\xe0J7\x9bye\xe2~\xc2\x36q\b\x8e\x9b\xf-\x16\x46l\xd2\xbc\xaa" 
  "\xd9\xcd\x12\xf8\xd0\xc7\xfeplb\x87\x8c\xa6*\xa2\r\xc9\x4\x1\xbc\xd3 " 
  "\xaa\xf5\xc1\xa3\x92\xa8\t\xd5*\x1a\x1d_t\xec\x7f\x37\x98\xe8Jsb\xd1\x88" 
  "zd6(\xfaHD\xd6q7\x96l-\x13\x11\x45\fo\xee\t\x84\xd3\x5@\x89\x2\x8a\xdc" 
  "\xee\"\xf3\x97\xbf\xd4\x8c/\xb6s\x9cm\x96*\x1f\x86\x30\x35\xaa\"k\xea" 
  "\x7f\b\xa1:\xbf\xf4\x42\x1fX\xd9\x89S\v\x92T}\xa4\xc5\x1bg\xdd\xcb\x36" 
  "\x97\x62\x97\\,\xb5=\xecIO/Q\xe4\x80s\x98\xefxw\xed\xdc\x64\xb6\x9a\xe" 
  "\xc7`\x95\xaf\x12\x31^\xe7<\xe8M \xfe\xd5\xb0P\t\x1a<\xf7\n\xc4\xbc\xfa" 
  "\x8f\x34q\x83l_\xe5\xf5\x1a\x38\x38V\xa8\x42\xb9\r>\xc5\xa1\n\xafY\xd0}" 
  "\xb6)\xf\x10#8\xda\xcf\x8e\xa9\x84\xd2\xc9\x6i\x93\x13,\x5X#\'\x87J\x6k" 
  "\xbdV\x9d\xeb\x34\xa5\xb6\xd8\xb1\x5i\xc3\x63\x89\xcf&4\xb3\xb5\xc2\xe7" 
  "\f\x11P\xc4\x1b\xe8\xe8\x83\xde\xe3\xeb\x85h\x86\x1fo \x96;\xc5\xa5JN" 
  "\xda\x9d\xb3\x94%\xa6\x96;\xcf\r\x8c\x1\x8d\xa7\x14Ve7\xba\xd6\xe8\xc3T" 
  "\x14\x4\x80\xa9,7C\x8f\x2/f\xe3\xfa\xc9S\xb1Q\x1n\x9c\xa2\x3\xee\x62\x0@" 
  "\n\xf0LDJ\x13\x1d\x63\xd4\bA\xea(\x8a\x45\xf2o\x94\x0\x89\x8f\x46\x18" 
  "\x84\x13\x99\x0\xf0\x8d\x4<\x8f\x34)x\x19\x11I",
  (unsigned char *) "Z\x83\xf3Ka\xf9\xba\x44J\x4H\r,2\xfc\xa2:\xfa\xc3\x39]" 
  "*W}\xe7;!\x13\x61\xd4\xc4\x45 \xc6.#)\x88\x16\'\xe1\x92\x90\xc2\xa6\x13" 
  "\xe4\x19\x9f\xa5\x98RN\x9e\xac\x5\xa0\x80\xf3\x8a\x43\x11 Q\xaf\xba\x82" 
  "\xc1\x80\t\x86\x97.\v\f\x99\x8a\xa9\x10\xad\x30\x64\"{!\x19\xc2:\x3\x2" 
  "\xbf\xd0\x64\'[YVU\xbe\xb2\x96\x85\x91\xe5-{\x99\x97_\xe\x33\xaf\xba," 
  "\xe6\x32k\xa1\x6hN\xb3\x9a\xd7\xcc\xe6\x36\xbb\x19\x6!\x0\x0;",
  NULL } ; /* --- end-of-image11[] --- */
/* ---
 * image12[] contains a 1426-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (12) dvips ran but failed:\\See mathtex.html\#message12}
 * ---------------------------------------------------------- */
#define	IMAGETYPE12 1			/* 1=gif, 2=png */
#define	IMAGESIZE12 1426		/* #bytes in image12[] */
static	unsigned char *image12[] = {
  (unsigned char *) "GIF89a\xcd\x0(\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0\xff" 
  "\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb\xdb" 
  "\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff\x16\x16\xff==\xff\x1\x1\xff[[" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4\x1\x0\x0" 
  "\x0\x0,\x0\x0\x0\x0\xcd\x0(\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa\xael\xeb" 
  "\xbep,\xcf\x30`\xdfx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\x41\x11r\xc9l:" 
  "\x9f\xd0\xe8P)\xadZ\xaf\xd8\xec\x91\xaa\xedz\xbf\xe0%7L.\x9b\xc1\x63\xde" 
  "\x80 D<\x82\bE\xe1\f\x97\?\xed\xbd\xf8\xdc\xa7\xa7\xa7u\x5\x6\x37\x81\x3" 
  "\x83\x6\x6\v\b6\x3\v@\x5\t\x82<\tot\x8f\x91\x41{9\x8f\n\x1;\x93\x0\x96\?" 
  "\xa1g\x7f\x39\a\x8a\x0\f\f\r\n6\b\xe\x36\xf\r7\v\x85\?\x87=\x4\xb6t\xb8" 
  "\x42\x97\x38\xb8\x99\x38\xba\x36\xbd>\xc6\x65\xa5\x37\x3\t\xa6\xad\x0" 
  "\x9c\x37\x1\xcf\x3\xb0\xb7\xbft=\xc8>\xf\xd8\xc5\xde;\xdb\xe1\xe0_\xca" 
  "\xdf\xce\xcb\x36\x5\x1\x99\x9d;\x8c\n\f\x90\xa9\a\xd6\t\xe\x11\r\x82\x4" 
  "\x10\x93\x6\t\x85\x10\x18P@\xc0\xda\xb2\x6\v\b\xb0KE\x80@$\x85\t\x15\x1c" 
  "\x10v\xe3_\xc3Z\x0\x6\xe8\xcb\xb8\x91\x1\x84\x3\x3qH\x4\xc9\xd0\xa1\r\x5" 
  "\x1b\xfe\x1\xf4\x62\x90p\xc1\x1cx\xf2\"\x89\xd3\x62\xe\xc0\x81g7n\xeaX" 
  "\xd0(\xe7\xae\x1b\b\x2\xa0\"\xa9\xb2\xa7\x1\x45\xbd\x10\xa9\x13j\x80\r" 
  "\x0J\xc0\x66=P\x4\x10\xc0\x2J\x6\xe\x9c\xd3\xa1\x14\x94P\x95\x32\xc3\x8e" 
  "\xb3Q\xf5\xeaV\\\xfc\x16\x9d\xfajSl\x98\x9a:q\xc4\xbd\xc1\xc0\xa0\x8d" 
  "\xb9\x37\x1eh\xbd\x1bi\x9d\"\x6g/\x1dx0\xe0\xe3T\xae\xd8\b:p;\xd3\xd8`" 
  "\xb0",
  (unsigned char *) "\x81\xc7\x9e,\xc8XP\x82\x9b\n8\xe9\xcdI\xce\v\\\x9cwA" 
  "\xf\x10T \x13\xde\x62{\xdb\xdep\xa0jOR\xc1\x6J\xd7M]\xf1W\x82V\xaf\xb7" 
  "\xe6p,(7d\xd0\x90\x1\xdc\xe\x9e\x9b(j\xce~\xdc\xf1X\x0X.\xce\x35\x36\bd" 
  "\x82\x80\nG\x1\b\xc8\x17\x41hN\xbc\xe7\xba@{f\xed\xbe$\xd1\xc6\xe2\xdenw" 
  "\xf7\f:\xa7\xd7\xe2s\xd8p\x95\ap\x9e\xf8>\xf1O\xafgoH\xa6&\x81\x9e\xa9" 
  "\x18\x10\xc0<\xeb\x90\x0\x14~9\xb0\xfe\x44\xd0G8\xc5\x85\x12+*\x1d@\x80" 
  "\x2\t\xb0\x81\xc8\x1aP\x9d\xd4@\x3\xcd!p\xc0\x0\xf1 \xf4`f\x1b\x2\xa7" 
  "\xd2\x3\nT\xe3\xd4# &\x0\x61\x1\r<\xe0\x94\x86\x1cz\b\xe2*\v\x8c\x38\xa2" 
  "J\xe<\xc0\\*9\xf2\xa3S\x2\xcd\xbc\xa5\x1c\xf\xb4\t\xa1\n\x1f\x0T\xe7\n" 
  "\xf\xc1\x0\xd5\xe4\x10N\x16\x41\xd1\x94UV\xa9\x83\x96=P\xa4\b\x97h\x1c" 
  "\xb9\x3\x1\xdc\xc1Q$\x14\x33\x65\xa3\x66r@\x10#\xc4\x61P(p\x88\x89k\xd6" 
  "\xd9\x9f\x98<\\\xc9\x3\x2`\xda\xe9\xe7\x9fS\xe0\t\xe8\xa0\x84\x86Y\xe8" 
  "\xa1\x88vQ\x13()\x82\x62\x46\x1fP@\xba\x84\xa4@\xa1\xd2g\xa2<\xd4\xb4" 
  "\x64 Ij\xb1\xc7(\x9a\x1c\x1j\xa8\x98\xcc\x93\x83K\x0\xf0\x37(!\x86 r\xa9" 
  "\x39\x5\xa4\xe6\x61\x18\x97\x34\x86\x44\x9a\x9d!\xa6\x43\x91\xe\\z\x86*" 
  "\x10\x36i\x8d,>\x98\xa3\x0u\x15\x81\xd1Mm:,{\xc4L\xce\x6\xb1M \xe6\xfe" 
  "\x1d\xea\xa0r\xd3\xf4\x0\x17\x4!\xd1\xd5\xd2\x1c,\xfdG\x11\?\x11}\x98" 
  "\xd9\x44\x36\x30\xd0P$\x1e\x81\x84[\xb9\xe0~\x14R\xb8.\xc9\xd3\f\x1\a\x0" 
  "X\f\xbc\xe9\xca\xbb`\xb9 J\xe4\x1a\x36\x84\x65\x4\x15\xb9\x13\x9a+p\xba" 
  "\xdf\n\xa4X\x93\x3\x15\x94\xee\xba\xde\"\"\xf\x90\xe2\xe6\xf4\xdcRz\xda" 
  "\xb0\xa8\x1\r\x88\xc0\x46Z\x19I\x88]\xc9\\\xed",
  (unsigned char *) "\x95\xd5VeaU\xab\xcan\x91<\x80V\x1a\xc5Q\xe6q\xc1\xe5" 
  "\xbcr\x84\x91\x5r\bHY\x1d\"\x93\xca\x30\xa7z\xf2\xccM\xc5\xa2\x92S\x94" 
  "\xb4\xec\xb1\"\?\xca\x9c\xdaiV\xe9\xab\xc3\xa2\xae< \xd4\x65\x99q\xc2ufx" 
  "\xfa\x66\x8c\x62\xe9\x89\x1d\xc9\xd7\xd1\xa4\xda\xc0\x8c\xc0\x94]ke\xba" 
  "\tK\xd6x\x91\xa1\x1d@a\x83)\x82\x37\x9c\xd0P\xe6q1o\xd8\xadq\x82v\xed`,h" 
  "\xac\x18\xa7\x9a\x36p\v\x87\x9bX\x8fG&\x91\x37\x1e\x16\x1e\xd9\x39\x91" 
  "\xdbwN\x1c\nt\x93\xd9m\xfe\x99]\x8e\x8b\xe2\xb2\x39\xa0U\xe9{\r\xd7KB&-" 
  "\x8e\xce\x32\xa4u\f\x80\xb1\xbf\x34\x80\x0\x1\xf8\xc9\x98\xbb\xae\x9a" 
  "\xcfW\x1f.\xe8\x45\xd6\xd4\xee\x2\xd1G\x89\xaa\xbe\x41\x16\xbc\xe6\x90q" 
  "\x8e\xd9U]\x8b~_^\xb1\xd9\x30K\xf5\x0\xcc\xe2;.7\xe3\x9e\xd7\xe0\x8b\x38" 
  "%\x9d\xb6\x82\x9e\x4If\xac\x15\xd3\xe3\x8f\x6\xac\x7f\xb3\x44\xac\xe8X" 
  "\xa2\x8d!6\x2\xa3\x8c\xf2\xbf\x18\xa3\x85\xee{\xd4\bD\x8fhF\xfe\x9eq\xbf" 
  "\x84\xc5\xaf\x44\x3\\M\xb5\x44r\x80\x3:\x10\x37\xee\xbb\x10\x1\xde \xc1" 
  "\x37\xd0\xfG\xf4\x39\x80\x6\x13\xc1#\x1f\x1\x86\x1\x2\"P\t\x8aU\xbe&)\"3" 
  "U\xa2\x88\xec\x80\xc0%_Mi\x10Op\xe1;(a9!\xec\xe1KR\xca\x12\xe\x16\x0:\x2" 
  "\xdc\x3\x86N\xc0\x1a\xa6\xecT\xb0\x1\x64\b\f\t\xb9\xc1\x7f\xaa \xc4!\xae" 
  "\xe9\x19x(C\xd7\x42\xc7\xc4\x12:\xf1\x8aX\xcc\x94\x15\xb3\xc8\xc5,6\xb1" 
  "\x8b`<\x14\xd4\x17\xc3H\xc6\?\xd1\xe0\x8chL\xa3\x1a\xd7\xc8\xc6\x36\xca " 
  "\x4\x0;",
  NULL } ; /* --- end-of-image12[] --- */
/* ---
 * image13[] contains a 2307-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (13) Can't run convert program:\\check -DCONVERT=$\backslash$
 * "path$\backslash$", etc.\\See mathtex.html\#message13}
 * ------------------------------------------------------------- */
#define	IMAGETYPE13 1			/* 1=gif, 2=png */
#define	IMAGESIZE13 2307		/* #bytes in image13[] */
static	unsigned char *image13[] = {
  (unsigned char *) "GIF89a\xef\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff==\xff\x1\x1\xff\x16\x16" 
  "\xff\x32\x32\xff\v\v\xff>>\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xef\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcft\x9d\x2x\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\xc8\xa4" 
  "r\xc9\x13\x31\x9f\xd0\xa8tJ\xadZs\xce\xabv\xcb\xedz\xbb\xd9\xafxL.\x93" 
  "\xc3\xe6\xb4z\xcd&\xa2}\x3\x2\x11\xe1\x18\x16\x14\x3\\\xc1X@\xb4\xff\x80" 
  "Ro<\x5\x6\x39\x85yz\n\n\x6\x89\x3\v@\x3\t\xe\n\x4\v\v\x86\x45\a\t\x87" 
  "\x81Q{\x9e\x62\x83;\a~\x0\f\f\r\n9\r{\n\x1\x39\v\x89<\x3\r\xa6\x38\t\x99" 
  "\x44\x5\f9\xba\xa1J\xbf\xc0[\xa3\x39\x92\xa4\xab\x38\xe~\xaf\xc6\x10>\a" 
  "\xbd:\x3\xc2\x42\xc9\xe\xd5\xc3\x44\xd8\xda`\xb0\?\x6\xc2\a\xc9:\t\xd2" 
  "\x38\xdf;\x5\x1\xb7\x39\xab\f\x4\x4\x99\x4\x1\v\x4\n\a\xa0\n~\f\xf\a\x6" 
  "\xe4p\xd4\xb2\x17\xa0@-C\a\x1\xd0\xb3\x87\xf\xd4\xb4\\\vz\xc9S\x0Oa\xbd{" 
  "\xf9N\x1dx\x6 \x1\x4^\xf6\x16\x18l@\x90\x97\?\x80;\xfe\b<\x98\x64 A\x9e" 
  "\x81\xf4\xf6L\xacx\nSD\x97$c\x9e\x8a\x37\xef\x1\xc3\x3\x3\x16\x65\x4\x10" 
  "nJ1\x1c\xe3\x90Ms\x90\xc0!\x0\xa0<\x9a\xf9p\t`A\x1d\xa2\ap\x14\xdd\xb1" 
  "\x95\x87\x81\x6\x0\x96\x11\xcd\xb4\xd5@\xd6\xb1\xea\n\x86M\xa0\xe0\x41" 
  "\xe\b\xab\xccj%\v\x89(\x2\x95\x2\xb3~\r\xeb\xa7+\xd7\xba\xeb\xfa\x82]" 
  "\xd6\xf6\xed*vU\xa5\xed\x15K\xd5\xaa\xd6\xb3",
  (unsigned char *) "r\xd1J\x8er\xf4i@\xcb<\x10<p\x98\x34m;=8*A K\xdak\xb6" 
  "\xc9\x92\xcb\x96\xd6\xe1\xe0,\xae\xba\x44\x39\xa9\xd6\x83\xb8W\x82q\x8b`" 
  "\xf9\xf5\xab\xa3\xeb\x81:]\x13\xc0n\t\xe0\x9b\x81\xab\xbc\x45\xaf\x9em" 
  "\xa5r\xe7\x1c\x9d\x11\x9c\xfb\a\xfd\x32\xd2s8\xe8t\x8c\xbb\xbcZQ\xeb\xbb" 
  "\xbb\xcf\xe5\xea\xfa\xe9p\xbd\xab\x1@@\xb5\x87z\xef_\xdfMC7\xe4\xfb|U" 
  "\x9e\xbe~\xb1M-\xbey\xba\x1e\x11)u\x8a[84p\xce\x3\x9f\t\xfe\x64Ko\b\xe0" 
  "\x83\xc3h\xf4\xa5\x97\x1fQ\xf2M\xa8^\x7f\x87 \xa6P*o\xf5\xc2\x1c\x0\x3< " 
  "\r\x1`)\x83ZQ\xde\x1\xa6VW\x1c>\xd8\vv\xe3\x85v\x16\x84\xfc\xc5\x18\x8fQ" 
  "\xff\xf1`I\xe\f\x18\x10@.\x0\xd0\xb1\a\x3\x1c\x5Yb\xf\x92PbI\x1e\b\x0" 
  "\x45\x11I\n4\xa0J\x94\xaa\xa8\xd3\x80\x3r\xe8@\xe5\x39\x5\xb8\xa4@\x2SJ" 
  "\xb9\x88\x98;0\xf0[#DA\x80\x8d!T\x8eY%R\xe4\x18\xa0\x66\x44[\x1ere\x96" 
  "\xbd\x1dpO\x2r\xd4\xa9\x95\x9a[Ap\xc0\xa0\v4(\xa5\x34M\x6\x95\xca\x2\xf8" 
  "\x84\xe9\xe8*\tp\"H\x8e<\x94\x87\xa4\x2N\xa1\x12\xc4\x1d\xb3\x64\x97\x44" 
  "\x82\?\xf8\x1*\x10\x9f\x39\x45*!A\x80Z\x94\xa9=8\xb4\x0[\x95x\xe4\xc3" 
  "\xa8\xa2P\x9a\x12\x8c\xa9J\xda\xcd\xae\xe0\x9c\x6\x84=9\xec\x38Le8\x10" 
  "\xd0i\x10\x62\xf1\xaa\xec\xe\x8c\xa0t\xc4\"\xd0vC,hC\xfe @\xeb\xb2\xd8" 
  "\x66[\xc4\xb4\xdav\xeb-1\xb6~+\xee\xb8Up\x9b\x1d\xa6G4\xc8\xea\x13\x9cR+" 
  "\x10\x1e\xde\xepU\xe\b\xec\xd1\a\x13\xea\xee \xef\xe\xf5\x2p\xef\x17\xe6" 
  "\xfa\v$\x1f\x3[\xe3#D\a\x14\xfa\x90\x92\x97\x64\x32@4\x14Q\x5\x80\x2\xe" 
  "\x4\x90I\x1\v\xd4sJ\x3\x16\xbb\xd3\xc0G\nd\f\xa1\x1\xf\xb0\xe9#\x84p\xb5",
  (unsigned char *) "\x15\xce\x1%\x9b\xe5k\xe\xbaj\x5\xcb&I\x80\xd2\x65\x35" 
  "\x31\x13\x35s\xce\xbb \x11\x30jD\xf0\x16\x44\x0\xe4\x44iJ-\xed\x0\x89" 
  "\x34\xbdP\xe1\x90q\x9c\x19:\xf5\v\xd1\xf4\xc2\x46u\x90\xf\x44\x90Hd\xda!" 
  "\xdb\xa9v\xbc$\xa1\x8b\xd0\xe|]G\xd8H\xbc\xec\xc3\xcf\x42\v\xd1\xf6\xfW" 
  "\?\x98I4\xfa\x1a\xf2\x0\x9e\xc5\xba\x6\x90\x86\xfa\xc1V\x0\xdeW\xbf\x83" 
  "\x5\x39\aH\xe0\x8b\xde\x42 Pdh\x8c\x1f\xc1M\x8c:(\xcel\xe3\x46<nDe\x92" 
  "\x18\x10\xa0\xe6\x18\x81\xc2@H\xfe{d\xbe\xb9!\x4\xb8\aD\xdc\n\xb9\x15XT" 
  "\xab\xef\xa0\x16\x85\xafj\xa5\x3=9\xcc\x8b\xce*\xd4\xb8\x9e\xcc\xd2\x8f" 
  "\x1d\xa1\xb0\x1e~\xcc\xa2\x12K\x12\xc3#\xf\xe\xfd\xfc\x13\xd7O\xae\xder" 
  "\xef\xb1>|n\x89I\xca\x83\b\x11\xae:\x1c\xb5\xce\x1eLa5\x1e^ \xe6\xa3V" 
  "\xf7\x45\x31:\x4\xea\xcdH\x15\x95\xadW\x1b\x82\xc0\x95\x14\xea b\x90\xb6" 
  "\x17\x37Z\xc1\xe8P\xe2\x0\x82y\xeaPH8\x0\x4 (\b\x80\xbd\xde\xa8\xc8\xf" 
  "\x8d\x41\xce\xd8 \xa3\v\x2&\x1|\xf\x93\xcc\xf6\xd6r\xbap\x85\xc5R\xb3" 
  "\xb9\r\xb4\x2\xd0\x9a\xbf<g\x1a\xd1\xba\x9d\x96T\xa7!\xffM0r\xafsX\x0r" 
  "\xd7\x1b\xb0\x30\xc0T\xed\xf3\x17(\xdaW\x9e\xc8\x18\x61q=\xf0\xcdU\x94" 
  "\x3\xb9\xf\xa9\'\t\x1a\xcc\x8d\x64:x>\v\xda\x30\x46\x45\x31\x9d\xf7z\xc3" 
  "\x96\x5\x95\t\x80\x83\xd3\x1&\xae\xc3\xaf:\xd0M\af\xf2\x45\xedH\x92\x96}" 
  "\xe9\xe\x8b\xa6\b\xfe\xdc\x7f\x8e\xd8 h\x99q\x1f\xbeX\x97\x85\x9e\x62" 
  "\x88\xfd\xcc\x86;\x90#\x8a\x1a\x81\xa0\x44\xb4\x30\xc2Rp\xb3`\xeb\xe4" 
  "\x30\x1b\x12\xd5n\x8fhq\x0\xe\xd7\x46\x8e\x87\x1d\xcd\x89Z\t\x1e\xff\xb2" 
  "\xd3\x80Y\xfc\x62P<\x80\xc0\x9b\xb2\x17\x90\"\x89\xd1\x15K<\xd7\x19\x43" 
  "\xa8\a\xd8\xe4PEw\x98\x11\x8a\xe8S\xa3\x43xR",
  (unsigned char *) "G\xd0\xf3\xa3\x89P\x4\xc8\x1b\x35\xc1\x82\x1a\xc1\x46P" 
  "\xc4\xd4\xa6\x34\x39 @f\x92\x65\x94p\xf3\xa3\x39\x86,\x0#\xfb\x9d@&Q\tY" 
  "\x18\xc3@\x14i\xda\x97\x2\xc0%\x9e\x15\x46K\"\x3\xa0\?&\x96\xb1\xfd\xbc" 
  "\x8f\x1\n\x98\xc0W,f\x9d \f2O{\x92\x43\xa2\x9e\x4\x89\x2\xdc\xa9M\xb5|\v" 
  "\x10n\x93\xc3\x39\xf5\xc2\x9cX\x8a%\x9a\"\xd5\x83i]\xab\x13\x91\xe3\x42" 
  "\xbbh\x1\xaf\x9a=\xa1\x9fK\xa0I\xe\ra*P\xdd\x13yx\x8b\xca\xf\n\x9a\xcf" 
  "\xa1\xc1\x92\\\x10\xfd\xa1\xf\xde\xc6\x84o\x9e\v\\\x11\xcdh\xfe\xf \x0" 
  "\xaa\x66u\xb3\xa2\xa3\x82\x1e\x15~\xa6QlY\x8b\f\'5\x3IK\xca\xd2\x88\x12" 
  "\xab]s\xdc\x42\xbe\xa4\x30\xd3%\xd4\x94^\xa6\x38hK\x97P\x19M\x15\x2\x8f[" 
  "\xb0\x19\xfe\xf0y\x84\x9b\xf9 \xa6\xfe\x1bjUd\x92P@ \xe2\x10\x8b@S\x1a" 
  "\xb4\xe7\x9a&\x8d\x61l\xd9P[\xaf&Z\x4\xa1I\x8a\xa3\xc0@\xc5$[1\xb1\x87" 
  "\x62\xd4\am\xb9\x85V\xa5`9\xb2\xadu\xa0=\xb0\x9c\xdb\x84Q\x88\au\xe3\x39" 
  "\x62Q_\x19\x9cS\xb2\x80HO$5\x99^J|\x82\x91\xa0\x34\x4\xa1\xc7\x1bP\xf5" 
  "\x38w\xd8\xe4\xa1\xe4\xaf\xbc\xa0g\xe9N\xc9\xd8\xa1\x38\xb6\x12\x84\xc5" 
  "\x87\x61\x87\xc2\x9b\xb2\x81h^*\xf9\xc9\x66=\a:\x4\x0\x84\x0\xcf\x30\xad" 
  "h\x10\xab\x8b\xcfi\x8e\x1\x9c\x80lux`\xe\x35\x10k\x9b\x1\x90\x3\x4\xf5" 
  "\x44\xa0\b\x92\xa7w\x92I`\xf\x19h\xa3\xde\x66\x5i\x14\xf1\nq\x87\v\xdc" 
  "\xd9\xfc\xcf\x65\xd0%\vd\xfe\x88\xbb[\x3\x64\t8\xd7\xc5\x45\x1e\x1cS\x1c" 
  "\?\x4h\xb7\xb3\x35\x6S\x90z\xd6TU\f\x1\x41|Ez)\xc5\x9c\xae\xf0p2\xed\xcd" 
  "\xc4z\x8b\xd5\x80\xa6\x96\x12\xbe\xfd\xf1\x8b\xe4:\xc2\x15\xf1\xac\x37" 
  "\x44\xbfQd\x80\xdd\x81\xdaL\x18\xa7\xe\xf3\x85Sf63U\v.B\a\xaaP",
  (unsigned char *) "b\x1d\xfb\x8b\x44\xf9\xc2\xd1\x8e\xa5\xfc\xce\x84\x9b" 
  "\xf4\xcd\xf\xc5\'\xbe\xe3)#6\x16\x1\xab\x64\x80X\x89\x5\xe0\x85\xa0\x64H" 
  "\xa4\xb3\xb8\x31\x13\xf6H\xec\x84\x31\x13\xa4\xe9\xbc\xb5\\\xe\xfe\x85-T" 
  "\x19\x16\x1e\xd7/\xc3l\x12\x65\x84(\x4\x62\xeb\x1e\xa9\xe\xa6U\xcfU\\" 
  "\xe9\x61R\x82\x18-e\xc4\x8dUB\bb\x1f\x1b\x0\x14`\xb9r\x81&&\xe4S\xa4\xe4" 
  "\xc8\xe1\xed\a+\n\xe8\x8d\x1f|\t \nX\xcf\x9fn\xe9!w2\xeb\x0\x8fj\xd3\x38" 
  "\x17\xe5\xafs\xd2\x92L\xf0\xe4\xa3;\xfb\x1\x89\x85t\x89\x13\xe8$S\x9d" 
  "\xb1\xd4(7\x19:\x9d\x12\xc5\x61\xa1\x3\x88]%9\xb1Ysq\x0\x8e,\b\x80\x64\'" 
  "\xd1YP\x84\xea\x8b\x9b{\xf4#\xf7\x39`H\x16\xf5\xc2QN\xba\x88R\xa9\xc3" 
  "\xa6=8h;\xc8[\xad$x1\xd4\x9b\xf2\xd4-D\xc5/)\xc2\x8a\x0\xb2r\x17\x1c\xd0" 
  "\x65[\xb3\xee\xf4\n\x9e\xf5\"\x19\x80U\xacS\xf2j\xa5\xbf\x86\x42\x32x]" 
  "\x86\x33v\v\xd9\xc9\x8e\xb6\xb2\xa0-\xedj\xf\xcb\xd7\xd6\xce\xf6\xae\xa8" 
  "\xad\xedn\xaf\x81\xdb\xde\xe\xf7\x19l@\xeer\x9b\xfb\xdc\xe8N\xb7\xba\xd7" 
  "]\x82\x10\x0\x0;",
  NULL } ; /* --- end-of-image13[] --- */
/* ---
 * image14[] contains a 1444-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (14) convert ran but failed:\\See mathtex.html\#message14}
 * ---------------------------------------------------------- */
#define	IMAGETYPE14 1			/* 1=gif, 2=png */
#define	IMAGESIZE14 1444		/* #bytes in image14[] */
static	unsigned char *image14[] = {
  (unsigned char *) "GIF89a\xcd\x0(\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0\xff" 
  "\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb\xdb" 
  "\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff\x1\x1\xff==\xff\x16\x16\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4\x1\x0" 
  "\x0\x0\x0,\x0\x0\x0\x0\xcd\x0(\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa\xael\xeb" 
  "\xbep,\xcf\x30`\xdfx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\x41\x11r\xc9l:" 
  "\x9f\xd0\xe8P)\xadZ\xaf\xd8\xec\x91\xaa\xedz\xbf\xe0%7L.\x9b\xc1\x63\xde" 
  "\x80 D<\xce\x65\x84\xa2\x0\x9d\xfb\xe4\xf4\xbb\xdd\x9a\xd6\x15\f7\x7f\x3" 
  ":\v\n6\x3\vp~X\x5\t\x80\x42y~\t\n\x1;\to\x8d\x8f=\x99W}9\a\b6\f\f\r\x86" 
  "\x38\x3\xe\xa6\x0\v\x83\x8a\x37\x9aV\x6\xb0\?\xb3\xaf\x80\x91\x38\x4\xad" 
  "\xb2@\xbc|\x95=\x3\t\x9f\xaa\x0\b\f\a\xaa\x3\x10\xae\x36\xf\xb5Q\xbe@" 
  "\xcf<\xd2>\xd6\xd5\xd0N\x9e\xb6\xc4\x38o\xc9\x38\xc0:\xc2\x6\v\f\x0\x4" 
  "\x6\n\fl\x4\x1\v\x4\n\at\xc8\xcc\x0\t\x10\x5\f\xf1\v\x5\x3\r\xe2\x5\xd8" 
  "\xe7\xe0\xc0\xba\x1c\x0\x5\xd6#\xa0\xce\xc6\xbbx\xf3p\xd9\x30\x90\x80!+" 
  "\x0\x0\x1\x65\x4\xc0\xa0\xe0\x41\x1c\xf3\frd\xf8HA\x83G\xbe\xfe\xf8\x11" 
  "\xf0\x87\xb1\x10\x3G\x13\xb5m\x19\xb7#\x1c\xe\x9b\xe9\x42\xe1\x4p\xa0" 
  "\x15\x8e\x2\x3\x1<\x98\xe4\xe0\x6\x4\x43\x6\xe\xc4\x9c\x98\b\x80\x1\x4" 
  "\x4\x8a\x62Tj\xa0\x81\xd0P\xd8^Y}\x10*\xc1\xa0\x5o\x9c*uZ\xcb\x9c\r\xa0X" 
  "Q\xaa\xcd\x66\xc3\xeb\xaa\xb0\xbexE=\x4*@(\x9ek\xa1p\xb3\xb1\x93\xa7\xa9" 
  "\x2\xa6v\xf6\x15:\xb6mS\xa7\xc3\xe2>B\xcb\x11_2\x5",
  (unsigned char *) "\x94\xc8\xe2\xc8\xba\x14$\x1\by\xb3J;\xf0\x46qe\x1d" 
  "\xd2\x14\\\xce\f(\xc1\x63J\xf\n\x8b\xfc\xe5\xa3\xafMu\xb2\x1c@@\xc7\xb7" 
  "\xd8\xc4\xc2<\xf\'\x95\xcc\x1b\xc0l\x6tVw\xeb\x66\xbb\x37>\xa4\xa4\x41k" 
  "\x12\xe9Yrq_\x93x{\x16.\xf6\x6\xf5({W\xd1\xb6^|\xa7\x83\xbb\?\xed:$e\x14" 
  "]s\x8c\xeh\x13\xb0\xea\xcc\x38\xaf\xb2\x9a\xe6\xd9\xc0\f\xe8\xfc\xab\xa6" 
  "\b\x82\xfa\xc2\xbc\x14>\x0\xf9\xbe\xbd\xb7\xd6z7<P\x80Tx9\xfe\xc4\x86^4" 
  "\xe9\xb0\xd2\r\f\x18\x10\x0LgI\x98\x0m\b\xb0\xa7\x3\x32\xcf\fb\x0\x4\xcf" 
  "\x0\x62R)#\xaa\x82\xd3\x87\xf\x9c\x33\xe2v\x5\x34\xf0\xc0\x82\x37\xach" 
  "\x3\x2=\xb1\x13P\x89%\xe6`\xc0\x3\n,\xb3`#\x3(\x90@)\x0\xb4\xf8\x62\xe+" 
  "\xd2\x18$)\x85\x34@\xa2\x93H\x81x\xeG\x85\x44\x15N\x2\xc3\x30\xf8\x3nB" 
  "\x8crG\xe\x12\xf5\x0^ \?\x8c\xf9\xe5\x11\x61\x86\x62\xa6\x99;\xb0\xb9I" 
  "\xejb\x91]:\xdb\x5\x81@\x96\xcd\xe4\xa9\xa7\x96\?\xe8\x32\x4W{\x6*\xe8LA" 
  "\x84\xc9\x3\x2n\xe\xaa\xe8\xa2\xe2\x34\xc8\xe8\xa3\x90\x62\xe7h\xa4\x94V" 
  "Z\xc4\x9c\x80\rbh\x17xD\xd1)\x13\x9f\xe2\x80\xe8\x8c\x96&1)\x84\xe8\xfc" 
  "\xc1\xa5\x16yp\xb2\x88\x11\xae\xbe\nD\xac\x37\xb0\xc4\x90\xa2\x82\x10R\\" 
  "\xa3\x9b\x14\x46\x63\x18\x9aP&\x13-\xda\f\xab\x63-YB\x90\xa8\"\xa3\x10yJ" 
  "*>\xec\xa5\xc0w\xaf\xfe\x80\x41\xcdg\x5\x1a\xdb\x3\x65\xd7\x6\x81\xcd" 
  "\x1f\xf3\x31\xba\xd3\x31\x83\xf1\xda\xc3\x1\xe|$J\?\xf5\xb0\x9b\x43T\x10" 
  "\xd5\x18\x91($\x89\xe2\x11R\xf1\xd6s/\x95+\xed\x83\x65:\a\x1c\xc6\x94<" 
  "\xf4\xd8k\x90h\xe\xc4\x1b\xe4\xbc\xc6\t5\xc8\x0\x61\xa5\x93\x30\xc1\v" 
  "\x17\xcc\xaf\?\b\xacs\x99\x31\x1a\xdf\xd3NC\xeb\x9a\xf3\xd2\xc5\x91\xec" 
  "\x4\xce\xae\x37\xcc",
  (unsigned char *) "Y\x95\b\xeeH5\xc0\x1sM\x5\xdaX\xbb\xf1\xe6\x16X\x9f" 
  "\xd5\xdc\\\xcc/c\xd4\x80\x1cu\xde\x96s\xb0\x34\xd3\xbc\xd6\x1f\xb2\x18" 
  "\x94\x94,(\x15\xbd\x14\xcf\x6-\xd8\x99\xd4m}\x15\x96xSB\xcd\xdd\r\x4\xe8" 
  "\x84\xb2\rs\xce\xf8\x80]\xa6\x41\x46I\xd9\x90\x39\xda\\h\xa3\r]\x19t\xa7" 
  "\x1\xb3\x1e\x8c\x93\xe5\xd5\xdb\xdav\x1bs\xf\x9eo\xaf\x85\x36%\xa8p\x16J" 
  "\xe0\x80\xda \x1a\x7f\x0\x0\xb3\xa3\x63\x66\x8f\x63\x13`\xb5\x45{\xea" 
  "\x7f\xc5\x94\x42\xddu\xc7\xf6\xdd\x16rKq.\x9d\x88\xfe\x98\xd3xO\xe6w\x8b" 
  "\xf8\xb9\xe6r(\xf0\fd\x93@\xa6\xb9\x64\xd4\x15\xb0\xf\x4J\xc9\xce\x0\xed" 
  "\x9b\xf3\x16\xf\xc8\x98\xfb\xe5\x10\xd3\xb2\x5m\xee\xe\n\xc0\xf2\x33\x81" 
  "\xce /\x94r\x9a\x3H\x9f\x64\xf5%g\x80\xf2od\xec[X\xb7\x9e\xde\x9f\xf6" 
  "\x9e\xa5\xfe\x18Xf\xbf.\x97\x86\xcf\xe4\x61\x95\x1\xe6\xff\x37\xd6\xf3" 
  "\x41S\xbf\xb5\x37=H\xeb\bd\xb3M$\xa5y\xf7#y\xc0\x93\xfc+\xa0\xa4\x8d\x89" 
  "\x30\x92<\xa0\x94#\x1:\xe5~\x1dI\xc4\x43\x1a\x31\f\x1c\x41\xe9,.\"X\xff" 
  "\x1c\xe8,\xdf\x84\v$\xfb\x83\f\x1\x1f\x88\xa2s\x98\x63\r\x9d\x61\x5\x1" 
  "\xaaW#&\xf9\xe6\x0(\\\x0V\xf2\x17\xa1\ti\xe2\xf\x13\x12^\xca&7*\xc8\x98I" 
  "\"\x9b\xb2S\x9b\x80\x30\xa6\x1c\x12\x61Y>\x80\xd8\x5\x61\x45*\xf0\xc4IT8" 
  "X@\xeb\b\x90\xf\x32mcr\xa5\xca\xd3\x3\x1e\x16\xb1\x30\xc4\x83k\x2{B\xd8" 
  "\xa2\b\aS\xec\xa1\ffs&]\x15\xb6\xc8\xc5\x32\x46\x8a\x8c\x66L\xa3\xa2\xd0" 
  "\xa8\xc6\x36\xea\x89\x8dn\x8c#\x1ch@\xc7:\xda\xf1\x8ex\xcc\xa3\x1e\x65" 
  "\x10\x2\x0;",
  NULL } ; /* --- end-of-image14[] --- */
/* ---
 * image15[] contains a 1870-byte gif rendering of expression
 * \usepackage{color}\color{red}\footnotesize\fparbox{
 * (15) Can't emit cached image:\\check permissions.\\
 * See mathtex.html\#message15}
 * ---------------------------------------------------------- */
#define	IMAGETYPE15 1			/* 1=gif, 2=png */
#define	IMAGESIZE15 1870		/* #bytes in image15[] */
static	unsigned char *image15[] = {
  (unsigned char *) "GIF89a\xcd\x0\x38\x0\x84\x0\x0\xff\xff\xff\xff\x0\x0" 
  "\xff\xff\xff\xff\x99\x99\xff\x64\x64\xff\xb8\xb8\xff--\xff\b\b\xff\xdb" 
  "\xdb\xff\xe\xe\xff}}\xff  \xffOO\xff\x3\x3\xff\x16\x16\xff==\xff\x1\x1" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" 
  "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0!\xf9\x4" 
  "\x1\x0\x0\x0\x0,\x0\x0\x0\x0\xcd\x0\x38\x0\x0\x5\xfe` \x8e\x64i\x9eh\xaa" 
  "\xael\xeb\xbep,\xcf\x30`\xdfx\xae\xef|\xef\xff\xc0\xa0pH,\x1a\x8f\x41" 
  "\x11r\xc9l:\x9f\xd0\xe8P)\xadZ\xaf\xd8\xec\x91\xaa\xedz\xbf\xe0%7L.\x9b" 
  "\xc1\x63\xde\x80 D<\x82\x5\xc5\xc0V \x16\x10g$BQo\xf3\xf3<i:\x5\x6\x37" 
  "\x84s6\f\xe\x6\v\f6\x3\v>\x3\t\xf\n\x4\v\v\x85\x43\a\t\x86P\x94\x9eP\x5" 
  "\t\x9ap\xa4:\xa0g\x82\x39\ax\x0\f\f\r\n7\x6\t\ao7\v\x88:\x3\r\xae\x36" 
  "\xa7\x43\x5\x8e\x36\xa5M\x4\xbb\xc6N\x6\xca\?\xcc:\xc8y\xab\x37\x93\xac" 
  "\xb3\xc5;\x3\xe<\a\xc4\xd4\xcd\?\xd7\xf\xe0M\xe3R\xcf\x43\xe8\x80\x39" 
  "\xd3\xc5\xca\a\xd7\x0\x6\n\x96\xec;\x5\x1\xbf\x37\xb3\f\x4\x4\x9a\x4\x2," 
  " \xa0\xe0@\x1f\x5x\x18@80O\a\x83\x81\v\n\x10\x80\x30\xb0\xe0\x0z\x6\x1(h" 
  "PH!\xc3x8&1r\xd4\xef\x1f\x35R\x8d\xfe\xe4U\xcc\xf8\nb\x1dH\n\x18\x4\xdb" 
  "\xc7\x11\xc0\xc4\x95\x17\v\xf6)Y\xea\xe1\xc8N\xf/\xd5Q\xc7.\x80\xfx\xd6n" 
  "0\xa8\xa3 \xd2\x8d\x3\xbb\xf6\x19\xe5\x91`\xce\x2\\\x6\xe\xb8\xd3\x41" 
  "\xf4\xc6\xc4GZ\xb3\x16\xd3*O\x13\xba\xae\x86\x2\xd4y\xd0\xa9*\x80\xab\x0" 
  "\xf0\xad\xed$\xb6\xacM\b`\x11\xe4\xb3\xc1\x90\xabY\xb2u\xd1\xb9\x85\v`o" 
  "\xca\xaf\x0\x6\x84%\xd7\xe\x0R\x1c",
  (unsigned char *) "\x8f\xd9\xf5q\f\x92\xce\xde\x1c\a\t,\xdaj7\aZ\x0\xb6" 
  "\xe8)0z\xd6\xaci\xce:\x1e\x90\xc5\x61i\xb3j\xcf\xa7\x43\xd3\v\xf0\x9ao" 
  "\xb3\xd2\x9c\xd5\xb5\xd6\x34\xd5\xc0\x1b\xd9\xa3\x7f\x34\x8e\xcc\xf7\xda" 
  "\xa8\x1b\x1\xe2\x11\x7f\xea\xcd\x86\x1b\xd0\xb3pw\xc6\xf1\xacr_\xea\xa7;" 
  "\xe3\xae,o5\xb0\xe8\x85\xea\xd2:}}\xec\xd3\xdb\xd9\xa5\'\x0oc\xa0I\xc7" 
  "\xe4v4N\t\xf9\x9a\x36\xe7\x97\x1@\xd0G\xcd\x17\xf5=d-\x12^v\xd8\xc9\x93" 
  "\x3\x1\r\xdc\xfe\xf0\x86t\f\x86\x37\xc8\x65\x4\x5\xc8\f>\xae\xb0\x81\x1b" 
  "\x82\n\x16\x80\x97m~\xe5vZA6\b\xf8\xca\x81\t\xda\xf0\x86\?\x81L\xc5\xc3%" 
  "J\x19\x10@0\xf\xd4\x41\x1f\x0\b\x94\x98\r%\x96\xe8\x42#T15\xb0\xc0\x46" 
  "\xb2\x0YY\x1\r<\xc0\x86g\xe<\xd0HAA6\xd0\xe4\x93\xb3\x10i\xe4\xe\f\xdc" 
  "\x62\xc0\x0\b\xf0\x18K$U\x8es\x91\x93\xf4\x80)O\x92)=d\xc9\x42 \t\xc9" 
  "\x64\x98Pfy\xd1\x96\x0\x38p\xc0\x9c\v\xe0\x61\x0\x99\x8e$\xd0\x89|*r\xf3" 
  "\xc3\x45\xfa\xc0\xf2\x43\x1cQ\xd1x\x4\x7f\x38Lf\x4\xa2\x38\xe8\xc3(\xa3" 
  "\x83\xe4\x80\a\xa4\x44\xe8\xb3\xc0z\x96$\xb0\r\x1dI\xf4\xb9\x3\x1\xcd\x1" 
  "\x81\xc0\x9e\xeb\x94jj\xe\x3y\xe5\xd4\x14\x9e~Z\xa8\xf\xfPz\xea\xac\x65" 
  "\x88\x46\x8f\x11\x8d\x85\"\xaa\xac\xb4\xf6\xea+\x9f\xbf\x6+,\x1a\xad\xek" 
  "\xec\xb1M\xe4\xda\xe8\x1f\x95\x32\x8b\xec\xb3\x66(k\xc8LB\xfe\x8c\x12" 
  "\x9f\x16\xa9\xfc\x90-\xb4\x64H;\x9e\x11\x9fy\x11\r\x10\xe3r\xdbm\xb1=" 
  "\x84\xeb\x83\xba\xe6\xb6\x1b\x45\x63\"\xa5\xc4H\x84;\xb9\x94\x18J\x8e<C@" 
  "y6LDI-\xbb\x4\x15Q/\x3\xa9uS\x84\x39\x65\xb4Q!\b\xcc\xa3\x19\x8d\xe;\xb0" 
  "p\xbf\xf3\xf4\x63\x93@\xf4\x42\xdc\x9a\xbb\xb8\xa2\x1b\x97Z\x0\xb0\xd5" 
  "\xddV\x88)&W\xc8t\x15",
  (unsigned char *) "\xf2\xe3\xe\x8cX\x86G\xc9\x61%\x18\xeb\xc8\x34\xe3\x66" 
  "\xc0\x91\v\xe2l\x97\x2\x1b\xc6\x19\x1d`\xe1\xe9\xcc\x31\x11\xed\xd4\xf6" 
  "\xad]\xc0\xd1\xe6\x9dJ\xcb\x1d\xed\xd8o\xf0\xcc\x36\x9dv\xe9\x15\x32\xc0" 
  "\x42\x33_}\x8b\x9d\x85$\xb0j-T\'\x86\x35\xaf\x43\x17\x95\xee\xd2\xb8\xf1" 
  "+^1\xeb\xf9\xd7!\x87\xfcN\xdd\xe0V\x5\f#g\\v/\xe6\xd8\xd7z\xdbU\xb7\"K" 
  "\x97\xed\x43;\x14\xf6\x1b\x36\x86&\x16n\x93]\xfl\xea\x99S\'#\x1e\xb2\xdc" 
  "Uoe@\x1f\t^nC\xe6\x1d\xd9\xe8@\xbe\xfe\xa7i\xe@\x89(\n\xde\x43\x63]^\xa9" 
  "\xa6\x98w*I\x92\x95_F\xfd\xa2\xa2\xdd\x11\x94\xc0\x91\x63\xba\xe$1k\xae" 
  "\xe%#k,\xa8\v\x1\x95\xb0\x9e\x64u`\n\t<\xf1\xc0\x90j\xba\xe\xca\x92\x1d" 
  "\x97\xa4\xe9\x14\x42;\xa7MLz\x83\xf6<\\O\xbd\xf4\xcf\x17\xe6q\x18\xec" 
  "\x86/\xb8\xb7Y(\xc0\fw\xe6\?\x8f~\xfb\xf0\x7f\xf1~\xfc\xf4\x63\x91+\xa1" 
  "\xd3\x97\xb1\x87\xf7z8\xdb\xff\xf5\bp\x5\xf8\x9e\x85:G\x10\"pY\xe8\x83" 
  "\xb5\xeeq\x84\x5\x32\xd0\x14\xcd\x88\x88MpG\xab\x43(e\x11\x33\x2V\xf7V" 
  "\x93%\xf2\x39\x8d:H\b\xd7\xb5`\x83\x8a\x10\r\x10\f\xb0\x90\x5-l\x81\x8b" 
  "\xd3y\x8cg\xbf\x18\x61\x14\xcc\x81\x1a\x5\xc9\x90\a\x9f\xa1\x61\x10\xba" 
  "\x42\x88\x10\t+27\x14\xdfQ \xd0\x10\xa5\xd8K`\xb4;\x98\x45\x30\xb2\x13" 
  "\x7fh\xc2#\r\x99\x97N^\xb1\x90\x86 Q&\x9d\xd8\xd7\xaa\x8a\xb1\x92\x9dT" 
  "\xf1L8a\xfe\x62\rC6\x87\x1\xb4P\x89<\x9a\x62K\x84\xd2\xb0\x8d\xb5\xf1" 
  "\x61\xafp\xa2\x11\x7f\xb2\x46\t\x16\x87\x16\xf4\xa0\xa0\x6\xd3\xd5\x0" 
  "\x11\xb0\x1\x66\x30\xe3\n`\x80\x6\f\xab`\xa5\x14\x81\xc9N {\xb1\x87P\xd5" 
  "\f\x91\x88\x1c\xa4\x87\xe2\xc2\x8c\xac\x30\x83!g\x91\xa4]`v3\x13\xc9\x43" 
  "h\x83\xc1\x85\x61\x1c\x11\xc8;&\x82",
  (unsigned char *) ")[\x84\xde\xf8\xb6\xf7\x80|$-io\x93\x8e\x46\x34\x43" 
  "\xa0\xb9\x81&j\xc1\xb1I\x3\xf4\x38I\xd4\xd8\x92(\b\xd8\x94\xf3\xc2\x86" 
  "\x34\\\x6@kY\x1b\xdb>hi\x83\xde@M4*jZa\xf8\'D\x1e\xdc\xea\x6\xb2\xe0W" 
  "\xdcHH\xcc\xf5\x84\x8d=\xdf\xdc\x66\x96\x1cW\xa0\xe9T\x87\x98\xb8\xd9" 
  "\x83\x2\xc6\x41\xfL]cn\xfc\xfa\xdb\xdd\xe4I\x16o\xa2\xc3=\x9a\xd8&e\xe8@" 
  "\xaa\xe4\xb8\xb0\a\xea\xc3\x81/$g$\x1b\xb5\xf0h\xdb\x91\xd0\x80\f4\xb7" 
  "\x9b\x19\x14\x62q\xc2\x45\xe9\x64\xf9\x8c\x85\xa2\x93\x61\x18\xa1\xc7" 
  "\xd1UD\xd3K}\x19Tt\x99\xc3\x9c\x46\x14:\"\x1c\x10\xf4)\xf6\xd9\x94^(\xd5" 
  "\xe\x5\x90\x82\x1e\x9f+\x6\x9er\x97\xc1\x91\x42IHn\xeaQ$\xa4\x44\x90\xe4" 
  "\x89\x89\xa7\x34\x85\x5\x45.\x16\x11=i\xc4\xa7*\x8cK\x91\"tS\xa4\xc6\x63" 
  "S\xe4\xb4)\x9b\xa6\x1a\xd4\xe5\t/x;z\x93\x8f\xe2\x34\xa7\x3\xd4)\xa8\xaf" 
  "p\x11\x8c\x64\xe4H\x1c\xb4#\x80\x1a\x41\bf\xd6\xaa\x87\x1d\x90M\x1f\xd4" 
  "\xac\xd4\x11\xcc\xe8\xc3\x6:\xc7Pw\xe5\x8f\xa5\x30\x45\x0M\xe9J\rj\x1d" 
  "\xdc*\xebg\x85\a\x94\xf1\xa0`HU\xbfR\xe9\x84\xf9\x11\xb6\t\xd7\xf0_\x18l" 
  "\xc5\xbe\x64\r\xf6\xb1\x98\x95\x82\x63\x33\xcb\xd9\x8eu\xf6\xb3\xf6\xbb," 
  "hG\xcb*\xd2\x9a\xb6\xb1\x34H\xadjW\xcb\xda\xd6\xba\xf6\xb5\x31\b\x1\x0;",
  NULL } ; /* --- end-of-image15[] --- */
/* --------------------------------------------------------------------------
Other Allocations and Declarations
-------------------------------------------------------------------------- */
/* ---
 * aggregate array of embedded images
 * ---------------------------------- */
static	unsigned char **imagetable[] = { NULL,
  image1,     image2,     image3,     image4,     image5,     image6,
  image7,     image8,     image9,     image10,    image11,    image12,
  image13,    image14,    image15,    NULL };
int	imagesizes[] = { 0,
  IMAGESIZE1, IMAGESIZE2, IMAGESIZE3, IMAGESIZE4, IMAGESIZE5, IMAGESIZE6,
  IMAGESIZE7, IMAGESIZE8, IMAGESIZE9, IMAGESIZE10,IMAGESIZE11,IMAGESIZE12,
  IMAGESIZE13,IMAGESIZE14,IMAGESIZE15,0 };
int	imagetypes[] = { 0,
  IMAGETYPE1, IMAGETYPE2, IMAGETYPE3, IMAGETYPE4, IMAGETYPE5, IMAGETYPE6,
  IMAGETYPE7, IMAGETYPE8, IMAGETYPE9, IMAGETYPE10,IMAGETYPE11,IMAGETYPE12,
  IMAGETYPE13,IMAGETYPE14,IMAGETYPE15,0 };
/* ---
 * other variables
 * --------------- */
static	unsigned char image[8192];	/* returned image */
unsigned char **imagestrings = NULL;	/* image1, image2, etc */
int	istring = 0,			/* imagestrings[] index */
	imagesz = 0;			/* sizeof entire image */
/* ---
 * look up requested image (if we have it)
 * --------------------------------------- */
if ( nbytes != NULL ) *nbytes = 0;	/* init for error */
if ( imagenum<1 || imagenum>MAXEMBEDDED ) imagenum=1; /*out-of-bounds check*/
imagestrings = imagetable[imagenum];	/* ptr to array of image strings */
imagesz = imagesizes[imagenum];		/* total #bytes in image */
if ( imagestrings == NULL ) imagesz = 0; /* sanity check */
if ( nbytes != NULL ) *nbytes = imagesz; /* caller wants #bytes in image */
if ( imgtype != NULL ) *imgtype = imagetypes[imagenum]; /* and image type */
/* ---
 * build up images from constituent strings
 * ---------------------------------------- */
if ( imagesz > 8192 ) imagesz = 8192;	/* don't overflow buffer */
memset(image,0,imagesz);		/* zero out returned image buffer */
while ( imagesz > 0 ) {			/* still need more bytes in image */
  unsigned char *string = imagestrings[istring]; /*so get next image string*/
  int	thissz = min2(stringsz,imagesz); /*entire string or image's tail end*/
  if ( string == NULL ) break;		/* looks like a program error */
  memcpy(image+(istring*stringsz),string,thissz); /* concat string to image*/
  imagesz -= thissz;			/* fewer remaining bytes */
  istring++;				/* get them from remaining strings */
  } /* --- end-of-while(imagesz>0) --- */
return ( image );
} /* --- end-of-function embeddedimages() --- */
/* --- end-of-file mathtex.c --- */
