/*
 * after_login.c  -- what to do after a login, before executing the shell
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
#include "limits.h"
#include "definitions.h"
#include "environment.h"
#include "log_message.h"
#include "emergency.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utmp.h>
#include <time.h>
#include <malloc.h>





static void print_welcome(struct passwd *userinfo);
static int print_lastlog(struct lastlog *oldlog);
static void print_motd(void);
static void check_mail(struct passwd *userinfo);
void after_login(uid_t uid, char *rmthost, struct lastlog *oldlog);





/******************************************************************************/
/* print_welcome: prints the "Welcome, username" message after the login.     */
/******************************************************************************/

static void
print_welcome (struct passwd *userinfo)
{
  char *name;
  if ((userinfo->pw_gecos == NULL) || (userinfo->pw_gecos[0] == 0))
    name=userinfo->pw_name;
  else
    name=userinfo->pw_gecos;

  printf ("Welcome, %s\n\n", name);
}  



/******************************************************************************/
/* print_lastlog: prints location and time of the last login.                 */
/******************************************************************************/

static int
print_lastlog (struct lastlog *oldlog)
{
  char buf [__MAX_STR_LEN__];   /* define a buffer for strtime */
  char *strtime = buf;          /* pointer to the buffer */

  /* convert ll_time to a human-readable string */
  strtime = ctime (&(oldlog->ll_time));

  /* print some information */
  printf ("Last login was from %s, at %s\n",
          oldlog->ll_line, strtime);

  return 0;
}



/******************************************************************************/
/* print_motd: prints the message of the day                                  */
/******************************************************************************/

static void
print_motd (void)
{
  FILE *motd;
  char c;

  if ((motd = fopen ("/etc/motd", "r")) != NULL)  /* open the file */
    {
      while ((c = fgetc(motd)) != EOF)            /* print the file */
        fputc (c, stdout);
      fclose (motd);                              /* close the file */
    }
}



#ifdef CHECK_MAIL

/*****************************************************************************/
/* check_mail: checks the current state of the user's mailbox                */
/*****************************************************************************/

static void
check_mail(struct passwd *userinfo)
{
  struct stat statbuf;
  char *mailbox;  

  if ((mailbox = getenv("MAILDIR"))!=NULL) 
    {
      char * newmail;
      newmail = malloc(strlen(mailbox) + 5);

      /*
       * this does NOT do what it looks like. It only checks whether
       * newmail is a NULL pointer and if so, it stops logging in
       */
      check(newmail);

      /* What's the mailbox filename? */
      sprintf(newmail, "%s/new", mailbox);

      /* Now let's get some information on the file */
      if ((stat(newmail, &statbuf) != -1) && (statbuf.st_size != 0))

        /*
         * is st_mtime (time of last modification) greater than
         * st_atime (time of last access)?
         */
        if (statbuf.st_mtime > statbuf.st_atime)
          {
            free(newmail);
            printf(MSG_NEWMAIL);
	    return;
          }

      free(newmail);
    }

  mailbox = malloc(strlen(MAILBOX_PATH)+strlen(userinfo->pw_name)+1);
  check(mailbox);

  strcpy(mailbox,MAILBOX_PATH);
  strcat(mailbox,userinfo->pw_name);

  if ((stat(mailbox, &statbuf) == -1) || (statbuf.st_size == 0))
    printf(MSG_NOMAIL);
  else 

  if (statbuf.st_atime > statbuf.st_mtime)
    printf(MSG_SEENMAIL);
  else
    printf(MSG_NEWMAIL);

  printf("\n\n");
}
   
#endif /* ifdef CHECK_MAIL */



/******************************************************************************/
/* after_login: All the stuff we have to do after a successful login          */
/******************************************************************************/

void
after_login (uid_t uid, char *rmthost, struct lastlog *oldlog)
{
  struct passwd *userinfo;

  userinfo = getpwuid(uid);
  check(userinfo);

  print_welcome (userinfo);
  print_lastlog (oldlog);
  print_motd ();
  printf ("\n");

#ifdef CHECK_MAIL
  check_mail(userinfo);
#endif

  fflush(stdout);
}



/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
