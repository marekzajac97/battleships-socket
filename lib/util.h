#ifndef _UTILS_H_
#define _UTILS_H_

#define PRINT_RED(string)       printf("\x1b[31m%s\x1b[0m", string);
#define PRINT_GREEN(string)     printf("\x1b[32m%s\x1b[0m", string);
#define PRINT_YELLOW(string)    printf("\x1b[33m%s\x1b[0m", string);
#define PRINT_BLUE(string)      printf("\x1b[34m%s\x1b[0m", string);
#define PRINT_MAGENTA(string)   printf("\x1b[35m%s\x1b[0m", string);
#define PRINT_CYAN(string)      printf("\x1b[36m%s\x1b[0m", string);


void welcome();

#endif