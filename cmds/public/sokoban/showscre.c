#include <stdio.h>
#include <curses.h>
#include "sokoban.h"
#if __TURBOC__
#include <graphics.h>
#endif

extern short curses;
extern short rows, cols, level, moves, pushes, packets, savepack;
extern char  map[MAXROW+1][MAXCOL+1];

showscreen() {

   register short i, j;

#if __TURBOC__
   if (!curses)
      cleardevice();
   else
#endif
   {  move( 0, 0); clrtobot();
   }
   for( i = 0; i < rows; i++)
      for( j = 0; map[i][j] != '\0'; j++)
         mapchar( map[i][j], i, j);
#if __TURBOC__
   if (!curses)
   {  moveto( 0, 16* MAXROW);
      outtext( "Level:      Packets:      Saved:      Moves:       Pushes:");
   }
   else
#endif
   {  move( MAXROW, 0);
      printw( "Level:      Packets:      Saved:      Moves:       Pushes:");
   }
   displevel();
   disppackets();
   dispsave();
   dispmoves();
   disppushes();
   if (curses)
   {  move( MAXROW+2,0);
      refresh();
   }
}

mapchar( c, i, j) 
register char c; 
register short i, j;
{
   OBJECT *obj, *get_obj_adr();
   register short offset_row = (MAXROW - rows) / 2;
   register short offset_col = MAXCOL - cols;

   obj = get_obj_adr( c);

#if __TURBOC__
   if (!curses)
      putimage(8 * (2*j + offset_col), 16 * (i + offset_row), obj->obj_map, COPY_PUT);
   else
#endif
   {  if( obj->invers) standout();
      move( i + offset_row, 2*j + offset_col);
      printw( "%c%c", obj -> obj_display, obj -> obj_display);
      if( obj->invers) standend();
   }
}

OBJECT *get_obj_adr( c)
register char c;
{
   register OBJECT *ret;

   if(      c == player.obj_intern)		ret = &player;
   else if( c == playerstore.obj_intern)	ret = &playerstore;
   else if( c == store.obj_intern)		ret = &store;
   else if( c == save.obj_intern)		ret = &save;
   else if( c == packet.obj_intern)		ret = &packet;
   else if( c == wall.obj_intern)		ret = &wall;
   else if( c == ground.obj_intern)		ret = &ground;
   else                                         ret = &ground;

   return( ret);
}


displevel() { 
#if __TURBOC__
   char buf[4];
   if (!curses)
   {  sprintf(buf, "%3d", level);
      outtextxy( 8* 7, 16* MAXROW, buf); 
   } else
#endif
   {  move( MAXROW, 7); printw( "%3d", level); 
   }
}
   
disppackets() { 
#if __TURBOC__
   char buf[4];
   if (!curses)
   {  sprintf(buf, "%3d", packets);
      outtextxy( 8* 21, 16* MAXROW, buf);
   } else
#endif
   {  move( MAXROW, 21); printw( "%3d", packets); 
   }
}
   
dispsave() { 
#if __TURBOC__
   char buf[4];
   if (!curses)
   {  sprintf(buf, "%3d", savepack);
      putimage( 8* 33, 16* MAXROW, &blank, COPY_PUT); /* outtext is OR_PUT ? */
      outtextxy( 8* 33, 16* MAXROW, buf);
   } else
#endif
   {  move( MAXROW, 33); printw( "%3d", savepack);
   }
}
   
dispmoves() { 
#if __TURBOC__
   char buf[6];
   if (!curses)
   {  sprintf(buf, "%5d", moves);
      putimage( 8* 45, 16* MAXROW, &blank, COPY_PUT);
      putimage( 8* 47, 16* MAXROW, &blank, COPY_PUT);
      outtextxy( 8* 45, 16* MAXROW, buf);
   } else
#endif
   {  move( MAXROW, 45); printw( "%5d", moves);
   }
}
      
disppushes() { 
#if __TURBOC__
   char buf[4];
   if (!curses)
   {  sprintf(buf, "%3d", pushes);
      putimage(8* 59, 16* MAXROW, &blank, COPY_PUT);
      outtextxy( 8* 59, 16* MAXROW, buf);
   } else
#endif
   {  move( MAXROW, 59); printw( "%5d", pushes);
   }
}

