/*
 * emergency.c  -- fancylogin emergency-handling
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



#include <malloc.h>
#include <unistd.h>

#include "config.h"
#include "environment.h"
#include "limits.h"
#include "log_message.h"
#include "fancy.h"



/******************************************************************************/
/* check: should be used after every malloc.                                  */
/******************************************************************************/
  
void
check(void *checkfor) 
{
  if (checkfor == NULL)
    {
      log_message (2, "Out of memory, executing emergency login (/bin/login)");

      close_prompt ();

#ifdef EXECUTE_EMERGENCY_LOGIN
      for (;;)
        {
          execl (EMERGENCY_LOGIN, EMERGENCY_LOGIN, NULL);
          log_message (1, "Because I ran out of memory, I tried to execute the \
                           emergency login (EMERGENCY_LOGIN), which failed.");
          sleep (5*60);                                                        
        }
#endif
      exit (-1);
    }                                                                          
}



/******************************************************************************/
/* fatal: should be called for fatal errors.                                  */
/******************************************************************************/

void
fatal (void)
{
  close_prompt ();
 
#ifdef EXECUTE_EMERGENCY_LOGIN
  for (;;)
    {
      execl (EMERGENCY_LOGIN, EMERGENCY_LOGIN, NULL);
      log_message (1, "Because I ran out of memory, I tried to execute the \
                       emergency login (/bin/login), which failed.");
      sleep (5*60);
    }
#endif
  exit (-2);
}



/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
