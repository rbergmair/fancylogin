int root_allowed (void);
struct sections *process_section (FILE *usertty);
int file_pos_str(FILE *input, char *pattern);
int time_ok (char *param);
void dispose_section (struct sections *sect);
char *get_time (char *complete);
char *get_orig (char *orig);
int orig_ok (char *givenorig, char *remotehost);
int user_allowed (struct passwd *user, char *rmthost);
