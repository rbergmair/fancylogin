#ifndef __HAVE_CONFIG_H__
#define __HAVE_CONFIG_H__


/*
 * define SUPPORT_USERTTY to get support for /etc/usertty-files
 * if unsure, don't do it.
 */

/* #define SUPPORT_USERTTY */


/*
 * define SUPPORT_USERTTY_NETWORK to get support for /etc/usertty's
 * network-functionality
 * if unsure, don't do it. 
 */

/* #define SUPPORT_USERTTY_NETWORK */


/*
 * define SUPPORT_USERTTY_TIME to get support for /etc/usertty's
 * time-functionality
 * if unsure, don't do it.
 */

/* #define SUPPORT_USERTTY_TIME */


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
 * if you don't want login to terminate at all, don't define it.
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
"This is fancylogin 0.99.7\n\
<http://fancylogin.sourceforge.net/>.\n\n"


/*
 * define ALLOW_NORMAL if you want to allow authenticating
 * using DES-encryption. If you are unsure define it.
 */

#define ALLOW_DES


/*
 * define ALLOW_MD5 if you want to allow authenticating
 * using MD5-encryption. If you are unsure define it.
 */

#define ALLOW_MD5


/*
 * define SET_PATH_ENV, to set the path-variable, corresponding
 * to you user-id, either to /usr/local/bin:/bin:/usr/bin:., if
 * you are a normal user, and to /sbin:/bin:/usr/sbin:/usr/bin
 * if you are root.
 *
 * If you want fancylogin to act like the "normal" login, define
 * this. I personally don't want every program to reset the
 * environment, without me knowing what is actually going on,
 * but login was designed to do so.
 *
 * if you are unsure define it.
 */

#define SET_PATH_ENV


/*
 * define TIMEOUT, if you want to use a timeout. The login times
 * out after TIMEOUT seconds
 */

/* #define TIMEOUT 60 */


/*
 * define the following stuff, if you want that fancylogin
 * checks for new E-mails in the system-mailbox
 */

#define CHECK_MAIL
#define MSG_NEWMAIL "You've got mail!"
#define MSG_SEENMAIL "You've got old mail."
#define MSG_NOMAIL "No mails for you."




#endif
