#define put_out(value)              \
  for (i=0,k=0;t!=EOF && k<3;)      \
    {                               \
      attrset(COLOR_PAIR(c-'0'));   \
      if (i<strlen(value))          \
        addch(value[i++]);          \
      else                          \
        addch(' ');                 \
      if (t=='%')                   \
        k++;                        \
      else                          \
        {                           \
          t=fgetc(textfile);        \
          c=fgetc(colorfile);       \
        }                           \
    }                               \
  isopen=FALSE;                     \
  c=fgetc(colorfile);               \
  t=fgetc(textfile);                \
  break;                                  
