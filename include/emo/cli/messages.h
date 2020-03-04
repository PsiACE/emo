#ifndef emo_utils_messages_h
#define emo_utils_messages_h

#include "emo-config.h"
#include "emo/include/emo.h"

#define __PROGRAM_NAME__ PROGRAM_NAME
#define __PROGRAM_VERSION__ PACKAGE_VERSION
#define __PROGRAM_COPYRIGHT__ PROGRAM_COPYRIGHT
#define __PROGRAM_AUTHOR__ PROGRAM_AUTHOR

void help();
void usage();
void version();
void overview();
void options();
void copyright();

#endif
