#ifndef __HAVE_MACROS_H__
#define __HAVE_MACROS_H__


#define put_out(value)              \
  for (i=0,k=0;c!=EOF && k<3;)      \
    {                               \
      if (i<strlen(value))          \
        putchar(value[i++]);        \
      else                          \
        putchar(' ');               \
      if (c=='%')                   \
        k++;                        \
      else                          \
        c=(*(ansiscreen++));        \
    }                               \
  isopen=FALSE;                     \
  c=(*(ansiscreen++));              \
  break;    


#endif
