/*
 * signal_handle.c  -- fancylogin-signal-handling
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
#include "fancy.h"
#include "log_message.h"
#include "emergency.h"

#include <signal.h>





static void handle_sigint(int val);
static void handle_sigterm(int val);
static void handle_sigsegv(int val);
static void handle_misc(int val);
void install_handlers(void);
  



/******************************************************************************/
/* handle_sigint: handler for SIGINT                                          */
/******************************************************************************/

void
handle_sigint (int val)
{
  close_prompt ();
  exit (-1);
}



/******************************************************************************/
/* handle_sigterm: handler for SIGTERM                                        */
/******************************************************************************/

void
handle_sigterm (int val)
{
  close_prompt ();
  exit (0);
}



/******************************************************************************/
/* handle_sigsegv: handler for SIGSEGV                                        */
/******************************************************************************/

void
handle_sigsegv (int val)
{
  close_prompt ();
  log_message (1, "OOPS! I recieved a SIGSEGV, please report this error, \
                   sending us the core-dump, your configuration files, \
                   and some information on your system.");
  fatal ();
}



/******************************************************************************/
/* handle_misc: handler for some other error-signals                          */
/******************************************************************************/

void
handle_misc (int val)
{
  close_prompt ();
  log_message (1, "OOPS! I recieved an error-signal, please report this error, \
                   sending us the core-dump, your configuration files, \
                   and some information on your system.");
  fatal ();
}



/******************************************************************************/
/* install_handlers: installs the signal-handlers                             */
/******************************************************************************/

void
install_handlers (void)
{
  signal (SIGINT, handle_sigint);
  signal (SIGILL, handle_misc);
  signal (SIGSEGV, handle_sigsegv);
  signal (SIGTERM, handle_sigterm);
  signal (SIGFPE, handle_misc);
}



/******************************************************************************/
/* (c) Copyright 1999-2000 Richard Bergmair, remember this program is GPLed!  */
/******************************************************************************/
