#ifndef __HAVE_ENVIRONMENT_H__
#define __HAVE_ENVIRONMENT_H__

/*
 * some information on your system-environment:
 *
 * these macros are strings of the complete file-names
 * including the whole path to them:
 *
 *  FILENAME_LOGIN_LOGGING  filename login.logging
 *  FILENAME_USERTTY        filename usertty
 *  FILENAME_SECURETTY      filename securetty
 *  FILENAME_LASTLOG        filename lastlog
 *  SHELL_PARAMS            parameters to pass to shell
 *  EMERGENCY_LOGIN         filename of program to execute
 *                          if fatal errors occur
 */


#define FILENAME_LOGIN_LOGGING  "/etc/login.logging"

#define FILENAME_USERTTY        "/etc/usertty"

#define FILENAME_SECURETTY      "/etc/securetty"

#define FILENAME_LASTLOG        "/var/log/lastlog"

#define FILENAME_DEFAULTFLT     "/etc/default.flt"

#define EMERGENCY_LOGIN         "/bin/login"

#define MAILBOX_PATH "/var/spool/mail"



#endif
