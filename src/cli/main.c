#include <stdio.h>
#include <stdlib.h>

#include "emo/cli/args.h"
#include "emo/cli/styles.h"

int main(int argc, char *argv[])
{
	struct options options;
	options_parser(argc, argv, &options);

#ifdef DEBUG
	fprintf(stdout, BLUE "Command line options:\n" NO_COLOR);
	fprintf(stdout, BROWN "help: %d\n" NO_COLOR, options.help);
	fprintf(stdout, BROWN "version: %d\n" NO_COLOR, options.version);
	fprintf(stdout, BROWN "use colors: %d\n" NO_COLOR, options.use_colors);
	fprintf(stdout, BROWN "filename: %s\n" NO_COLOR, options.file_name);
#endif

	return EXIT_SUCCESS;
}
