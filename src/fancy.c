/* fancy.c  -- fancy-login module. Uses ncurses to display a colorful login-
               screen with input-masks.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Written by Richard Bergmair.  */




#include "definitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>


int uyc;       /* Username Y-Coordinate         */
int uxc;       /* Username X-Coordinate         */
int unc;       /* Username Number of Characters */
int umc;       /* Username Mask Color           */
int pyc;       /* Password Y-Coordinate         */
int pxc;       /* Password X-Coordinate         */
int pnc;       /* Password Number of Characters */
int pmc;       /* Password Mask Color           */
int fxc;       /* Failure X-Coordinate          */
int fyc;       /* Failure Y-Coordinate          */
int fmc;       /* Failure-Message-Color         */



/******************************************************************************/
/* scanline: puts everything from the current position til the next '\n'      */
/*           into the char *line and returns TRUE; if it gets an infile which */
/*           is already EOF it returns FALSE.                                 */
/******************************************************************************/

static int
scanline(FILE *infile, char *line)
{
  int i=0;
  int c;

  /* return FALSE if EOF */
  if ((c=fgetc(infile))==EOF)
    return FALSE;

  if (c != '\n')               /* put each character from infile... */
    do                          
      {
        line[i]=c;             /* ...into line... */ 
        i++;
      }
    while (((c=fgetc(infile))!='\n') && (c!=EOF));
                               /* until '\n' or EOF is found  */

  line[i]=0;                   /* and terminate the string. */
  return TRUE;
}



/******************************************************************************/
/* setcol: converts the color it gets from the char* colc to the short value  */
/*         defined by the ncurses-defines and puts it to coln.                */
/******************************************************************************/

static void
setcol (short *coln, char *colc)
{
  /*
   * if anybody knows a more elegant   
   * way of doing this please tell me! 
   */

  if (strcmp(colc,"BLACK")==0)
    (*coln)=COLOR_BLACK;
  if (strcmp(colc,"RED")==0)
    (*coln)=COLOR_RED;
  if (strcmp(colc,"GREEN")==0)
    (*coln)=COLOR_GREEN;
  if (strcmp(colc,"YELLOW")==0)
    (*coln)=COLOR_YELLOW;
  if (strcmp(colc,"BLUE")==0)
    (*coln)=COLOR_BLUE;
  if (strcmp(colc,"MAGENTA")==0)
    (*coln)=COLOR_MAGENTA;
  if (strcmp(colc,"CYAN")==0)
    (*coln)=COLOR_CYAN;
  if (strcmp(colc,"WHITE")==0)
    (*coln)=COLOR_WHITE;
}



/******************************************************************************/
/* setc: init_pair-equivalent which takes char* instead of short              */
/******************************************************************************/

static int
setc(int num, char *forec, char *backc)
{
  short foregc;
  short backgc;

  /* convert */
  setcol(&foregc, forec);
  setcol(&backgc, backc);

  /* init */
  init_pair(num, foregc, backgc);
}



/******************************************************************************/
/* process_config: Processes the configuration file "logon.defs" as well      */
/*                 as inits the color pairs using the definition numbers.     */
/******************************************************************************/

static int
process_config(void)
{
  FILE *configuration;    /* configuration-file handler            */
  char line[100];         /* buffer to take the input line by line */
  int i;                  /* counter                               */
  int no;                 /* Number                                */
  char foreg[10];         /* Foreground                            */
  char backg[10];         /* Background                            */ 

  /*
   * open the configuration.
   */

  if ((configuration = fopen("/etc/signon.defs","r"))==NULL)
    {
      fprintf (stderr, "Could not open /etc/signon.defs\n");
      return -1;
    }


  /* go through the file line by line... */
  while (scanline(configuration, line)) 

    /* ...leaving out comments */    
    if ((line[0]!='#') && (line[0]!=0) && (line[0] !=10))

      /* as we can only define "color" "username" and "password", */
      /* it is enough to compare the first character.             */
      switch (line[0])
        {
           /* color ****************************************************/
           case 'c' : sscanf (line+5, " %d %s %s", &no, foreg, backg);
                      setc (no, foreg, backg);
                      break; 

           /* username *************************************************/
           case 'u' : sscanf (line+8, " %d %d %d %d", &uyc, &uxc, &unc, &umc);
                      break;

           /* password *************************************************/  
           case 'p' : sscanf (line+8, " %d %d %d %d", &pyc, &pxc, &pnc, &pmc);
                      break;

           /* failure **************************************************/  
           case 'f' : sscanf (line+7, " %d %d %d", &fyc, &fxc, &fmc);
                      break;
        }


  /*
   * close file
   */

  fclose(configuration);
}



