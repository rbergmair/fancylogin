#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <readline/readline.h>

#include "explanations.h"
#include "../global/flt.h"

int
main(void)
{
  int i;
  int fd;
  struct stat statf;
  char *wsf;
  char *fsf;
  char *bck;
  fancylogin_theme buf;

  printf("You can enter '?' at any time to get help\n\n");

  do
    {

      for (;;)
        {
          wsf=readline("welcome-screenfile: ");
          if (strcmp(wsf,"?")==0)
            {
              printf(WELCOME_SCREENFILE_EXPLAIN);
              printf("\n");
              continue;
            }
          if (open(wsf,O_RDONLY)!=-1)
            break;
          printf ("couldn't open '%s' for reading!\n\n", wsf);
        }

      for (;;)
        {
          fsf=readline("failed-screenfile: ");
          if (strcmp(fsf,"?")==0)
            {
              printf(FAILED_SCREENFILE_EXPLAIN);
              printf("\n");
              continue;
            }
          if (open(fsf,O_RDONLY)!=-1)
            break;
          printf ("couldn't open '%s' for reading!\n\n", fsf);
        }


#define ASK_FOR(prompt,back,help) for (;;)	\
    {						\
      bck=readline(prompt);			\
      if (strcmp(bck,"?")==0)			\
        {					\
          printf(NOHELP);			\
          printf("\n\n");			\
          continue;				\
        }					\
      break;					\
    }						\
  back=atoi(bck);

      ASK_FOR("Username-Mask X-Position: ", buf.unpx, NOHELP)
      ASK_FOR("Username-Mask Y-Position: ", buf.unpy, NOHELP)
      ASK_FOR("Username-Mask length: ", buf.unle, NOHELP)

      ASK_FOR("Password-Mask X-Position: ", buf.pwpx, NOHELP)
      ASK_FOR("Password-Mask Y-Position: ", buf.pwpy, NOHELP)
      ASK_FOR("Password-Mask length: ", buf.pwle, NOHELP)


#define ASK_ATTR(prmpt,idx,back) for (;;)       \
    {                                           \
      char prompt[128];                         \
      sprintf(prompt, "%s[%d]: ", prmpt, idx);  \
      bck=readline(prompt);                     \
      if (strcmp(bck,"?")==0)                   \
        {                                       \
          printf(ATTRIBUTES);                   \
          printf("\n\n");                       \
          continue;                             \
        }                                       \
      break;                                    \
    }                                           \
  back=atoi(bck);


      for (i=0;i<=9;i++)
        buf.unattr[i]=0;

      for (i=0;i<=9;i++)
        {
          ASK_ATTR("Username-Mask attribute", i, buf.unattr[9-i])
          if (buf.unattr[9-i]==0)
            break;
        }


      for (i=0;i<=9;i++)
        buf.pwattr[i]=0;

      for (i=0;i<=9;i++)
        {
          ASK_ATTR("Password-Mask attribute", i, buf.pwattr[9-i])
          if (buf.pwattr[9-i]==0)
            break;
        }

      strncpy(buf.author, readline("author: "), sizeof buf.author);

      printf("\n--\n\
author:                     %s\n\
welcome-screenfile:         %s\n\
failed-screenfile:          %s\n\
username-Mask x-coordinate: %d\n\
username-Mask y-coordinate: %d\n\
username-Mask length:       %d\n\
username-Mask attributes:   %c[%d;%d;%d;%d;%d;%d;%d;%d;%d;%dm[sample]%c[%dm\n\
password-Mask x-coordinate: %d\n\
password-Mask y-coordinate: %d\n\
password-Mask length:       %d\n\
password-Mask attributes:   %c[%d;%d;%d;%d;%d;%d;%d;%d;%d;%dm[sample]%c[%dm\n\
--\n",
      buf.author,
      wsf,
      fsf,
      buf.unpx,
      buf.unpy,
      buf.unle,
      27,
      buf.unattr[0], buf.unattr[1], buf.unattr[2], buf.unattr[3], buf.unattr[4],
      buf.unattr[5], buf.unattr[6], buf.unattr[7], buf.unattr[8], buf.unattr[9],
      27,0,
      buf.pwpx,
      buf.pwpy,
      buf.pwle,
      27,
      buf.pwattr[0], buf.pwattr[1], buf.pwattr[2], buf.pwattr[3], buf.pwattr[4],
      buf.pwattr[5], buf.pwattr[6], buf.pwattr[7], buf.pwattr[8], buf.pwattr[9],
      27,0);

      do
        {
          bck=readline("everything okay (yes/no)? ");
        }
      while (!(((strcmp(bck, "yes"))==0) ||
               ((strcmp(bck, "no"))==0)));

    }
  while (strcmp(bck, "no")==0);

  fd=open(wsf, O_RDONLY);
  read(fd, &buf.ws, sizeof buf.ws);
  close(fd);
  stat(wsf, &statf);
  buf.ws[statf.st_size]=EOF;

  fd=open(fsf, O_RDONLY);
  read(fd, &buf.fs, sizeof buf.ws);
  close(fd);
  stat(fsf, &statf);
  buf.fs[statf.st_size]=EOF;

  fd=open("out.flt", O_WRONLY | O_CREAT);
  write (fd, &buf, sizeof buf);
  close(fd);

  return 0;
}
