/* login.c  -- fancylogin-login program.
               fancylogin uses ncurses to display a colorful login-
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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <shadow.h>
#include <utmp.h>

#include "definitions.h"
#include "fancy.h"

#define __LEN_USERNAME__ 20
#define __LEN_PASSWORD__ 10




/******************************************************************************/
/* auth_normal: tries to authenticate the given password from passwd          */
/*              (passwd) and the password entered by the user (password)      */
/*              using standard-authentication.                                */
/******************************************************************************/

int
auth_normal (struct passwd *passwd, char *clear)
{
  /*
   * crypt the cleartext-password
   * using the salt-string, and finally
   * see if it's the same as entered
   */ 

  return (strcmp ((char *)crypt(clear, passwd->pw_passwd), passwd->pw_passwd)
          == 0);
}



/******************************************************************************/
/* auth_shadow: tries to authenticate the given password from passwd          */
/*              (passwd) and the password entered by the user (password)      */
/*              using shadow-authentication                                   */
/******************************************************************************/

int
auth_shadow (struct passwd *passwd, char *clear)
{
  struct spwd *entry;
  entry = getspnam (passwd->pw_name);
  return (strcmp ((char *)crypt(clear, entry->sp_pwdp), entry->sp_pwdp) == 0);
}


/******************************************************************************/
/* authenticated: determines whether username and password are correct        */
/*                returns NULL if not authenticated, else return the user-    */
/*                structure.                                                  */
/******************************************************************************/

struct passwd *
authenticated (char *username, char *password)
{
  struct passwd *passwd_entry;

  /*
   * if (((strcmp (username, "DMR"))==0) && (strcmp (password, "ABCD") == NULL))
   *   return TRUE;
   *
   * Greetings to Dennis Ritchie!
   */


  /*
   * did the user enter a name?
   */

  if (username[0]=='\0')
    return NULL;


  /*
   * is the user known to the system?
   */ 

  if ((passwd_entry = getpwnam (username)) == NULL)
    return NULL;

 
  /*
   * is the user locked?
   */ 

  if ((passwd_entry->pw_passwd[0])=='!')
    return NULL;


  /*
   * does the user have a password?
   */

  if (passwd_entry->pw_passwd[0]=='\0')
    return passwd_entry;


  /*
   * is the password correct?
   */

  if (strcmp(passwd_entry->pw_passwd,"x")==0)  /* Do we use shadow? */
    {                                          /* -> yes? */
      if (auth_shadow (passwd_entry, password))
        return passwd_entry;
    }
  else                                         /* -> no? */ 
    if (auth_normal (passwd_entry, password))
      return passwd_entry;


  return NULL;
}


  
/******************************************************************************/
/* getnewenv: returns the environment, the shell needs from us, to give       */
/*            username, our PID, home-directory, ...                          */
/******************************************************************************/

char **
getnewenv (struct passwd *user)
{
  #define __NUM_VARIABLES__ 6 
  #define __MAX_STR_LEN__ 30
          /* Does anyone know the defininitons of how long such an
          environment-string can be (__MAX_STR_LEN__) ? */


  int i;


  /*
   * allocate memory for the environment
   */

  char **ne;

  if ((ne = (char **)malloc( sizeof(char *) * (__NUM_VARIABLES__+1))) == NULL)
    {
      fprintf (stderr, "Error allocating memory!\n");
      return NULL;
    }

  for (i=0;i<7-1;i++)
    if ((ne[i] = (char *)malloc(sizeof(char) * __MAX_STR_LEN__+1)) == NULL)
      {
        fprintf (stderr, "Error allocating memory!\n");
        return NULL;
      }


  /*
   * put some values in there
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
system_logon (struct passwd *user)
{
  char **new_environment;
  int i;
  struct utmp *utmpentry;


  printf("\fWelcome, %s!\nYou are being logged on...", user->pw_gecos);


  /*
   * Update utmp and wtmp entries
   */

  utmpentry = (struct utmp *) malloc (sizeof(struct utmp));
  
  utmpentry->ut_type = USER_PROCESS;                        /* type of login */ 
  utmpentry->ut_pid = getpid ();                                  /* our pid */
  strcpy (utmpentry->ut_user, user->pw_name);                    /* username */
  time (&(utmpentry->ut_tv.tv_sec));                         /* current time */
  sscanf (ttyname(0), "/dev/%s", utmpentry->ut_line);                /* line */

  /* I don't care about that remote-login stuff, because */
  /* fancylogin is not supposed to be used for that. */

  updwtmp ("/var/log/wtmp", utmpentry);  
  pututline (utmpentry);  


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
   * Change to user's uid and gid
   */

  setuid (user->pw_uid);
  setgid (user->pw_gid);

  printf("Done.\n");


  /*
   * exec user's shell
   */

  execle (user->pw_shell, user->pw_shell, "--login", NULL, new_environment);
}



/******************************************************************************/
/* Main-program                                                               */
/******************************************************************************/

int
main (void)
{
  char username[__LEN_USERNAME__+1];  
  char password[__LEN_PASSWORD__+1];

  struct passwd *user;

  printf ("This is fancylogin 0.99.3 (http://fancylogin.cjb.net).\n\n");
  sleep (1);
  
  initialize_prompt ();

  while (1)
    {
      fancy_prompt (username, password);
      if (!((user = authenticated (username, password))==NULL))
        break;
      else
        fail_login ();
    }
  
  close_prompt ();

  system_logon (user);
}

