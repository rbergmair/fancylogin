/*
 * system_logon.c  -- fancylogin system-logon procedure.
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
#include "environment.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "macros.h"
#include "limits.h"
#include "log_message.h"
#include "emergency.h"




  
/******************************************************************************/
/* getnewenv: returns the environment, the shell needs from us, to give       */
/*            username, our PID, home-directory, ...                          */
/******************************************************************************/

char **
getnewenv (struct passwd *user)
{
  #define __NUM_VARIABLES__ 6 


  int i;


  /*
   * allocate memory for the environment
   */

  char **ne;

  ne = (char **)malloc( sizeof(char *) * (__NUM_VARIABLES__+1));
  check (ne);

  for (i=0;i<7-1;i++)
    {
      ne[i] = (char *)malloc(sizeof(char) * (__MAX_STR_LEN__+1));
      check (ne[i]);
    }


  /*
   * put some values in there.
   *
   * if you add something here, don't forget to increment
   * __NUM_VARIABLES__!
   */

  i=0;
  sprintf(ne[i++], "HOME=%s", user->pw_dir);
  sprintf(ne[i++], "SHELL=%s", user->pw_shell);
  sprintf(ne[i++], "USER=%s", user->pw_name);
  sprintf(ne[i++], "LOGINAME=%s", user->pw_name);
  sprintf(ne[i++], "TERM=%s", getenv("TERM"));

  ne[i] = NULL;

  return ne;
}



/******************************************************************************/
/* system_logon: log user on to system                                        */
/******************************************************************************/

int
system_logon (struct passwd *user, char *rmthost)
{
  char **new_environment;
  struct utmp *utmpentry;
  struct lastlog lastlog;
  int lastlog_file;


  printf("\fWelcome, %s!\nYou are being logged on...", user->pw_gecos);


  /*
   * Update utmp and wtmp entries
   */

  utmpentry = (struct utmp *) malloc (sizeof(struct utmp));
  check (utmpentry);
  
  utmpentry->ut_type = USER_PROCESS;                        /* type of login */ 
  utmpentry->ut_pid = getpid ();                                  /* our pid */
  strcpy (utmpentry->ut_user, user->pw_name);                    /* username */
  strcpy (utmpentry->ut_host, rmthost);                   /* remote hostname */
  time (&(utmpentry->ut_tv.tv_sec));                         /* current time */
  sscanf (ttyname(0), "/dev/%s", utmpentry->ut_line);                /* line */

  updwtmp ("/var/log/wtmp", utmpentry);  
  pututline (utmpentry);  


  /*
   * Get user's environment
   */

  if ((new_environment = getnewenv (user)) == NULL)
    return -1;


  /*
   * create an entry to /var/log/lastlog
   */

  if ((lastlog_file = open (FILENAME_LASTLOG, O_WRONLY)) == -1)
    {
      fprintf (stderr, "could not open %s", FILENAME_LASTLOG);
      return -1;
    }


  /*
   * seek to the uid
   */

  lseek (lastlog_file, (unsigned long)user->pw_uid, SEEK_SET);


  /*
   * write the new record
   */

  sscanf (ttyname(0), "/dev/%s", lastlog.ll_line);
  strcpy (lastlog.ll_host, rmthost);
  write (lastlog_file, (char *)&lastlog, sizeof lastlog);
  close (lastlog_file);

  /*
   * cd to user's home
   */

  chdir (user->pw_dir);


  /*
   * Change to user's uid and gid
   */

  setuid (user->pw_uid);
  setgid (user->pw_gid);

  printf("Done.\n");


  /*
   * exec user's shell
   */

  execle (user->pw_shell, user->pw_shell, SHELL_PARAMS, NULL, new_environment);

  return 0;
}

/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
