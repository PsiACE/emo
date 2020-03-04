#ifndef emo_utils_styles_h
#define emo_utils_styles_h

#define NO_COLOR "\x1b[0m"
#define BOLD "\x1b[1m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define BROWN "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define GRAY "\x1b[37m"

#define FILENAME_STYLE NO_COLOR
#define LINE_STYLE NO_COLOR
#define LINE_NUM_STYLE NO_COLOR BLUE
#define COL_NUM_STYLE NO_COLOR CYAN
#define MESSAGE_STYLE NO_COLOR BOLD
#define INFO_STYLE NO_COLOR GREEN BOLD
#define ERROR_STYLE NO_COLOR RED BOLD

#endif
