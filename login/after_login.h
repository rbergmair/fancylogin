#ifndef __HAVE_AFTER_LOGIN_H__
#define __HAVE_AFTER_LOGIN_H__

#include <sys/types.h>
#include <pwd.h>

void after_login(uid_t uid, char *rmthost, struct lastlog *oldlog);

#endif
