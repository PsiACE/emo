#include <stdio.h>
#include <stdlib.h>

#include "cli/args.h"
#include "cli/styles.h"

#include "core/chunk.h"
#include "core/common.h"
#include "core/debug.h"

int main(int argc, char *argv[])
{
	struct options options;
	options_parser(argc, argv, &options);

	Chunk chunk;
	init_chunk(&chunk);
	int constant = add_constant(&chunk, 1.2);
	write_chunk(&chunk, OP_CONSTANT, 123);
	write_chunk(&chunk, constant, 123);

	write_chunk(&chunk, OP_RETURN, 123);
	write_chunk(&chunk, OP_RETURN, 125);
	disassemble_chunk(&chunk, "test chunk");
	free_chunk(&chunk);

#ifdef DEBUG
	fprintf(stdout, BLUE "Command line options:\n" NO_COLOR);
	fprintf(stdout, BROWN "help: %d\n" NO_COLOR, options.help);
	fprintf(stdout, BROWN "version: %d\n" NO_COLOR, options.version);
	fprintf(stdout, BROWN "use colors: %d\n" NO_COLOR, options.use_colors);
	fprintf(stdout, BROWN "filename: %s\n" NO_COLOR, options.file_name);
#endif

	return EXIT_SUCCESS;
}
