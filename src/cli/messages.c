#include <stdio.h>
#include <stdlib.h>

#include "cli/messages.h"
#include "cli/styles.h"

void help()
{
	overview();
	usage();
	options();
}

void repl_helper()
{
	version();
	copyright();
	hit();
}

void overview()
{
	printf("The \"%s\" Repl and Compiler\n\n", __PROGRAM_NAME__);
}

void copyright()
{
	printf("Copyright (C) %s %s\n", __PROGRAM_COPYRIGHT__, __PROGRAM_AUTHOR__);
}

void usage()
{
	printf("USAGE: \n");
	printf("    %s [OPTIONS] [FILENAME]\n\n", __PROGRAM_NAME__);
}

void options()
{
	printf("OPTIONS: \n");
	printf("    -v, --version           Prints %s version\n", __PROGRAM_NAME__);
	printf("    -h, --help              Prints this help message\n");
	printf("        --no-color          Does not use colors/styles for printing\n\n");
}

void version()
{
	printf("\"%s\" version: %s\n", __PROGRAM_NAME__, __PROGRAM_VERSION__);
}

void hit()
{
	printf("Type 'exit()' to exit\n");
}
