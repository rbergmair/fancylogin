/*
 * login_permissions.c  -- fancylogin's routines to figure out what
 *                         user is allowed to login to whitch terminal,
 *                         from whitch network, what time, ...
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
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>


#include "macros.h"
#include "definitions.h"
#include "limits.h"
#include "fancy.h"
#include "log_message.h"
#include "emergency.h"
#include "login.h"





#ifdef SUPPORT_USERTTY

/*
 * We'll need this for parsing the
 * usertty-file.
 */

struct sections
  {
    char sectionname [__MAX_STR_LEN__];
    struct sectionmember *members;
    struct sections *next;
  };

struct sectionmember
  {
    char membername[__MAX_STR_LEN__];
    struct sectionmember *next;
  };

#endif



#ifdef SUPPORT_SECURETTY

/******************************************************************************/
/* root_allowed: checks if /etc/securetty lets root login on this terminal.   */
/******************************************************************************/

int
root_allowed (void)
{
  FILE *securetty;
  char line[__MAX_STR_LEN__];
  char linec[__MAX_STR_LEN__];
  char tty[__MAX_STR_LEN__];

  sscanf (ttyname(0), "/dev/%s", tty);

  if ((securetty = fopen(FILENAME_SECURETTY,"r")) == NULL)
    return TRUE;

  while (!(feof(securetty)))
    {  
      /*
       * compare every line with
       * read a line
       */
      if ((fgets (line, sizeof (line), securetty)) == NULL)
        break;

      /*
       * leave out whitespaces
       */
      sscanf (line, "%s", linec);

      /*
       * if the line matches the terminal
       * we are on.
       */
      if (strcmp(linec, tty)==0)
        {
          /*
           * close the securetty-file and
           * return TRUE
           */
          fclose (securetty);
          return TRUE;
        }
    }

  /*
   * no line matched.
   */
  fclose (securetty);
  return FALSE;
}

#endif



#ifdef SUPPORT_USERTTY

/******************************************************************************/
/* process_section: read a classes section from the file *usertty, and create */
/*                  a linked list (LIFO) containing all classes, and for each */
/*                  class a list of members.                                  */
/******************************************************************************/

