/*
 * fancy.c  -- fancy-login module. Uses ncurses to display a colorful login-
 *             screen with input-masks.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Written by Richard Bergmair.
 */



#include "config.h"
#include "environment.h"
#include "macros.h"
#include "definitions.h"
#include "limits.h"

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/utsname.h>

#include "log_message.h"
#include "emergency.h"



static int uyc;       /* Username Y-Coordinate         */
static int uxc;       /* Username X-Coordinate         */
static int unc;       /* Username Number of Characters */
static int umc;       /* Username Mask Color           */
static int pyc;       /* Password Y-Coordinate         */
static int pxc;       /* Password X-Coordinate         */
static int pnc;       /* Password Number of Characters */
static int pmc;       /* Password Mask Color           */
static int fxc;       /* Failure X-Coordinate          */
static int fyc;       /* Failure Y-Coordinate          */
static int fmc;       /* Failure-Message-Color         */



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
  return 0;                     
}



/******************************************************************************/
/* process_config: Processes the configuration file "logon.defs" as well      */
/*                 as inits the color pairs using the definition numbers.     */
/******************************************************************************/

static int
process_config(void)
{
  FILE *configuration;            /* configuration-file handler            */
  char line[__MAX_STR_LEN__];     /* buffer to take the input line by line */
  int no;                         /* Number                                */
  char foreg[__MAX_STR_LEN__];    /* Foreground                            */
  char backg[__MAX_STR_LEN__];    /* Background                            */ 

  /*
   * open the configuration.
   */

  if ((configuration = fopen(FILENAME_SIGNON_DEFS,"r"))==NULL)
    {
      char message[__MAX_STR_LEN__];  
      sprintf (message, "Could not open %s", FILENAME_SIGNON_DEFS);
      log_message (1, message);
      fatal ();
    }


  /* go through the file line by line... */
  while (fgets (line, sizeof (line), configuration) != NULL)

    /* ...leaving out comments */    
    if ((line[0]!='#') && (line[0]!=0) && (line[0] !=10))

      /* as we can only define "color", "username", "password" and "failure", */
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
  return 0;
}



/******************************************************************************/
/* close_prompt: stuff to do when terminating                                 */
/******************************************************************************/

void
close_prompt (void)
{
  endwin();
}



/******************************************************************************/
/* initncurses: do initialization for ncurses                                 */
/******************************************************************************/

void
initncurses (void)
{
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
  char c;             /* color-character-buffer */
  char t;             /* text-character-buffer */
  FILE *colorfile;    /* file-handler for signon.colors */
  FILE *textfile;     /* file-handler for signon.text */
  int isopen=0;
  char termname[__MAX_STR_LEN__];
  struct utsname info;
  
  /* 
   * we try to get the hostname and the terminal-name; 
   * if this fails, the hostname becomes an empty
   * strings and the terminal-name is simply `tty'.            
   */

  sscanf (ttyname(0), "/dev/%s", termname);
  uname(&info);


  /*
   * open the files 
   */

  if ((colorfile = fopen(FILENAME_SIGNON_COLORS,"rt"))==NULL)
    {
      char message[__MAX_STR_LEN__];
      sprintf (message, "Could not open %s\n", FILENAME_SIGNON_COLORS);
      log_message (1, message);
      fatal ();
    }


  if ((textfile = fopen(FILENAME_SIGNON_TEXT, "rt")) == NULL)
    {
      char message[__MAX_STR_LEN__];
      sprintf(message,"Could not open %s\n", FILENAME_SIGNON_TEXT);
      log_message (1, message);
      fclose (colorfile);
      fatal ();
    }


  move (0, 0);

   
  while ((c=fgetc(colorfile))!=EOF)  /* for every character you find in */
    {                                /* signon.colors ...*/  
      if ((t=fgetc(textfile))==EOF)  /* take the next character from text */
        break;             
      attrset(COLOR_PAIR(c-'0'));    /* set the color-pair */

      if (t=='%')
        {
          int i; 
          int k;
          isopen=TRUE;

          /*
           * the code-sequence of put_out is defined
           * in the macros file, and looks like this:
           *
           * #define put_out(value)              \
           *   for (i=0,k=0;t!=EOF && k<3;)      \
           *     {                               \
           *       attrset(COLOR_PAIR(c-'0'));   \
           *       if (i<strlen(value))          \
           *         addch(value[i++]);          \
           *       else                          \
           *         addch(' ');                 \
           *       if (t=='%')                   \
           *         k++;                        \
           *       else                          \
           *         {                           \
           *           t=fgetc(textfile);        \
           *           c=fgetc(colorfile);       \
           *         }                           \
           *     }                               \
           *   isopen=FALSE;                     \
           *   c=fgetc(colorfile);               \
           *   t=fgetc(textfile);                \
           *   break;
           */
                                                      
          /* take the next character */ 
          if ((t=fgetc(textfile))==EOF) 
            break;
          c=fgetc(colorfile);
          attrset(COLOR_PAIR(c-'0'));    /* set the color-pair */

          /* what are we supposed to put? */
          switch (t)
            {
              case 'h': put_out(info.nodename)
              case 't': put_out(termname)
              case 's': put_out(info.sysname)
              case 'm': put_out(info.machine)
	    }
	}
      else
        if (t >= 0x20)                 /* leaving out special-characters */
          addch(t);                    /* print the character from text */
    
   } /* while */


  /*
   * close the files
   */

  fclose (colorfile);
  fclose (textfile);
  
  refresh ();

  return 0;
}



/******************************************************************************/
/* loginfail: puts a message, telling the user, that the login failed.        */
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

  if ((failmsg = fopen(FILENAME_SIGNON_FAIL,"r")) == NULL)
    {
      char message[__MAX_STR_LEN__];
      sprintf (message, "Could not open %s\n", FILENAME_SIGNON_FAIL);
      log_message (1, message);
      fatal ();
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

      if (c == KEY_BACKSPACE)                  /* is <BACKSPACE> pressed? */
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

void
initialize_prompt (void)
{
  initncurses ();
  process_config ();
}



/******************************************************************************/
/* main-program                                                               */
/******************************************************************************/

void
fancy_prompt (char *user, char *password)
{
  draw_logon ();

  do
    inputmask (uyc, uxc, unc, user, 0,  ' ', umc);
  while (inputmask (pyc, pxc, pnc, password, '*',  ' ', pmc));
}

/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
