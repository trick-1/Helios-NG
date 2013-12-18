/* $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/ops.c,v 1.1 1993/08/06 15:17:14 nickc Exp tony $
 *
 * Contains routines that implement the operators in vi. Everything in this
 * file is called only from code in normal.c
 */

#include "stevie.h"
#include "ops.h"

/*
 * doshift - handle a shift operation
 */
void
doshift(op, c1, c2, num)
int	op;
char	c1, c2;
int	num;
{
	void	tabinout();
	LPTR	top, bot;
	int	nlines;
	char	opchar;

	top = startop;
	bot = *Curschar;

	if (lt(&bot, &top))
		pswap(&top, &bot);

	u_save(top.linep->prev, bot.linep->next);

	nlines = cntllines(&top, &bot);
	*Curschar = top;
	tabinout((op == LSHIFT), nlines);

	/* construct Redo buff */
	opchar = (op == LSHIFT) ? '<' : '>';
	if (num != 0)
		sprintf(Redobuff, "%c%d%c%c", opchar, num, c1, c2);
	else
		sprintf(Redobuff, "%c%c%c", opchar, c1, c2);

	/*
	 * The cursor position afterward is the prior of the two positions.
	 */
	*Curschar = top;

	/*
	 * If we were on the last char of a line that got shifted left,
	 * then move left one so we aren't beyond the end of the line
	 */
	if (gchar(Curschar) == NUL && Curschar->index > 0)
		Curschar->index--;

	updatescreen();

	if (nlines > P(P_RP))
		smsg("%d lines %ced", nlines, opchar);
}

/*
 * dodelete - handle a delete operation
 */
void
dodelete(c1, c2, num)
char	c1, c2;
int	num;
{
	LPTR	top, bot;
	int	nlines;
	int	botindex;
	register int	n;

	/*
	 * Do a yank of whatever we're about to delete. If there's too much
	 * stuff to fit in the yank buffer, then get a confirmation before
	 * doing the delete. This is crude, but simple. And it avoids doing
	 * a delete of something we can't put back if we want.
	 */
	if (!doyank()) {
		msg("yank buffer exceeded: press <y> to delete anyway");
		if (vgetc() != 'y') {
			msg("delete aborted");
			*Curschar = startop;
			return;
		}
	}

	top = startop;
	bot = *Curschar;

	if (lt(&bot, &top))
		pswap(&top, &bot);

	u_save(top.linep->prev, bot.linep->next);
	/* Don't leave even the potential for orphan marks */
	clrmark (top.linep);

	nlines = cntllines(&top, &bot);
	*Curschar = top;
	cursupdate();

	if (mtype == MLINE) {
		delline(nlines, TRUE);
	} else {
		botindex = -1;
		if (!mincl) {
			botindex = bot.index;	/* where it WAS */
			if (bot.index != 0)
				dec(&bot);
		}

		if (top.linep == bot.linep) {		/* del. within line */
			n = bot.index - top.index + 1;
			while (n--)
				if (!delchar(TRUE))
					break;
		} else {				/* del. between lines */
			n = Curschar->index;
			while (Curschar->index >= n)
				if (!delchar(TRUE))
					break;

			top = *Curschar;
			*Curschar = *nextline(Curschar);
			delline(nlines-2, TRUE);
			Curschar->index = 0;
			n = bot.index + 1;
			while (n-- && botindex)
				if (!delchar(TRUE))
					break;
			*Curschar = top;
			(void) dojoin(FALSE);
			oneright();	/* we got bumped left up above */
		}
	}

	/* construct Redo buff */
	if (num != 0)
		sprintf(Redobuff, "d%d%c%c", num, c1, c2);
	else
		sprintf(Redobuff, "d%c%c", c1, c2);

	if (mtype == MCHAR && nlines == 1)
		updateline();
	else
		updatescreen();

	if (nlines > P(P_RP))
		smsg("%d fewer lines", nlines);
}

/*
 * dofilter - handle a filter operation
 */

#define	ITMP	"viXXXXXX"
#define	OTMP	"voXXXXXX"

