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
#include <grp.h>
#include <sys/types.h>
#include <shadow.h>
#include <utmp.h>

#include "definitions.h"
#include "fancy.h"

#define __LEN_USERNAME__ 20
#define __LEN_PASSWORD__ 10



/******************************************************************************/
/* DEFINITIONS                                                                */
/******************************************************************************/

struct sections
  {
    char sectionname [20];
    struct sectionmember *members;
    struct sections *next;
  };

struct sectionmember
  {
    char membername[20];
    char membertime[128];
    struct sectionmember *next;
  };

int auth_normal (struct passwd *passwd, char *clear);
int auth_shadow (struct passwd *passwd, char *clear);
int root_allowed (void);
struct classes *process_classes (FILE *usertty);
int file_pos_str(FILE *input, char *pattern);
int user_allowed (struct passwd *user);
struct passwd *authenticated (char *username, char *password);
char **getnewenv (struct passwd *user);
int system_logon (struct passwd *user);



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
/* root_allowed: checks if /etc/securetty lets root login on this terminal.   */
/******************************************************************************/

int
root_allowed (void)
{
  FILE *securetty;
  char line[10];

  if ((securetty = fopen("/etc/securetty","r")) == NULL)
    return TRUE;

  while (!(feof(securetty)))
    {  
      if ((fgets (line, sizeof (line), securetty)) == NULL)
        break;

      if (strcmp(line, ttyname(0))==0)
        {
          fclose (securetty);
          return TRUE;
        }
    }

  fclose (securetty);
  return FALSE;
}



/******************************************************************************/
/* process_section: read a classes section from the file *usertty, and create */
/*                  a linked list (LIFO) containing all classes, and for each */
/*                  class a list with members.                                */
/******************************************************************************/

struct sections *
process_section (FILE *usertty)
{
  char line [256];
  char str [256];
  char *ch;
  int n=1;

  struct sectionmember *sectionmember_buf;
  struct sectionmember *sectionmember_first=NULL;
  struct sections *section_buf;
  struct sections *section_first=NULL;


  while (1)
    {
      if (fgets(line, sizeof(line), usertty)==NULL)
        break;

      if (feof(usertty))
        break;

      if ((line != NULL) && (line[0]!='\n') && (line[0]!='#'))
        {
          /* now we've got a new line to process */

          /* does this line tell us to stop processing ? */
          sscanf (line, "%s", str);
          if ((strcmp(str, "GROUPS") == 0) || (strcmp(str, "USERS") == 0) ||
              (strcmp(str, "CLASSES") == 0)) 
            break;

          /* start processing the line */
          ch = line;

          /* add a new item to the list */
          section_buf = (struct sections *) malloc (sizeof (struct sections));
          section_buf->next = section_first;
          section_first = section_buf;

          /* read the name of the new section out of the file */
          sscanf (ch, "%s", section_first->sectionname);
          ch += strlen (section_first->sectionname);
          
          sectionmember_first=NULL;
          
          while (*(ch) != '\n')
            {
              /* add a new member to the memberlist */
              sectionmember_buf = (struct sectionmember *)
                                   malloc (sizeof (struct sectionmember));
              sectionmember_buf->next = sectionmember_first;
              sectionmember_first = sectionmember_buf;

              /* read the membername and membertime*/
             
              sscanf (ch, "%s", str);
              if (str[0]=='[')
                sscanf (str, "[%s]%s", sectionmember_first->membertime,
                        sectionmember_first->membername); 
              else
                {
                  sscanf (ch, "%s", sectionmember_first->membername);
                  sectionmember_first->membertime[0]='\0';
                }
 
              ch += strlen (str) + 1;
            }

          section_first->members = sectionmember_first;
        }      
    }

  return section_first;
}  



/******************************************************************************/
/* file_pos_str: position file pointer to where string is first found         */
/******************************************************************************/

int
file_pos_str(FILE *input, char *pattern)
{
  char line[256];
  char str[256];

  rewind (input);

  while (!feof(input))
    {
      if (fgets (line, sizeof (line), input)==NULL)
        break;

      sscanf (line, "%s", str);

      if (strcmp(str, pattern)==0)
        break;
    }
}



/******************************************************************************/
/* time_ok: determine wheather the origin allows to login at this time,       */
/*          using the string in the syntax, as defined for HP-UX logins.      */
/******************************************************************************/

int
time_ok (char *allowed)
{
  /*
   * any volunteers to do that? (At the moment it is 21:27, and I don't 
   * want to do any more coding!), Tomorrow I'll start with the syslog-support!
   */

  return TRUE;
}



/******************************************************************************/
/* user_allowed: checks wheather /etc/usertty allows to log on to system.     */
/******************************************************************************/

