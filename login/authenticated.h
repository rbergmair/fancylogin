#include <pwd.h>

int auth_normal (struct passwd *passwd, char *clear);
int auth_shadow (struct passwd *passwd, char *clear);
struct passwd *authenticated (char *username, char *password, char *rmthost);
