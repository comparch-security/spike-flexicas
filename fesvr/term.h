#ifndef _TERM_H
#define _TERM_H

#include<cstdio>

class canonical_terminal_t
{
 public:
  static int read();
  static void write(char);
  static void log(FILE *log_file);
};

#endif
