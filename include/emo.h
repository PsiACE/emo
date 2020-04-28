#ifndef emo_h
#define emo_h

#include "emo-config.h"

#define PROGRAM_NAME "emo"
#define PROGRAM_VERSION PACKAGE_VERSION
#define PROGRAM_COPYRIGHT "2020"
#define PROGRAM_AUTHOR "Chojan Shang <psiace@outlook.com>"

#ifdef DEBUG
#define DEBUG_OPTIONS
#define DEBUG_TRACE_EXECUTION
#define DEBUG_PRINT_CODE
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC
#endif

#ifndef USR_EMOHISTORY_FILE
#ifdef _WIN32
#define USR_EMOHISTORY_FILE "\\_emo_history"
#else
#define USR_EMOHISTORY_FILE "/.emo_history"
#endif
#endif

#endif
