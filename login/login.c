/*
 * login.c  -- fancylogin-login program.
 *
 *             fancylogin uses ncurses to display a colorful login-
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
 * ANSI-conformance testing by Andreas Krennmair
 */



#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <utmp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <malloc.h>

#include "limits.h"
#include "definitions.h"
#include "fancy.h"
#include "authenticated.h"
#include "log_message.h"
#include "system_logon.h"
#include "signal_handle.h"
#include "emergency.h"



/******************************************************************************/
/* system_reset: resets the permissions for tty's and logs the logout to wtmp */
/******************************************************************************/

void
system_reset (void)
{
  struct utmp *wtmpentry;


  /*
   * log the logout to wtmp
   */

  wtmpentry = (struct utmp *) malloc (sizeof(struct utmp));
  check(wtmpentry);

  wtmpentry->ut_type = USER_PROCESS;
  wtmpentry->ut_user[0] = '\0';
  time (&(wtmpentry->ut_tv.tv_sec));
  sscanf (ttyname(0), "/dev/%s", wtmpentry->ut_line);
  updwtmp ("/var/log/wtmp", wtmpentry);
  free (wtmpentry);


  /*
   * reset the permissions of the terminal
   */

  chown (ttyname(0), 0, 0);
  chmod (ttyname(0), 0600);
}



#ifdef TIMEOUT

/******************************************************************************/
/* timeout: action to be taken when the login times out                       */
/******************************************************************************/

void
timeout (int val)
{
  log_message (20000, "Login timed out");
  close_prompt ();
  exit (-1);
}

#endif



/******************************************************************************/
/* Main-program                                                               */
/******************************************************************************/

int
main (int argc, char **argv, char **env)
{
  char username[__LEN_USERNAME__+1]="";  
  char password[__LEN_PASSWORD__+1]="";
  char errormsg[__MAX_STR_LEN__];
  char rmthost[__MAX_STR_LEN__];
  int preserve=FALSE;
  int i;
  int cntwrong = 0;
  struct passwd *user=NULL;

  install_handlers ();

#ifdef INTRO_STRING
  printf (INTRO_STRING);
  sleep (1);
#endif

  rmthost[0]='\0';  
  if ((argc>=3) && ((strcmp(argv[1], "-r")==0) || (strcmp(argv[1], "-h") == 0)))
    strcpy (rmthost, argv[2]);

  set_rmthost (rmthost);

  for (i=1;i<argc;i++)
    if ((argv[i][0]=='-') && (argv[i][1]=='p'))
      preserve=TRUE;

  if (initialize_prompt() != 0)
    {
      log_message(1, "Could not tcsetattr() my terminal.");
      fatal ();
    }

#ifdef TIMEOUT
  if (rmthost != 0)
    {
      signal (SIGALRM, timeout);
      alarm (TIMEOUT);
    }
#endif

#ifdef CLOSE_AFTER_FAILED
  for (i=0;i<CLOSE_AFTER_FAILED;i++)
#else
  while (1)
#endif
    {
      fancy_prompt (username, password);

      if ((user = authenticated (username, password, rmthost))!=NULL)
        {
          /*
           * scramble password so that it can't be read out of the
           * core in any circumstances
           */
          int i;
          for (i=0;i<sizeof(password);i++)
            password[i]=rand()%256;
          break;
        }
      else
        draw_faillogon ();

      cntwrong++;
      sprintf (errormsg, "login failed %d times", cntwrong);
      log_message (20000+cntwrong, errormsg);
    }

#ifdef TIMEOUT
  signal (SIGALRM, SIG_IGN);
  alarm (0);
#endif

  close_prompt ();

  if (user != NULL)
    {
      system_logon (user, rmthost, preserve);
      system_reset ();
    }

  return 0;
}



/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
