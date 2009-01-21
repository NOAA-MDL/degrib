#ifndef USER_H
#define USER_H

typedef struct {
   int f_filter;
   char *input;
   char *output;
} usrType;

void usrFree(usrType *usr);
int usrParse(usrType **Usr, int argc, char **argv, char *pkgName,
             char *pkgVers, char *pkgDate);

#endif