static	char	itmp[32];
static	char	otmp[32];


/*
 * dofilter - filter lines through a command given by the user
 *
 * We use temp files and the system() routine here. This would normally
 * be done using pipes on a UNIX machine, but this is more portable to
 * the machines we usually run on. The system() routine needs to be able
 * to deal with redirection somehow, and should handle things like looking
 * at the PATH env. variable, and adding reasonable extensions to the
 * command name given by the user. All reasonable versions of system()
 * do this.
 */
void
dofilter(c1, c2, num)
char	c1, c2;
int	num;
{
	char	*mktemp();
	static	char	*lastcmd = NULL;/* the last thing we did */
	char	*buff;			/* cmd buffer from getcmdln() */
	char	cmdln[200];		/* filtering command line */
	LPTR	top, bot;
	int	nlines;

	top = startop;
	bot = *Curschar;

	buff = getcmdln('!');

	if (buff == NULL)	/* user backed out of the command prompt */
		return;

	if (*buff == '!') {		/* use the 'last' command */
		if (lastcmd == NULL) {
			emsg("No previous command");
			return;
		}
		buff = lastcmd;
	}

	/*
	 * Remember the current command
	 */
	if (lastcmd != NULL)
		free(lastcmd);
	lastcmd = strsave(buff);

	if (lt(&bot, &top))
		pswap(&top, &bot);

	u_save(top.linep->prev, bot.linep->next);

	nlines = cntllines(&top, &bot);
	*Curschar = top;
	cursupdate();

	/*
	 * 1. Form temp file names
	 * 2. Write the lines to a temp file
	 * 3. Run the filter command on the temp file
	 * 4. Read the output of the command into the buffer
	 * 5. Delete the original lines to be filtered
	 * 6. Remove the temp files
	 */

#ifdef	TMPDIR
	strcpy(itmp, TMPDIR);
	strcpy(otmp, TMPDIR);
#else
	itmp[0] = otmp[0] = NUL;
#endif
	strcat(itmp, ITMP);
	strcat(otmp, OTMP);

	if (mktemp(itmp) == NULL || mktemp(otmp) == NULL) {
		emsg("Can't get temp file names");
		return;
	}

	if (!writeit(itmp, &top, &bot)) {
		emsg("Can't create input temp file");
		return;
	}

	sprintf(cmdln, "%s <%s >%s", buff, itmp, otmp);

	if (system(cmdln) != 0) {
		emsg("Filter command failed");
		remove(ITMP);
		return;
	}

	if (readfile(otmp, &bot, TRUE)) {
		emsg("Can't read filter output");
		return;
	}

	delline(nlines, TRUE);

	remove(itmp);
	remove(otmp);

	/* construct Redo buff */
	if (num != 0)
		sprintf(Redobuff, "d%d%c%c", num, c1, c2);
	else
		sprintf(Redobuff, "d%c%c", c1, c2);

	updatescreen();

	if (nlines > P(P_RP))
		smsg("%d lines filtered", nlines);
}

#ifdef	TILDEOP
void
dotilde(c1, c2, num)
char	c1, c2;
int	num;
{
	LPTR	top, bot;
	register char	c;

	/* construct Redo buff */
	if (num != 0)
		sprintf(Redobuff, "~%d%c%c", num, c1, c2);
	else
		sprintf(Redobuff, "~%c%c", c1, c2);

	top = startop;
	bot = *Curschar;

	if (lt(&bot, &top))
		pswap(&top, &bot);

	u_save(top.linep->prev, bot.linep->next);

	if (mtype == MLINE) {
		top.index = 0;
		bot.index = strlen(bot.linep->s);
	} else {
		if (!mincl) {
			if (bot.index)
				bot.index--;
		}
	}

	for (; ltoreq(&top, &bot) ;inc(&top)) {
		/*
		 * Swap case through the range
		 */
		c = gchar(&top);
		if (isalpha(c)) {
			if (islower(c))
				c = toupper(c);
			else
				c = tolower(c);

			pchar(&top, c);		/* Change current character. */
			CHANGED;
		}
	}
	*Curschar = startop;
	updatescreen();
}
#endif

