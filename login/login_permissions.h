#ifndef __HAVE_LOGIN_PERMISSIONS_H__
#define __HAVE_LOGIN_PERMISSIONS_H__

#ifdef SUPPORT_USERTTY
int user_allowed(struct passwd *user, char *rmthost);
#endif

#ifdef SUPPORT_SECURETTY
int root_allowed(void);
#endif

#endif
