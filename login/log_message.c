/*
 * log_message.c  -- fancylogin logging capabilities
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef SUPPORT_SYSLOG
#include <syslog.h>
#endif

#include "limits.h"
#include "definitions.h"
#include "log_message.h"

/*
 * Please excuse my using a global-variable,
 * but I'm just too lazy to pass this parameter
 * through the program over and over again,
 * because almost every function calls these
 * logging-functions, and passing the rmthost
 * variable to all of these functions wouldn't
 * be very readable.
 */

static char rmthost[__MAX_STR_LEN__]="";





void set_rmthost (char *input);
int log_message (int msgid, char *msg);





/******************************************************************************/
/* set_rmthost: sets the global variable rmthost                              */
/******************************************************************************/

void
set_rmthost (char *input)
{
  strcpy (rmthost, input);
}



/******************************************************************************/
/* log_message: log the message with message-id msgid and the message-string  */
/*              msg.                                                          */
/******************************************************************************/

int
log_message (int msgid, char *msg)
{
  char message[__MAX_STR_LEN__];
  char xmessage[__MAX_STR_LEN__];
  char line[__MAX_STR_LEN__];
  char msgtype[__MAX_STR_LEN__];
  char logtype[__MAX_STR_LEN__];
  char logarg[__MAX_STR_LEN__];
  char chtime[__MAX_STR_LEN__];
  int msgarg;
  int fmsgid;
  time_t xxtime;
  
  FILE *logging;
  FILE *logfile;


  /*
   * What category is msgid of?
   */

  switch ((int)(msgid / 10000))
    {
      case 0: sprintf(message, "FATAL ERROR: %s", msg);     break;
      case 1: sprintf(message, "NON-FATAL ERROR: %s", msg); break;
      case 2: sprintf(message, "FAILED LOGINS: %s", msg);   break;
      case 3: sprintf(message, "ACCESS GRANTED: %s", msg);  break;
      case 4: sprintf(message, "ACCESS DENIED: %s", msg);   break;
      default: sprintf(message, "OOPS #001");
    }


  /*
   * open the file login.logging
   */

  if ((logging = fopen (FILENAME_LOGIN_LOGGING,"r")) == NULL)
    {
      if ((int)(msgid / 10000) == 0)
        fprintf(stderr, "%s", message);
      return FALSE;
    }


  while (!(feof(logging)))
    {
     /*
      * read the line
      */

     if (fgets(line, sizeof (line), logging) == NULL)
        break;


      /*
       * parse the line
       */

      sscanf (line, "%s %d %s %s", msgtype, &msgarg, logtype, logarg);

      fmsgid = 0;


      /*
       * determine fmsgid, the message
       * as given in the file.
       */

      if (strcmp(msgtype, "FATAL") == 0)
        fmsgid = 0;
      if (strcmp(msgtype, "NONFATAL") == 0)
        fmsgid = 10000;
      if (strcmp(msgtype, "FAILED") == 0)
        fmsgid = 20000;
      if (strcmp(msgtype, "GRANTED") == 0)
        fmsgid = 30000;
      if (strcmp(msgtype, "DENIED") == 0)
        fmsgid = 40000;

      fmsgid+=msgarg;

      if (fmsgid == msgid)
        {

#ifdef SUPPORT_FILELOG

          if (strcmp (logtype, "FILE") == 0)
            {
              /*
               * add the timestamp and terminal to the logging-string
               */
              time (&xxtime);
              strcpy (chtime, ctime(&xxtime));
              chtime[strlen(chtime)-1] = '\0';
              sprintf (xmessage, "login from %s on %s: %s",
                                 (rmthost[0]==0 ? ttyname(0) : rmthost),
                                 chtime, message);

              /*
               * write the string to the file
               */
              if ((logfile = fopen (logarg, "at")) == NULL)
                  break;
              fprintf (logfile, "%s\n", xmessage);
              fclose (logfile);
            }

#endif

#ifdef SUPPORT_SYSLOG

          if (strcmp (logtype, "SYSLOG") == 0)
            {
              /*
               * we don't have to add a timestamp here,
               * because syslog will do it for us, so we
               * just add the terminal, the login is from
               */
              sprintf (xmessage, "from %s: %s",
                                  (rmthost[0]==0 ? ttyname(0) : rmthost),
                                  message);

              /*
               * syslog it
               */
              openlog ("fancylogin", 0, atoi(logarg));
              if ((int)(msgid / 10000)==0)
                syslog (LOG_CRIT, "%s\n", xmessage);
              else
                syslog (LOG_INFO, "%s\n", xmessage);
              closelog ();
            }

#endif

          break;
        }
    }

  return TRUE;  
}



/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
