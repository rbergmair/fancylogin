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
#include <unistd.h>
#include <signal.h>

#include "limits.h"
#include "definitions.h"
#include "fancy.h"
#include "authenticated.h"
#include "log_message.h"
#include "system_logon.h"
#include "signal_handle.h"
  




/******************************************************************************/
/* Main-program                                                               */
/******************************************************************************/

int
main (int argc, char **argv)
{
  char username[__LEN_USERNAME__+1];  
  char password[__LEN_PASSWORD__+1];
  char errormsg[__MAX_STR_LEN__];
  char rmthost[__MAX_STR_LEN__];

#if CLOSE_AFTER_FAILED > 0
  int i;
#endif

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

  initialize_prompt ();

#if CLOSE_AFTER_FAILED > 0
  for (i=0;i<CLOSE_AFTER_FAILED;i++)
#else
  while (1)
#endif
    {
      fancy_prompt (username, password);

      if ((user = authenticated (username, password, rmthost))!=NULL)
        break;
      else
        fail_login ();

      cntwrong++;
      sprintf (errormsg, "login failed %d times", cntwrong);
      log_message (20000+cntwrong, errormsg);
    }
  
  close_prompt ();

  system_logon (user, rmthost);
  
  return 0;
}

/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
