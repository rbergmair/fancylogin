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
 * Modified by Andreas Krennmair.
 */



#include "config.h"
#include "environment.h"
#include "macros.h"
#include "definitions.h"
#include "limits.h"

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "log_message.h"
#include "emergency.h"
#include "../global/flt.h"

#define PASS_FLD  0x01
#define NAME_FLD  0x02
#define KEY_BACKSPACE 127
#define KEY_RETURN 10
#define KEY_TAB 9 


/*
 * couldn't find the header
 * again.
 */

void setlinebuf(FILE *stream);
int kill(pid_t pid, int sig);


/* 
 * simple imitiations of the ncurses 
 * functions move() and clear() 
 */

#define move(y,x) printf("%c[%d;%df",27,y,x)
#define clear()   printf("%c[2J",27);fflush(stdout)


static struct termios oldtermios;
static fancylogin_theme theme;





static void set_pwfield_attrs(void);
static void set_userfield_attrs(void);
static void reset_field_attrs(void);
static void init_screen(void);
static int process_config(void);
static int draw_ansiscreen(char *ansiscreen);
static int draw_logon(void);
static int inputmask(int ypos, int xpos, const int maxchars, char *getback,
                     char mask, char emptyc);
static char readkey(void);
int initialize_prompt(void);
void fancy_prompt(char *user, char *password);
int close_prompt(void);
int draw_faillogon(void);





/***********************************************************************/
/* set_pwfield_attrs: sets the attributes for the password-mask from   */ 
/*                    the themefile.                                   */
/***********************************************************************/

static void 
set_pwfield_attrs(void)
{
  printf("%c[%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%dm",27,0,
  theme.pwattr[0],
  theme.pwattr[1],
  theme.pwattr[2],
  theme.pwattr[3],
  theme.pwattr[4],
  theme.pwattr[5],
  theme.pwattr[6],
  theme.pwattr[7],
  theme.pwattr[8],
  theme.pwattr[9]);
}



/***********************************************************************/
/* set_userfield_attrs: sets the attributes for the username-mask from */
/*                      the themefile.                                 */
/***********************************************************************/

static void 
set_userfield_attrs(void)
{
  printf("%c[%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%dm",27,0,
  theme.unattr[0],
  theme.unattr[1],
  theme.unattr[2],
  theme.unattr[3],
  theme.unattr[4],
  theme.unattr[5],
  theme.unattr[6],
  theme.unattr[7],
  theme.unattr[8],
  theme.unattr[9]);
}



/************************************************************************/
/* reset_field_colors: resets the colors to the terminal's default      */
/************************************************************************/

static void 
reset_field_attrs(void)
{
  printf("%c[m",27);
}



/*************************************************************************/
/* init_screen: initializes the screen to show special characters, e.g.  */
/*              card symbols, arrows, the famous framebars, etc.         */
/*************************************************************************/

static void 
init_screen(void)
{
  printf("\033[m\033]R\033]R\033[H\033[J\033[0;10;1;11m");
}



/*****************************************************************************/
/* process_config: Processes the configuration file "signon.defs".           */
/*****************************************************************************/

static int
process_config(void)
{
  int fd;

  fd=open(FILENAME_DEFAULTFLT, O_RDONLY);
  read(fd, &theme, sizeof theme);
  close(fd);

  return 0;
}



/*****************************************************************************/
/* close_prompt: stuff to do when terminating                                */
/*****************************************************************************/

int
close_prompt (void)
{
  /* set the new attributes */
  if (tcsetattr(0,TCSADRAIN,&oldtermios))
    return -1;

  setlinebuf(stdout); 
  fflush(stdout);
  printf("%cc", 27);
  fflush(stdout);
  return 0;
}



/*****************************************************************************/
/* draw_ansifile: draws the logon-screen, defined in an ansi file.           */
/*****************************************************************************/

static int
draw_ansiscreen (char *ansiscreen)
{
  char c;             /* color-character-buffer */
  FILE *theme;        
  struct utsname info;
  char termname[__MAX_STR_LEN__];
  
  /* 
   * we try to get the hostname and the terminal-name; 
   * if this fails, the hostname becomes an empty
   * string and the terminal-name is simply `tty'.            
   */

  sscanf (ttyname(0), "/dev/%s", termname);
  uname(&info);

  init_screen();

  clear();
  move (0, 0);

   
  while ((c=(*(ansiscreen++)))!=EOF)  /* for every character you find in */
    {                            /*  the themefile...*/  
      if (c=='%')
        {
          int i;
          int k;
          int isopen=TRUE;
          c=(*(ansiscreen++));
          switch (c)
            {
              case 'h': put_out(info.nodename)
              case 't': put_out(termname)
              case 's': put_out(info.sysname)
              case 'm': put_out(info.machine)
            }
        }
      else
        putchar(c);   
    } /* while */


  fflush(stdout);

  return 0;
}