/******************************************************************************/
/* getpos: returns the number found at position y, x in file infile           */
/******************************************************************************/

static int
getpos(FILE *infile, int y, int x)
{
  int c;      /* buffer for the character */
  int px;     /* index for the row */
  int py=0;   /* index for the column */

  while ((c=fgetc(infile))!=EOF)
    {
      if (c=='\n')  /* if a \n is found increase the y-position-index */
        py++;
      if (py==y)    /* if we are in the correct line ... */
        {
          for (px=0;((px<=x)&&(c!=EOF));px++) /* ... look for the correct col */
            c=fgetc(infile);
          return (int)(c-'0'); /* ... and return the char as int */
        }
    }
}



/******************************************************************************/
/* finish: on program-exit also do a curses-endwin()                          */
/******************************************************************************/

static void
finish (int sig)
{
  endwin();
}

void
close_prompt (void)
{
  finish (0);
}



/******************************************************************************/
/* initncurses: do initialization for ncurses                                 */
/******************************************************************************/

void
initncurses (void)
{
  signal(SIGINT, finish);  /* arrange interrupts to terminate */

  initscr ();              /* initialize the curses library */
  keypad (stdscr, TRUE);   /* enable keyboard mapping */
  nonl ();                 /* tell curses not to do NL->CR/NL on output */
  cbreak ();               /* take input chars one at a time, no wait for \n */
  noecho ();               /* don't echo input */
  start_color ();          /* initialize the use of colors */

  init_pair(50, COLOR_WHITE, COLOR_RED);
}



/******************************************************************************/
/* draw_logon: draws the logon-screen, defined in signon.colors and           */
/*             signon.text onto the screen.                                   */
/*             NOTE: to keep the code small and simple, the program           */
/*             supposes, that signon.colors and signon.text both have 25*80   */
/*             (or whatever size your screen has) characters, except for \n   */
/*             and other control-characters.                                  */
/******************************************************************************/

static int
draw_logon (void)
{
  char c;           /* color-character-buffer */
  char t;           /* text-character-buffer */
  FILE *colorfile;  /* file-handler for signon.colors */
  FILE *textfile;   /* file-handler for signon.text */


  /*
   * open the files 
   */

  if ((colorfile = fopen("/etc/signon.colors","r"))==NULL)
    {
      fprintf(stderr,"Could not open /etc/signon.colors\n");
      return -1;
    }
  if ((textfile = fopen("/etc/signon.text","r")) == NULL)
    {
      fprintf(stderr,"Could not open /etc/signon.text\n");
      return -1;
    }

  move (0, 0);
 
  while ((c=fgetc(colorfile))!=EOF)   /* for every character you find in */
    {                                 /* signon.colors ...*/
      t=fgetc(textfile);              /* ...take the next character from text */
      attrset(COLOR_PAIR(c-'0'));     /* ...and set the color-pair */
      if ((t != '\n') && (t > 30))    /* leaving out special-characters */
        addch(t);                     /* print the character from text */
    }


  /*
   * close the files
   */

  fclose (colorfile);
  fclose (textfile);

  refresh ();

  return 0;
}



