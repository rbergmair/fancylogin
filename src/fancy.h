/* fancy.h  -- Header-file for fancy.o.
               Fancylogin uses ncurses to display a colorful login-
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


/*
 * puts the prompt
 */

int
fancy_prompt (char *user, char *password);



/*
 * puts the error-message
 */

int
fail_login (void);



/*
 * must be called before fancy_prompt, or fail_login
 */

int 
initialize_prompt (void);



/*
 * must be called after fancy_prompt and fail_login
 */

int
close_prompt (void);
