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
#include <grp.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#include "macros.h"
#include "limits.h"
#include "log_message.h"
#include "emergency.h"
#include "after_login.h"


/*
 * Couldn't find the declaration
 */
 
int initgroups (const char *user, gid_t group);
int putenv(const char *string);





static char **getnewenv (struct passwd *user);
static int write_lastlog_entry(uid_t user_uid, char *rmthost,
                               struct lastlog *oldlog);
int system_logon (struct passwd *user, char *rmthost, int preserve);




  
/******************************************************************************/
/* getnewenv: returns the environment, the shell needs from us, to give       */
/*            username, our PID, home-directory, ...                          */
/******************************************************************************/

static char **
getnewenv (struct passwd *user)
{
  #define __NUM_VARIABLES__ 7 


  int i;
  char **ne;


  /*
   * allocate memory for the environment
   */

  ne = (char **)malloc( sizeof(char *) * (__NUM_VARIABLES__));
  check (ne);

  for (i=0;i<__NUM_VARIABLES__;i++)
    {
      ne[i] = (char *)malloc(sizeof(char) * (__MAX_STR_LEN__));
      check (ne[i]);
    }

  i=0;


  /*
   * put some values in there.
   *
   * if you add something here, don't forget to increment
   * __NUM_VARIABLES__!
   */

  sprintf(ne[i++], "HOME=%s", user->pw_dir);
  sprintf(ne[i++], "SHELL=%s", user->pw_shell);
  sprintf(ne[i++], "USER=%s", user->pw_name);
  sprintf(ne[i++], "LOGINAME=%s", user->pw_name);
  sprintf(ne[i++], "TERM=%s", getenv("TERM"));


#ifdef SET_PATH_ENV
  if ((user->pw_uid) == 0)
    sprintf(ne[i++], "PATH=/sbin:/bin:/usr/sbin:/usr/bin");
  else
    sprintf(ne[i++], "PATH=/usr/local/bin:/bin:/usr/bin:.");
#endif

  ne[i] = NULL;

  return ne;
}



/******************************************************************************/
/* write_lastlog_entry: writes an entry of the current login into the lastlog */
/******************************************************************************/

static int
write_lastlog_entry(uid_t user_uid, char *rmthost, struct lastlog *oldlog)
{
  struct lastlog lastlog;       /* structure for parsing lastlog */
  int lastlog_file;             /* file-handler for /var/log/lastlog */

  /* open lastlog for writing */
  if ((lastlog_file = open (FILENAME_LASTLOG, O_RDWR)) == -1)
    {
      log_message (10001, "couldn't open FILENAME_LASTLOG for O_RDWR!");
      return -1;
    }

  /* what is our line? */
  sscanf (ttyname(0), "/dev/%s", lastlog.ll_line);

  /* what host do we log on from? */
  strcpy (lastlog.ll_host, rmthost);

  /* what is the time? */
  time (&lastlog.ll_time);

  /* what user logged on? */
  lseek (lastlog_file, (unsigned long)user_uid, SEEK_SET);

  /* read old data */
  read (lastlog_file, oldlog, sizeof *oldlog);

  /* "reseek" to where we just read from */
  lseek (lastlog_file, (unsigned long)user_uid, SEEK_SET);

  /* write data */
  write (lastlog_file, (char *)&lastlog, sizeof lastlog);

  /* close file */
  close (lastlog_file);

  /* all is groovey */
  return 0;
}



/******************************************************************************/
/* system_logon: log user on to system                                        */
/******************************************************************************/

int
system_logon (struct passwd *user, char *rmthost, int preserve)
{
  char **new_environment;
  struct utmp *utmpentry;
  struct group *grp;
  struct lastlog oldlog;
  int i=0;
  int rc;

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
  free (utmpentry);


  /*
   * Get user's environment
   */
  
  if ((new_environment = getnewenv (user)) == NULL)
    return -1;
   

  /*
   * cd to user's home
   */

  chdir (user->pw_dir);


  /*
   * Change permissions to terminal
   * thanks, Thomas Wernitz.
   */

  grp = getgrnam("tty");
  chown (ttyname(0), user->pw_uid, grp->gr_gid);
  chmod (ttyname(0), 0620);

  write_lastlog_entry(user->pw_uid, rmthost, &oldlog);


  /*
   * Change to user's uid and gid
   */

  setgid (user->pw_gid);
  initgroups (user->pw_name, user->pw_gid);
  setuid (user->pw_uid);
  
  after_login (user->pw_uid, rmthost, &oldlog);


  /*
   * execute user's shell
   */
  rc=fork();
  if (rc==0)
    {
      char login_shell[41] = "-";
    
      strcat(login_shell,user->pw_shell);

      if (!(preserve))
        execle(user->pw_shell, login_shell , NULL,
               new_environment);
      else
        {
          while (new_environment[i]!=NULL)
            putenv(new_environment[i++]);
          execl (user->pw_shell, login_shell , NULL);
        }
    }
  else
    if (rc == -1)
      {
        char errormessage[__MAX_STR_LEN__];

        sprintf(errormessage,"couldn't fork off process for user-environment!");
        log_message (1, errormessage);
        return -1;
      }
    else
      wait (&i);
 
  return 0;
}



/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