/******************************************************************************/
/* loginfail: puts a message, that the login failed                           */
/******************************************************************************/
int 
fail_login(void)
{
  FILE *failmsg;
  int c;
  int xp;
  int yp;


  /*
   * open the configuration file
   */

  if ((failmsg = fopen("/etc/signon.fail","r")) == NULL)
    {
      fprintf (stderr, "Could not open /etc/signon.fail\n");
      return -1;
    }


  /*
   * set attributes for error-message
   */

  attrset(COLOR_PAIR(fmc));

  xp=fxc;
  yp=fyc;

  while ((c=fgetc(failmsg))!=EOF)   /* for every character you find */
    {
       if (c!='\n')
         {
           move (yp, xp);           /* put the character, if it's not a CRLF */
           addch (c);
           xp++;
         }
       else
         {
           yp++;                    /* modify indices if CRLF */
           xp=fxc;
         }
    }


  /*
   * Close the file
   */

  fclose(failmsg);  


  /*
   * Display the new screen
   */

  refresh ();


  /*
   * Wait for the user to press a key
   */

  getch ();


  /*
   * no errors happened, putting the
   * messages.
   */

  return 0;
}

/******************************************************************************/
/* inputmask: draws an input field at y, x which can take up to maxchars      */
/*            characters, returns the result in char *getback, uses char mask */
/*            to display an input-mask for example for password-input. If you */
/*            don't want a mask, set mask 0. char emptyc defines what         */
/*            character to use, if nothing is displayed, usually ' '.         */
/*            colp defines which color-pair to use.                           */
/*            returns 0 if the user terminates the input with <RETURN> and    */
/*            1 if the user terminates with <TAB>.                            */
/******************************************************************************/

static int
inputmask(int ypos, int xpos, const int maxchars, char *getback, char mask, char emptyc, int colp)
{
  int x=0;     /* Indexcounter for the current editing position */
  int c;       /* Character-buffer */
  int i;       /* counter for several purposes */

  attrset(COLOR_PAIR(colp));

  /* clear the part of the screen we'll need */
  move(ypos, xpos);
  for (i=0;i<maxchars;i++)
    addch(emptyc);
  move(ypos, xpos);
  refresh ();


  /* initialize the string to return */
  for (i=0;i<=maxchars;i++)
    getback[i]=0;

  for (;;)                                     /* forever do */
    {
      x++;                                     /* increment indexcounter */
      c = getch();                             /* get character from input */

      if ((c == 13) || (c == 9)) break;        /* break if <RETURN> or <TAB> */

      if (i == KEY_BACKSPACE)                  /* is <BACKSPACE> pressed? */
        {                                      /* yes-> */
          if (x>1)                             /* are there characters left */
            {                                  /* to delete? */
              x--;                             /* yes-> decrement counter */
              move(ypos, xpos+x-1);            /* move one position back */
              getback[x-1]=0;                  /* delete char from string */
              addch(emptyc);                   /* overwrite the ch on screen */
              move(ypos, xpos+x-1);            /* and go back again */
              refresh ();                      /* show everything */
            }
          x--;                                 /* decrement counter */
        }
      else                                     /* no-> */
        if (x <= maxchars)                     /* is there space left? */
          {                                    /* yes-> */
            move(ypos, xpos+x-1);              /* go one character back */
            if (mask==0)                       /* do we need masks? */
              addch(c);                        /* no:write the char from inp. */
            else                               
              addch(mask);                     /* yes: write the mask-char */
            getback[x-1]=(char)c;              /* add the char to the str. */
            refresh();                         /* show everything */
          }
        else                                    /* no-> */
          x--;                                  /* decrement counter */
    }

  if (c==13) return 0;                 /* return 0 when exited with <RETURN> */
  return 1;                            /* return 1 if not so (<TAB>) */
}



/******************************************************************************/
/* initialize_prompt: Initializes everything                                  */
/******************************************************************************/
int
initialize_prompt (void)
{
  initncurses ();
  if (process_config () == -1)
    {
      finish (-1);
      return -1;
    }
  return 0;
}



/******************************************************************************/
/* main-program                                                               */
/******************************************************************************/

int
fancy_prompt (char *user, char *password)
{
  if (draw_logon () == -1)
    {
      finish (-1);
      return -1;
    }

  do
    inputmask (uyc, uxc, unc, user, 0,  ' ', umc);
  while (inputmask (pyc, pxc, pnc, password, '*',  ' ', pmc));

  return 0;
}

