/*
 * authenticated.c  -- fancylogin user-authentication.
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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>

#include "config.h"
#include "definitions.h"
#include "limits.h"
#include "log_message.h"
#include "login_permissions.h"

/*
 * I couldn't find the header for
 * the deklaration of crypt, so I just inserted
 * the deklaration from the manpage literally.
 */

char *crypt(const char *key, const char *salt);



#ifdef ALLOW_NORMAL

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

#endif



#ifdef ALLOW_SHADOW

/******************************************************************************/
/* auth_shadow: tries to authenticate the given password from passwd          */
/*              (passwd) and the password entered by the user (password)      */
/*              using shadow-authentication                                   */
/******************************************************************************/

int
auth_shadow (struct passwd *passwd, char *clear)
{
  /*
   * same as auth_normal, but before
   * verifying the password, we first get it
   * from shadow
   */

  struct spwd *entry;

  entry = getspnam (passwd->pw_name);

  return (strcmp ((char *)crypt(clear, entry->sp_pwdp), entry->sp_pwdp) == 0);
}

#endif



/******************************************************************************/
/* authenticated: determines whether username and password are correct        */
/*                returns NULL if not authenticated, else return the user-    */
/*                structure.                                                  */
/******************************************************************************/

struct passwd *
authenticated (char *username, char *password, char *rmthost)
{
  struct passwd *passwd_entry;
  char errormessage [__MAX_STR_LEN__];

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
    {
      sprintf (errormessage, "%s is not known to the system!", username);
      log_message (49998, errormessage);
      log_message (49999, errormessage);
      return NULL;
    }


  /*
   * is root trying to log on to a terminal, he is not allowed
   * to log on to ?
   */

#ifdef SUPPORT_SECURETTY

  if ((passwd_entry->pw_uid==0) && !(root_allowed()))
    {
      sprintf (errormessage, "accorting to /etc/securetty root is not allowed to log here");
      log_message (40000, errormessage); 
      log_message (49999, errormessage); 
      return NULL;
    }

#endif

 
  /*
   * is the user locked?
   */ 

  if ((passwd_entry->pw_passwd[0])=='!')
    {
      sprintf (errormessage, "%s is locked", passwd_entry->pw_name);
      log_message (40000+passwd_entry->pw_uid, errormessage);
      log_message (4999, errormessage);
      return NULL;
    }


  /*
   * Is the user allowed to log on to this terminal
   * at this time, according to /etc/usertty?
   */

#ifdef SUPPORT_USERTTY

  if ((passwd_entry->pw_uid != 0) && (!(user_allowed (passwd_entry, rmthost))))
    {
      sprintf (errormessage, "according to /etc/usertty %s is not allowed to log on here.", passwd_entry->pw_name);
      log_message (40000+passwd_entry->pw_uid, errormessage);
      log_message (49999, errormessage);
      return NULL;
    }

#endif


  /*
   * does the user have a password?
   */

  if (passwd_entry->pw_passwd[0]=='\0')
    {
      sprintf (errormessage, "%s logged in", passwd_entry->pw_name);
      log_message (30000+passwd_entry->pw_uid, errormessage);
      log_message (39999, errormessage);
      return passwd_entry;
    }


  /*
   * is the password correct?
   */

  if (strcmp(passwd_entry->pw_passwd,"x")==0)  /* Do we use shadow? */
    {                                          /* -> yes? */

#ifdef ALLOW_SHADOW

      if (auth_shadow (passwd_entry, password))
        {
          sprintf (errormessage, "%s logged in", passwd_entry->pw_name);
          log_message (30000+passwd_entry->pw_uid, errormessage);
          log_message (39999, errormessage);
          return passwd_entry;
        }

#endif

    }
  else                                         /* -> no? */ 
    {

#ifdef ALLOW_NORMAL

      if (auth_normal (passwd_entry, password))
        {
          sprintf (errormessage, "%s logged in", passwd_entry->pw_name);
          log_message (30000+passwd_entry->pw_uid, errormessage);
          log_message (39999, errormessage);
          return passwd_entry;
        }

#endif

    }


  /*
   * Log that the authentication failed
   */

  sprintf (errormessage, "%s entered a wrong password", passwd_entry->pw_name);
  log_message (40000+passwd_entry->pw_uid, errormessage);
  log_message (49999, errormessage);
  return NULL;
}

/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