/*
 * dochange - handle a change operation
 */
void
dochange(c1, c2, num)
char	c1, c2;
int	num;
{
	char	sbuf[16];
	bool_t	doappend;	/* true if we should do append, not insert */
	bool_t	at_eof;		/* changing through the end of file */
	LPTR	top, bot;

	top = startop;
	bot = *Curschar;

	if (lt(&bot, &top))
		pswap(&top, &bot);

	doappend = endofline(&bot);
	at_eof = (bot.linep->next == Fileend->linep);

	dodelete(c1, c2, num);

	if (mtype == MLINE) {
		/*
		 * If we made a change through the last line of the file,
		 * then the cursor got backed up, and we need to open a
		 * new line forward, otherwise we go backward.
		 */
		if (at_eof)
			opencmd(FORWARD, FALSE);
		else
			opencmd(BACKWARD, FALSE);
	} else {
		if (doappend && !lineempty())
			inc(Curschar);
	}

	if (num)
		sprintf(sbuf, "c%d%c%c", num, c1, c2);
	else
		sprintf(sbuf, "c%c%c", c1, c2);

	startinsert(sbuf, mtype == MLINE);
}

#ifndef	YBSIZE
#define	YBSIZE	4096
#endif

static	char	ybuf[YBSIZE];
static	int	ybtype = MBAD;

bool_t
doyank()
{
	LPTR	top, bot;
	char	*yptr = ybuf;
	char	*ybend = &ybuf[YBSIZE-1];
	int	nlines;

	top = startop;
	bot = *Curschar;

	if (lt(&bot, &top))
		pswap(&top, &bot);

	nlines = cntllines(&top, &bot);

	ybtype = mtype;			/* set the yank buffer type */

	if (mtype == MLINE) {
		top.index = 0;
		bot.index = strlen(bot.linep->s);
		/*
		 * The following statement checks for the special case of
		 * yanking a blank line at the beginning of the file. If
		 * not handled right, we yank an extra char (a newline).
		 */
		if (dec(&bot) == -1) {
			ybuf[0] = NUL;
			if (operator == YANK)
				*Curschar = startop;
			return TRUE;
		}
	} else {
		if (!mincl) {
			if (bot.index)
				bot.index--;
			else		/* already first column */
				bot = *( prevchar (&bot));
		}
	}

	for (; ltoreq(&top, &bot) ;inc(&top)) {
		*yptr = (gchar(&top) != NUL) ? gchar(&top) : NL;
		if (++yptr >= ybend) {
			msg("yank too big for buffer");
			ybtype = MBAD;
			return FALSE;
		}
	}

	*yptr = NUL;

	if (operator == YANK) {	/* restore Curschar if really doing yank */
		*Curschar = startop;

		if (nlines > P(P_RP))
			smsg("%d lines yanked", nlines);
	}

	return TRUE;
}

/*
 * doput(dir)
 *
 * Put the yank buffer at the current location, using the direction given
 * by 'dir'.
 */
void
doput(dir)
int	dir;
{
	void	inslines();

	if (ybtype == MBAD) {
		beep();
		return;
	}
	
	u_saveline();

	if (ybtype == MLINE)
		inslines(Curschar->linep, dir, ybuf);
	else {
		/*
		 * If we did a character-oriented yank, and the buffer
		 * contains multiple lines, the situation is more complex.
		 * For the moment, we punt, and pretend the user did a
		 * line-oriented yank. This doesn't actually happen that
		 * often.
		 */
		if (strchr(ybuf, NL) != NULL)
			inslines(Curschar->linep, dir, ybuf);
		else {
			char	*s;
			int	len;

			len = strlen(Curschar->linep->s) + strlen(ybuf) + 1;
			s = alloc((unsigned) len);
			if (!s)  return;
			strcpy(s, Curschar->linep->s);
			if (dir == FORWARD)
				Curschar->index++;
			strcpy(s + Curschar->index, ybuf);
			strcat(s, &Curschar->linep->s[Curschar->index]);
			free(Curschar->linep->s);
			Curschar->linep->s = s;
			Curschar->linep->size = len;
			updateline();
		}
	}

	CHANGED;
}