/*****************************************************************************/
/* draw_logon: paint the login-screen                                        */
/*****************************************************************************/

static int
draw_logon(void)
{
  draw_ansiscreen(theme.ws);
  return 0;
}



/*****************************************************************************/
/* draw_faillogin: draw the failed-login-screen, and wait for the user to    */
/*                 press a key.                                              */
/*****************************************************************************/

int
draw_faillogon(void)
{
  int x;
  draw_ansiscreen(theme.fs);
  x=getchar();
  return 0;
}



/*****************************************************************************/
/* readkey: read a key from stdin, that is >= 0x20                           */
/*****************************************************************************/

static char
readkey(void)
{
  char c;

  do
    c=getchar();
  while (!((c>=0x20) ||
           (c==KEY_BACKSPACE) ||
           (c==KEY_RETURN) ||
           (c==KEY_TAB)));

  return c;
}



/*****************************************************************************/
/* inputmask: draws an input field at y, x which can take up to maxchars     */
/*            characters, returns the result in char *getback, uses char mask*/
/*            to display an input-mask for example for password-input. If you*/
/*            don't want a mask, set mask 0. char emptyc defines what        */
/*            character to use, if nothing is displayed, usually ' '.        */
/*            colp defines which color-pair to use.                          */
/*            returns 0 if the user terminates the input with <RETURN> and   */
/*            1 if the user terminates with <TAB>.                           */
/*****************************************************************************/

static int
inputmask(int ypos, int xpos, const int maxchars, char *getback,
          char mask, char emptyc)
{
  int x=0;     /* Indexcounter for the current editing position */
  int c;       /* Character-buffer */
  int i;       /* counter for several purposes */

  /* clear the part of the screen we'll need */
  move(ypos, xpos);
  for (i=0;i<maxchars;i++)
    putchar (emptyc);
  move(ypos, xpos);
  fflush (stdout);

  /* initialize the string to return */
  for (i=0;i<=maxchars;i++)
    getback[i]=0;

  for (;;)                                     /* forever do */
    {
      int i;
      x++;                                     /* increment indexcounter */
      c=readkey();                             /* get character from input */
      if ((c == KEY_RETURN) || (c == KEY_TAB))
        break;                                 /* break if <RETURN> or <TAB> */

      switch (c)
        {
          case KEY_BACKSPACE:
            /*
             * the user hit <BACKSPACE>
             */
            if (x>1)                           /* are there characters left */
              {                                /* to delete? */
                x--;                           /* yes-> decrement counter */
                move(ypos, xpos+x-1);          /* move one position back */
                getback[x-1]=0;                /* delete char from string */
                for (i=x;i<=maxchars;i++)
                  putchar(emptyc);             /* overwrite the ch on screen */
                move(ypos, xpos+x-1);          /* and go back again */
                fflush (stdout);               /* show everything */
              }
            x--;                               /* decrement counter */
            break;

          case 3:
            kill(getpid(), SIGINT); 

          default:
            if (x <= maxchars)                /* is there space left? */
              {                               /* yes-> */
                move(ypos, xpos+x-1);         /* go back one character */
                if (mask==0)                  /* do we need masks? */
                  putchar(c);                 /* no:write the char from inp.*/
                else
                  putchar(mask);              /* yes: write the mask-char */
                getback[x-1]=(char)c;         /* add the char to the str. */
                fflush(stdout);               /* show everything */
              }
            else
              x--;                            /* no: decrement counter */
            break;
        } /* switch */
    } /* for */

  if (c==10) return 0;                 /* return 0 when exited with <RETURN> */
  return 1;                            /* return 1 if not so (<TAB>) */
}



/*****************************************************************************/
/* initialize_prompt: Initializes everything                                 */
/*****************************************************************************/

int
initialize_prompt (void)
{
  struct termios tio;

  /* load the configuration */
  process_config ();

  /* get the current terminal attributes */
  if (tcgetattr(0,&tio))
    return -1;

  oldtermios=tio;

  tio.c_lflag &= ICANON;
  tio.c_lflag &= ECHO;

  /* set the new attributes */
  if (tcsetattr(0,TCSADRAIN,&tio))
    return -1;

  return 0;
}



/******************************************************************************/
/* main-function                                                              */
/******************************************************************************/

void
fancy_prompt (char *user, char *password)
{
  do 
    {
      process_config ();
      draw_logon ();
      set_userfield_attrs();
      inputmask (theme.unpy, theme.unpx, theme.unle, user, 0, ' ');
      set_pwfield_attrs();
    }
  while (inputmask (theme.pwpy, theme.pwpx, theme.pwle, password, '*', ' '));

  reset_field_attrs();
}



/*****************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed! */
/*****************************************************************************/