struct sections *
process_section (FILE *usertty)
{
  char line[__MAX_STR_LEN__];
  char str[__MAX_STR_LEN__];
  char *ch;

  struct sectionmember *sectionmember_buf;
  struct sectionmember *sectionmember_first=NULL;
  struct sections *section_buf;
  struct sections *section_first=NULL;

  while (fgets(line, sizeof(line), usertty)!=NULL)           /* for each line */
    {
      if (feof(usertty))                                      /* break on EOF */
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
          check (section_buf);
          section_buf->next = section_first;
          section_first = section_buf;


          /* read the name of the new section out of the file */
          sscanf (ch, "%s", section_first->sectionname);
          ch += strlen (section_first->sectionname);
          
          sectionmember_first=NULL;
          
          while ((*(ch) != '\n') && (*(ch)!='\0'))
            {
              /* add a new member to the memberlist */
              sectionmember_buf = (struct sectionmember *)
                                   malloc (sizeof (struct sectionmember));
              check (sectionmember_buf);
              sectionmember_buf->next = sectionmember_first;
              sectionmember_first = sectionmember_buf;

              /* read the membername and membertime*/
             
              sscanf (ch, "%s", sectionmember_first->membername);
 
              ch += strlen (sectionmember_first->membername) + 1;
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
  char line[__MAX_STR_LEN__];
  char str[__MAX_STR_LEN__];

  rewind (input);

  while (!feof(input))
    {
      if (fgets (line, sizeof (line), input)==NULL)
        break;

      sscanf (line, "%s", str);

      if (strcmp(str, pattern)==0)
        break;
    }
  return 0;
}



/******************************************************************************/
/* time_ok: determine wheather the origin allows to login at this time,       */
/*          using the string in the syntax, as defined for HP-UX logins.      */
/******************************************************************************/

int
time_ok (char *param)
{

#ifdef SUPPORT_USERTTY_TIME

  /*
   * this has to be improved some time,
   * the algorithm assumes the daytime (such as 9-17)
   * to come after the weekdays
   */

  if (param!=NULL)
    {

      time_t seconds;
      struct tm *date;
      int weekday_allowed[7]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
      int i;
      int min=0;
      int max=24;
    
      time(&seconds);
      date = localtime(&seconds);

    
      i=0;
      for (;;)
        {
          if ((param[i] >= '0') && (param[i] <= '9'))
            {
              sscanf(&(param[i]), "%d-%d", &min, &max);
              for (;param[i]!=':' && param[i]!='\0';i++);
              if (param[i]=='\0')
                break;
              i++;  
              continue;
            }
    
          switch (param[i])
            {
              case 'm' : weekday_allowed[1]=TRUE;
                         break;
              case 't' : if (param[i+1] == 'u')
                           weekday_allowed[2]=TRUE;
                         else
                           weekday_allowed[4]=TRUE;
                         break;
              case 'w' : weekday_allowed[3]=TRUE;
                         break;
              case 'f' : weekday_allowed[5]=TRUE;
                         break;
              case 's' : if (param[i+1] == 'a')
                           weekday_allowed[6]=TRUE;
                         else
                           weekday_allowed[0]=TRUE;
            }
            
          for (;param[i]!=':' && param[i]!='\0';i++);

          if (param[i]=='\0')
            break;

          i++;
        }  

      if ((weekday_allowed[date->tm_wday]) &&
          (date->tm_hour>=min) &&
          (date->tm_hour<=max))
        return TRUE;
    
      return FALSE;
    }

#endif

   return TRUE;
}



/******************************************************************************/
/* dispose_section: frees the memory for the section                          */
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
/* get_time: parses the part [<return-value>]... out of complete.             */
/******************************************************************************/

char *
get_time (char *complete)
{

#ifdef SUPPORT_USERTTY_TIME

  static char output[__MAX_STR_LEN__];
  int i;

  if (complete[0]=='[')
    {
      for (i=1;complete[i]!=']';i++)
        output[i-1]=complete[i];
      output[i-1]='\0';
      return (char *)&output;
    }

#endif

  return (char *)NULL;
}



/******************************************************************************/
/* get_orig: parses the part [...]<return-value> out of complete.             */
/******************************************************************************/

char *
get_orig (char *complete)
{
  static char output[__MAX_STR_LEN__];
  int i=0;
  if (complete != NULL)
    {
      if (complete[0]=='[')
        {
          for (i=1;complete[i]!=']';i++);
          i++;
        }

      strcpy (output, &(complete[i]));
    
      return (char *)&output;
    }
  return NULL;
}



/******************************************************************************/
/* orig_ok: check wheather the given origin is ok for given remote host.      */
/******************************************************************************/

int
orig_ok (char *tryorig, char *remotehost)
{
  static char terminal[__MAX_STR_LEN__] = {0};
  static struct hostent *remotelogin = NULL;

  if ((tryorig[0]=='@') && (remotehost != NULL) && (remotehost[0] != '\0'))
    {

#ifdef SUPPORT_USERTTY_NETWORK

      /*
       * this macro is used to convert that weird
       * format, ip-address bytes are stored in.
       */

      #define ipaddress(orig) \
      ((orig < 0) ? 256+orig : orig)

      tryorig++;
      if (remotehost[0]=='\0')
        return FALSE;

      if (remotelogin == NULL)
        {
          /*
           * get the data of the host, that is trying to log in.
           * with the second call we ensure, that we get everything,
           * in case an IP-Address is passed by telnetd/rlogind
           */

          remotelogin = gethostbyname (remotehost);
        }

      if (strchr(tryorig,'/')!=NULL)
        {
          int ipaddr[4];
          int netmask[4];
          int *min_ip;
          int max_ip[4];
          int i,j;
          int k=0;
          int is_ok;
          char *last;


          /* convert the string to an integer-array */
          last = tryorig;
          for (i=0;;i++)
            if ((tryorig[i]=='.') || (tryorig[i]=='/'))
              {
                tryorig[i]='\0';
                ipaddr[k]=ipaddress(atoi(last));
                last=tryorig+i+1;
                if (k==3)
                  break;
                k++;
              }

          tryorig+=i+1;
          k=0;
          last=tryorig;

          for (i=0;;i++)
            if ((tryorig[i]=='.') || (tryorig[i]=='\0'))
              {
                tryorig[i]='\0';
                netmask[k]=ipaddress(atoi(last));
                last = tryorig+i+1;
                if (k==3)
                  break;
                k++;
              }

          min_ip=ipaddr;
          for (i=0;i<4;i++)
            {
              max_ip[i]=min_ip[i]+255-netmask[i];
              if (max_ip[i] > 255)
                max_ip[i]=255;
            }

          for (j=0;;)
            {
              is_ok=TRUE;
              for (i=0;i<4;i++)
                if (!(
                      (min_ip[i]
                          <=
                       ipaddress(remotelogin->h_addr_list[j][i]))
                          &&
                      (ipaddress(remotelogin->h_addr_list[j][i])
                          <=
                       max_ip[i])
                     ))
                  {
                    is_ok = FALSE;
                    break;
                  }

              if (is_ok)
                return TRUE;

              j++;

              if (remotelogin->h_addr_list[j]==NULL)
                return FALSE;
            }
        }
      else
        {
          /* at this point we are sure, that we have no ip-address */
          int i;
          int j;
          int k=-1;
          int is_ok;
          char *realname;

          for (realname=remotelogin->h_name;
               remotelogin->h_aliases[k]!=NULL;
               realname=remotelogin->h_aliases[k])
            {
              is_ok = TRUE;
              for (i=strlen(tryorig),j=strlen(realname);
                   i>=0 && j>=0;
                   i--,j--)
                if (realname[j]!=tryorig[i])
                  {
                    is_ok = FALSE;
                    break;
                  }

              if (is_ok)
                return TRUE;

              k++;
           }

          return FALSE;
        }

#endif

      return FALSE;
    }

  if (terminal[0]==0)
    sscanf (ttyname(0), "/dev/%s", terminal);

  return (strcmp(terminal, tryorig) == 0);

}



/******************************************************************************/
/* user_allowed: checks wheather /etc/usertty allows to log on to system.     */
/******************************************************************************/

int
user_allowed (struct passwd *user, char *rmthost)
{
  FILE *usertty;
  struct sections *classes;
  struct sections *users;
  struct sections *groups;
  struct sections *cur1; 
  struct sections *cur3;
  struct sectionmember *cur2;
  struct sectionmember *cur4;
  char terminal[10];
  char username[20];
  struct group *usergroup;

 
  /*
   * this macro is used to stop the
   * function and tell the caller, that
   * /etc/usertty allows the user to log on
   */

  #define ACCESS_GRANTED()       \
    {                            \
                                 \
      dispose_section (users);   \
      dispose_section (groups);  \
      dispose_section (classes); \
                                 \
      return TRUE;               \
    }


  if ((usertty = fopen(FILENAME_USERTTY,"r"))==NULL)
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

            if (cur1==NULL)       /* no default rule means standard behavior, */
              ACCESS_GRANTED ()   /* which is letting the user pass. */
            else
              {
                username[0] = '*';
                username[1] = '\0';
              }
          }
    }


  /*
   * now comes my favourite piece of code in this program!
   */

  /*
   * look up user's permissions
   */

  for (cur1=users;cur1!=NULL;cur1=cur1->next)
    if (strcmp (cur1->sectionname, username) == 0)
      {                               /* added braces to make gcc -Wall happy */
        for (cur2=cur1->members;cur2!=NULL;cur2=cur2->next)
          if (orig_ok (get_orig (cur2->membername), rmthost) &&
              time_ok (get_time (cur2->membername)))
            ACCESS_GRANTED ()
          else
            for (cur3=classes;cur3!=NULL;cur3=cur3->next)
              if (strcmp (cur2->membername, cur3->sectionname) == 0)
                for (cur4=cur3->members;cur4!=NULL;cur4=cur4->next)
                  if (orig_ok (get_orig (cur4->membername), rmthost) &&
                      time_ok (get_time (cur4->membername)))
                    ACCESS_GRANTED ()
      }


  /*
   * look up the group's permissions
   */

  for (cur1=groups;cur1!=NULL;cur1=cur1->next)
    if (strcmp (cur1->sectionname, usergroup->gr_name) == 0)
      {                               /* added braces to make gcc -Wall happy */
        for (cur2=cur1->members;cur2!=NULL;cur2=cur2->next)
          if (orig_ok (get_orig (cur2->membername) , rmthost) &&
              time_ok (get_time (cur2->membername)))
            ACCESS_GRANTED ()
          else
            for (cur3=classes;cur3!=NULL;cur3=cur3->next)
              if (strcmp (cur2->membername, cur3->sectionname) == 0)
                for (cur4=cur3->members;cur4!=NULL;cur4=cur4->next)
                  if (orig_ok (get_orig (cur4->membername), rmthost) &&
                      time_ok (get_time (cur4->membername)))
                    ACCESS_GRANTED ()
      }


  dispose_section (users); 
  dispose_section (groups); 
  dispose_section (classes); 

  return FALSE;
}

#endif

/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