bool_t
dojoin(join_cmd)
bool_t	join_cmd;		/* handling a real "join" command? */
{
	int	scol;		/* save cursor column */
	int	size;		/* size of the joined line */

	if (nextline(Curschar) == NULL)		/* on last line */
		return FALSE;

	if (!canincrease(size = strlen(Curschar->linep->next->s)))
		return FALSE;

	while (oneright())			/* to end of line */
		;

	strcat(Curschar->linep->s, Curschar->linep->next->s);

	/*
	 * Delete the following line. To do this we move the cursor
	 * there briefly, and then move it back. Don't back up if the
	 * delete made us the last line.
	 */
	Curschar->linep = Curschar->linep->next;
	scol = Curschar->index;

	if (nextline(Curschar) != NULL) {
		delline(1, TRUE);
		Curschar->linep = Curschar->linep->prev;
	} else
		delline(1, TRUE);

	Curschar->index = scol;

	if (join_cmd)
		oneright();	/* go to first char. of joined line */

	if (join_cmd && size != 0) {
		/*
		 * Delete leading white space on the joined line
		 * and insert a single space.
		 */
		while (gchar(Curschar) == ' ' || gchar(Curschar) == TAB)
			delchar(TRUE);
		inschar(' ');
	}

	return TRUE;
}

void
startinsert(initstr, startln)
char	*initstr;
int	startln;	/* if set, insert point really at start of line */
{
	register char	*p, c;

	*Insstart = *Curschar;
	if (startln)
		Insstart->index = 0;
	Ninsert = 0;
	Insptr = Insbuff;
	for (p=initstr; (c=(*p++))!='\0'; )
		*Insptr++ = c;

	if (*initstr == 'R')
		State = REPLACE;
	else
		State = INSERT;

	if (P(P_MO))
		msg((State == INSERT) ? "Insert Mode" : "Replace Mode");
}
/*
 * tabinout(inout,num)
 *
 * If inout==0, add a tab to the begining of the next num lines.
 * If inout==1, delete a tab from the beginning of the next num lines.
 */
static void
tabinout(inout, num)
int	inout;
int	num;
{
	int	ntodo = num;
	LPTR	*p;

	beginline(FALSE);
	while (ntodo-- > 0) {
		beginline(FALSE);
		if (inout == 0)
			inschar(TAB);
		else {
			if (gchar(Curschar) == TAB)
				delchar(TRUE);
		}
		if ( ntodo > 0 ) {
			if ((p = nextline(Curschar)) != NULL)
				*Curschar = *p;
			else
				break;
		}
	}
}

/*
 * inslines(lp, dir, buf)
 *
 * Inserts lines in the file from the given buffer. Lines are inserted
 * before or after "lp" according to the given direction flag. Newlines
 * in the buffer result in multiple lines being inserted. The cursor
 * is left on the first of the inserted lines.
 */
static void
inslines(lp, dir, buf)
LINE	*lp;
int	dir;
char	*buf;
{
	register char	*cp = buf;
	register int	len;
	char	*ep;
	LINE	*l, *nc = NULL;

	if (dir == BACKWARD)
		lp = lp->prev;

	do {
		if ((ep = strchr(cp, NL)) == NULL)
			len = strlen(cp);
		else
			len = ep - cp;

		l = newline(len);
		if (len != 0)
			strncpy(l->s, cp, len);
		l->s[len] = NUL;

		l->next = lp->next;
		l->prev = lp;
		lp->next->prev = l;
		lp->next = l;

		if (nc == NULL)
			nc = l;

		lp = lp->next;

		cp = ep + 1;
	} while (ep != NULL);

	if (dir == BACKWARD)	/* fix the top line in case we were there */
		Filemem->linep = Filetop->linep->next;

	renum();

	updatescreen();
	Curschar->linep = nc;
	Curschar->index = 0;
}
