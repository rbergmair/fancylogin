#ifndef __HAVE_FANCY_H__
#define __HAVE_FANCY_H__

int initialize_prompt(void);
void fancy_prompt(char *user, char *password);
int close_prompt(void);
int draw_faillogon(void);

#endif
