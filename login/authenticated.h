#ifndef __HAVE_AUTHENTICATED_H__
#define __HAVE_AUTHENTICATED_H__

#include <pwd.h>

struct passwd *authenticated(char *username, char *password, char *rmthost);

#endif
