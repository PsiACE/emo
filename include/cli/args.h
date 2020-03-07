#ifndef emo_utils_args_h
#define emo_utils_args_h

#include <getopt.h>
#include <stdbool.h>

#define FILE_NAME_SIZE 512

struct options {
	bool help;
	bool version;
	bool use_colors;
	char file_name[FILE_NAME_SIZE];
};

typedef struct options Options;

void options_parser(int argc, char *argv[], Options *options);

#endif