void
dispose_section (struct sections *sect)
{
  struct sections *sections_buf;
  struct sections *sections_next;
  struct sectionmember *sectionm_buf;
  struct sectionmember *sectionm_next;

  for (sections_buf=sect;sections_buf!=NULL;sections_buf=sections_next)
    {
      sections_next = sections_buf->next;
      for (sectionm_buf=sections_buf->members;sectionm_buf!=NULL;
           sectionm_buf=sectionm_next)
        {
          sectionm_next = sectionm_buf->next;
          free (sectionm_buf);
        }
      free (sections_buf);
    }  
}



/******************************************************************************/
/* user_allowed: checks wheather /etc/usertty allows to log on to system.     */
/******************************************************************************/

int
user_allowed (struct passwd *user)
{
  FILE *usertty;
  struct sections *classes;
  struct sections *users;
  struct sections *groups;
  struct sections *cur1; 
  struct sections *cur3;
  struct sectionmember *cur2;
  struct sectionmember *cur4;
  char line[256];
  char terminal[10];
  char username[20];
  struct group *usergroup;

  if ((usertty = fopen("/etc/usertty","r"))==NULL)
    return TRUE;

  file_pos_str (usertty, "CLASSES");
  classes = process_section (usertty);
  
  file_pos_str (usertty, "USERS");
  users = process_section (usertty);
  
  file_pos_str (usertty, "GROUPS");
  groups = process_section (usertty);

  sscanf (ttyname(0), "/dev/%s", terminal);
  usergroup = getgrgid (user->pw_gid);
  strcpy (username, user->pw_name);



  /*
   * Is the user mentioned, or do we have to apply
   * default rules?
   */

  for (cur1=users;cur1!=NULL;cur1=cur1->next)
    if (strcmp (user->pw_name, cur1->sectionname) == 0)
      break;

  if (cur1==NULL)
    {
        /* user is not mentioned explicitly, is his group mentioned? */

        for (cur1=groups;cur1!=NULL;cur1=cur1->next)
          if (strcmp (cur1->sectionname, usergroup->gr_name) == 0)
            break;

        if (cur1==NULL)
          {
            /* the group is not mentioned, we'll apply default rules: */
            /* are default rules mentioned? */

            for (cur1=users;cur1!=NULL;cur1=cur1->next)
              if (strcmp (cur1->sectionname, "*") == 0)
                break;

            if (cur1==NULL) /* no default rule means standard behavior, */
              goto access_granted;  /* which is letting the user pass. */
            else
              {
                username[0] = '*';
                username[1] = '\0';
              }
          }
    }



  /* now comes my favourite piece of code in this program! */

  /*
   * look up user's permissions
   */

  for (cur1=users;cur1!=NULL;cur1=cur1->next)
    if (strcmp (cur1->sectionname, username) == 0)
      for (cur2=cur1->members;cur2!=NULL;cur2=cur2->next)
        if ((strcmp (terminal, cur2->membername) == 0) &&
            time_ok (cur2->membertime))
          goto access_granted;
        else
          for (cur3=classes;cur3!=NULL;cur3=cur3->next)
            if (strcmp (cur2->membername, cur3->sectionname) == 0)
              for (cur4=cur3->members;cur4!=NULL;cur4=cur4->next)
                if ((strcmp (terminal, cur4->membername) == 0) &&
                    time_ok (cur4->membertime))
                  goto access_granted;


  /*
   * look up the group's permissions
   */

  for (cur1=groups;cur1!=NULL;cur1=cur1->next)
    if (strcmp (cur1->sectionname, usergroup->gr_name) == 0)
      for (cur2=cur1->members;cur2!=NULL;cur2=cur2->next)
        if ((strcmp (terminal, cur2->membername) == 0) &&
            time_ok (cur2->membertime))
          goto access_granted;
        else
          for (cur3=classes;cur3!=NULL;cur3=cur3->next)
            if (strcmp (cur2->membername, cur3->sectionname) == 0)
              for (cur4=cur3->members;cur4!=NULL;cur4=cur4->next)
                if ((strcmp (terminal, cur4->membername) == 0) &&
                    time_ok (cur4->membertime))
                  goto access_granted;

  access_denied:
    dispose_section (users);
    dispose_section (groups);
    dispose_section (classes);
    return FALSE;

  access_granted:
    dispose_section (users);
    dispose_section (groups);
    dispose_section (classes);
    return TRUE;

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
   * is root trying to log on to a terminal, he is not allowed
   * to log on to ?
   */

  if ((passwd_entry->pw_uid==0) && !(root_allowed))
    return NULL;

 
  /*
   * is the user locked?
   */ 

  if ((passwd_entry->pw_passwd[0])=='!')
    return NULL;


  /*
   * Is the user allowed to log on to this terminal
   * at this time, according to /etc/usertty?
   */

  if ((passwd_entry->pw_uid != 0) && (!(user_allowed (passwd_entry))))
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

  printf ("This is fancylogin 0.99.4 (http://fancylogin.cjb.net).\n\n");
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

