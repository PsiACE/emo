#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli/args.h"
#include "cli/messages.h"
#include "cli/styles.h"

#include "core/chunk.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/vm.h"

static char buffer[2048];

static char *read_line(char *prompt)
{
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char *cpy = malloc(strlen(buffer) + 1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy) - 1] = '\0';
	return cpy;
}

static void run_repl()
{
	repl_helper();
	char *line = NULL;
	for (line = read_line("emo > "); line != NULL && strcmp(line, "exit()") != 0; line = read_line("emo > ")) {
		// printf("> ");

		// if (!fgets(line, sizeof(line), stdin)) {
		// 	printf("\n");
		// 	break;
		// }
		interpret(line);
		free(line);
	}
}

static char *read_file(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char *buffer = (char *)malloc(fileSize + 1);
	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}

	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;
}

static void run_file(const char *path)
{
	char *source = read_file(path);
	InterpretResult result = interpret(source);
	free(source);

	if (result == INTERPRET_COMPILE_ERROR)
		exit(65);
	if (result == INTERPRET_RUNTIME_ERROR)
		exit(70);
}

int main(int argc, char *argv[])
{
	struct options options;
	options_parser(argc, argv, &options);

#ifdef DEBUG_OPTIONS
	fprintf(stdout, BLUE "Command line options:\n" NO_COLOR);
	fprintf(stdout, BROWN "help: %d\n" NO_COLOR, options.help);
	fprintf(stdout, BROWN "version: %d\n" NO_COLOR, options.version);
	fprintf(stdout, BROWN "use colors: %d\n" NO_COLOR, options.use_colors);
	fprintf(stdout, BROWN "filename: %s\n" NO_COLOR, options.file_name);
#endif

	init_vm();

	if (argc == 1) {
		run_repl();
	} else if (argc == 2) {
		run_file(options.file_name);
	}

	// Chunk chunk;
	// init_chunk(&chunk);

	// int constant = add_constant(&chunk, 1.2);

	// write_chunk(&chunk, OP_CONSTANT, 123);
	// write_chunk(&chunk, constant, 123);

	// for (int i = 0; i < 500; i++) {
	// 	write_constant(&chunk, i, 123);
	// }

	// write_chunk(&chunk, OP_ADD, 123); // 498 + 499 = 997

	// write_chunk(&chunk, OP_DIVIDE, 123); // 497 / 997 = 0.498495

	// write_chunk(&chunk, OP_NEGATE, 123);
	// write_chunk(&chunk, OP_RETURN, 123);

	// disassemble_chunk(&chunk, "test chunk");

	// interpret(&chunk);

	free_vm();

	// free_chunk(&chunk);

	return EXIT_SUCCESS;
}
