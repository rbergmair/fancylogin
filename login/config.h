/*
 * define SUPPORT_USERTTY to get support for /etc/usertty-files
 * if unsure, don't do it.
 */

#define SUPPORT_USERTTY


/*
 * define SUPPORT_USERTTY_NETWORK to get support for /etc/usertty's
 * network-functionality
 * if unsure, don't do it. 
 */

#define SUPPORT_USERTTY_NETWORK


/*
 * define SUPPORT_USERTTY_TIME to get support for /etc/usertty's
 * time-functionality
 * if unsure, don't do it.
 */

#define SUPPORT_USERTTY_TIME


/*
 * define SUPPORT_SECURETTY to get support for /etc/securetty.
 * if unsure define it.
 */

#define SUPPORT_SECURETTY


/*
 * define SUPPORT_SYSLOG if you want fancylogin to support
 * logging through syslogd.
 * if unsure don't do it.
 */

#define SUPPORT_SYSLOG


/*
 * define SUPPORT_FILELOG if you want fancylogin to support
 * logging through log-files.
 * if unsure define it.
 */

#define SUPPORT_FILELOG


/*
 * define ALLOW_SHADOW if you want to allow shadow's password-verification.
 * if unsure define it.
 */

#define ALLOW_SHADOW


/*
 * define ALLOW_NORMAL if you want to allow non-shadow password-verification.
 * if unsure define it.
 */

#define ALLOW_NORMAL


/*
 * set CLOSE_AFTER_FAILED if you want login to terminate
 * after CLOSE_AFTER_FAILED failed logins.
 *
 * #define CLOSE_AFTER_FAILED 3
 *
 * for example means, that login terminates after 3 failed
 * logins
 *
 * if you don't want login to terminate at all, set to 0.
 */

#define CLOSE_AFTER_FAILED 5


/*
 * set EXECUTE_EMERGENCY_LOGIN if you want emergency-login
 * (EMERGENCY_LOGIN from environment.h) to be called on fatal
 * errors
 */

#define EXECUTE_EMERGENCY_LOGIN


/*
 * set INTRO_STRING to a string, that should be shown on
 * startup of the login-program. If you don't define it, no
 * startup-string is shown
 */

#define INTRO_STRING \
"This is fancylogin 0.99.6 \n\
(http://fancylogin.cjb.net).\n\n"