helpmessage() {

   move( MAXROW+2, 0); 
   printw( "Press ? for help.%c", '\007');
   if (curses)
      refresh();
   raw();			/* no input allowed while sleeping */
   sleep( 2);
   move( MAXROW+2, 0); deleteln();
   if (curses)
      refresh();
   noraw();				/* end raw mode */
}

static char *helppages[] = { /* be sure that there are max 9 lines per page */
   "The problem is to push packets to",
   "saving positions by moving around",
   "and  pushing only one packet at a",
   "        time if possible.        ",
   "                                 ",
   "                                 ",
   "                                 ",
   "                                 ",
   "                                 ",
   NULL,					/* end of page */
   "Moving: You can move by using    ",
   "           the vi-keys hjkl.     ",
   "                                 ",
   "              left right up down ",
   "  Move/Push     h    l    k   j  ",
   "  Run/Push      H    L    K   J  ",
   "  Run only     ^H   ^L   ^K  ^J  ",
   "                                 ",
   "                                 ",
   NULL,					/* end of page */
   "Other commands:                  ",
   "   c:  temporary save            ",
   "   q:  quit                      ",
   "  ^R:  refresh the screen        ",
   "   s:  save the game             ",
   "   u:  undo last move/push       ",
   "   U:  undo all                  ",
   "  ^U:  reset to temp save        ",
   "   ?:  this help screen          ",
   NULL,					/* end of page */
   "Characters on screen are:        ",
   "                                 ",
   "  %@  player                     ",
   "  %+  player on saving position  ",
   "  %.  saving position for packet ",
   "  %$  packet                     ",
   "  %*  saved packet               ",
   "  %#  wall                       ",
   "                                 ",
   NULL,				/* end of page */
   "If you set a temporary  save, you",
   "need not  undo  all when you  get",
   "stuck. Just reset to this save.  ",
   "                                 ",
   "A temporary save is automatically",
   "made at the start.",
   "                                 ",
   "                                 ",
   "                                 ",
   NULL,					/* end of page */
   NULL						/* total end */
};

static char *title[] = {
   "          S O K O B A N          ",
   "---------------------------------"
};

static char *helphelp[] = {
   "   (Press return to exit help,   ",
   "    any other key to continue)   "
};

#define HELPROWS	16
#define HELPCOLS	37

showhelp() {

   register short line, i;
   short goon = 1;
   WINDOW *win, *makehelpwin();

#if __TURBOC__
   int cmode;
   if (!curses)
   {  cmode = getgraphmode();
      restorecrtmode();
   }
#endif
   win = makehelpwin();
   for( i = 0, line = 2; goon; i++, line++) {
      if( helppages[i] != NULL) {
	 wmove( win, line+1, 2);
	 printhelpline( win, helppages[i]);
      }
      else {
	 wmove( win, HELPROWS-1, 0);
	 wrefresh( win);
	 if( (goon = (wgetch( win) != '\n'))) {
	    line = 1;
	    if( helppages[i+1] == NULL) i = -1;
	 }
      }
   }
   werase( win);
   wrefresh( win);
   delwin( win);
#if __TURBOC__
   if (!curses)
      setgraphmode(cmode);
#endif
}

WINDOW *makehelpwin() {

   WINDOW *win, *newwin();

   win = newwin( HELPROWS, HELPCOLS, 2, 0);
   box( win, '|', '-');
   wmove( win, 1, 2);
   wprintw( win, "%s", title[0]);
   wmove( win, 2, 2);
   wprintw( win, "%s", title[1]);
   wmove( win, HELPROWS-3, 2);
   wprintw( win, "%s", helphelp[0]);
   wmove( win, HELPROWS-2, 2);
   wprintw( win, "%s", helphelp[1]);

   return( win);
}

printhelpline( win, line)
WINDOW *win;
char *line;
{
   OBJECT *obj, *get_obj_adr();

   for( ; *line != '\0'; line++) {
      if( *line == '%') {
	 ++line;
	 obj = get_obj_adr( *line);
         if( obj -> invers) wstandout( win);
         waddch( win, obj -> obj_display); waddch( win, obj -> obj_display);
         if( obj -> invers) wstandend( win);
      }
      else waddch( win, *line);
   }
}
