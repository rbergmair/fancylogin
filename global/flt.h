#ifndef __HAVE_FLT_H__
#define __HAVE_FLT_H__


typedef struct s_fancylogin_theme
  {
    char ws[10*1024];  /* Welcome-Screen ANSI-file */
    char fs[10*1024];  /* Fail-Screen ANSI-file */
    char author[128]; /* Author name */

    int unpx;         /* Username-Mask */
    int unpy;
    int unle;
    int unattr[10];

    int pwpx;         /* Password-Mask */
    int pwpy;
    int pwle;
    int pwattr[10];
  }
fancylogin_theme;

#endif
