/*
 * some information on your system-environment:
 *
 * these macros are strings of the complete file-names
 * including the whole path to them:
 *
 *  FILENAME_SIGNON_DEFS    filename of signon.defs
 *  FILENAME_SIGNON_FAIL    filename signon.fail
 *  FILENAME_LOGIN_LOGGING  filename login.logging
 *  FILENAME_USERTTY        filename usertty
 *  FILENAME_SECURETTY      filename securetty
 *  FILENAME_LASTLOG        filename lastlog
 *  SHELL_PARAMS            parameters to pass to shell
 *  EMERGENCY_LOGIN         filename of program to execute
 *                          if fatal errors occur
 */


#define FILENAME_SIGNON_DEFS    "/etc/signon.defs"

#define FILENAME_SIGNON_FAIL    "/etc/signon.fail"

#define FILENAME_SIGNON_TEXT    "/etc/signon.text"

#define FILENAME_SIGNON_COLORS  "/etc/signon.colors"
 
#define FILENAME_LOGIN_LOGGING  "/etc/login.logging"

#define FILENAME_USERTTY        "/etc/usertty"

#define FILENAME_SECURETTY      "/etc/securetty"

#define FILENAME_LASTLOG        "/var/log/lastlog"

#define SHELL_PARAMS            "-login"

#define EMERGENCY_LOGIN         "/bin/login"
