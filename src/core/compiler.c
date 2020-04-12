#include <stdio.h>
#include <stdlib.h>

#include "core/common.h"
#include "core/compiler.h"
#include "core/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "core/debug.h"
#endif

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;

typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT, // =
	PREC_OR,		 // or
	PREC_AND,		 // and
	PREC_EQUALITY,	 // == !=
	PREC_COMPARISON, // < > <= >=
	PREC_TERM,		 // + -
	PREC_FACTOR,	 // * /
	PREC_UNARY,		 // ! -
	PREC_CALL,		 // . ()
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

Parser parser;

Chunk *compilingChunk;

static Chunk *current_chunk()
{
	return compilingChunk;
}

static void error_at(Token *token, const char *message)
{
	if (parser.panicMode)
		return;
	parser.panicMode = true;

	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == TOKEN_EOF) {
		fprintf(stderr, " at end");
	} else if (token->type == TOKEN_ERROR) {
		// Nothing.
	} else {
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	}

	fprintf(stderr, ": %s\n", message);
	parser.hadError = true;
}

static void error(const char *message)
{
	error_at(&parser.previous, message);
}

static void error_at_current(const char *message)
{
	error_at(&parser.current, message);
}

static void advance()
{
	parser.previous = parser.current;

	for (;;) {
		parser.current = scan_token();
		if (parser.current.type != TOKEN_ERROR)
			break;

		error_at_current(parser.current.start);
	}
}

static void consume(TokenType type, const char *message)
{
	if (parser.current.type == type) {
		advance();
		return;
	}

	error_at_current(message);
}

static bool check(TokenType type)
{
	return parser.current.type == type;
}

static bool match(TokenType type)
{
	if (!check(type))
		return false;
	advance();
	return true;
}

static void emit_byte(uint8_t byte)
{
	write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2)
{
	emit_byte(byte1);
	emit_byte(byte2);
}

static void emit_return()
{
	emit_byte(OP_RETURN);
}

// static uint8_t make_constant(Value value)
// {
// 	int constant = add_constant(current_chunk(), value);
// 	if (constant > UINT8_MAX) {
// 		error("Too many constants in one chunk.");
// 		return 0;
// 	}

// 	return (uint8_t)constant;
// }

// static void emit_constant(Value value)
// {
// 	emit_bytes(OP_CONSTANT, make_constant(value));
// }

static void emit_constant(Value value)
{
	write_constant(current_chunk(), value, parser.previous.line);
}

static void end_compiler()
{
	emit_return();
#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError) {
		disassemble_chunk(current_chunk(), "code");
	}
#endif
}

static void expression();
static void statement();
static void declaration();
static ParseRule *get_rule(TokenType type);
static void parse_precedence(Precedence precedence);

static uint8_t identifier_constant(Token *name)
{
	return add_constant(compilingChunk, OBJ_VAL(copy_string(name->start, name->length)));
}

static uint8_t parse_variable(const char *errorMessage)
{
	consume(TOKEN_IDENTIFIER, errorMessage);
	return identifier_constant(&parser.previous);
}

static void define_variable(uint8_t global)
{
	emit_bytes(OP_DEFINE_GLOBAL, global);
}

static void binary(bool canAssign)
{
	// Remember the operator.
	TokenType operatorType = parser.previous.type;

	// Compile the right operand.
	ParseRule *rule = get_rule(operatorType);
	parse_precedence((Precedence)(rule->precedence + 1));

	// Emit the operator instruction.
	switch (operatorType) {
	case TOKEN_BANG_EQUAL:
		emit_bytes(OP_EQUAL, OP_NOT);
		break;
	case TOKEN_EQUAL_EQUAL:
		emit_byte(OP_EQUAL);
		break;
	case TOKEN_GREATER:
		emit_byte(OP_GREATER);
		break;
	case TOKEN_GREATER_EQUAL:
		emit_bytes(OP_LESS, OP_NOT);
		break;
	case TOKEN_LESS:
		emit_byte(OP_LESS);
		break;
	case TOKEN_LESS_EQUAL:
		emit_bytes(OP_GREATER, OP_NOT);
		break;
	case TOKEN_PLUS:
		emit_byte(OP_ADD);
		break;
	case TOKEN_MINUS:
		emit_bytes(OP_NEGATE, OP_ADD);
		break;
	case TOKEN_STAR:
		emit_byte(OP_MULTIPLY);
		break;
	case TOKEN_SLASH:
		emit_byte(OP_DIVIDE);
		break;
	default:
		return; // Unreachable.
	}
}

static void literal(bool canAssign)
{
	switch (parser.previous.type) {
	case TOKEN_FALSE:
		emit_byte(OP_FALSE);
		break;
	case TOKEN_TRUE:
		emit_byte(OP_TRUE);
		break;
	default:
		return; // Unreachable.
	}
}

static void grouping(bool canAssign)
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign)
{
	double value = strtod(parser.previous.start, NULL);
	emit_constant(NUMBER_VAL(value));
}

static void string(bool canAssign)
{
	emit_constant(OBJ_VAL(copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

static void named_variable(Token name, bool canAssign)
{
	uint8_t arg = identifier_constant(&name);

	if (canAssign && match(TOKEN_EQUAL)) {
		expression();
		emit_bytes(OP_SET_GLOBAL, arg);
	} else {
		emit_bytes(OP_GET_GLOBAL, arg);
	}
}

static void variable(bool canAssign)
{
	named_variable(parser.previous, canAssign);
}

static void unary(bool canAssign)
{
	TokenType operatorType = parser.previous.type;

	// Compile the operand.
	parse_precedence(PREC_UNARY);

	// Emit the operator instruction.
	switch (operatorType) {
	case TOKEN_NOT:
		emit_byte(OP_NOT);
		break;
	case TOKEN_MINUS:
		emit_byte(OP_NEGATE);
		break;
	default:
		return; // Unreachable.
	}
}

ParseRule rules[] = {
	{grouping, NULL, PREC_NONE},	 // TOKEN_LEFT_PAREN
	{NULL, NULL, PREC_NONE},		 // TOKEN_RIGHT_PAREN
	{NULL, NULL, PREC_NONE},		 // TOKEN_LEFT_BRACE
	{NULL, NULL, PREC_NONE},		 // TOKEN_RIGHT_BRACE
	{NULL, NULL, PREC_NONE},		 // TOKEN_COMMA
	{NULL, NULL, PREC_NONE},		 // TOKEN_DOT
	{unary, binary, PREC_TERM},		 // TOKEN_MINUS
	{NULL, binary, PREC_TERM},		 // TOKEN_PLUS
	{NULL, NULL, PREC_NONE},		 // TOKEN_SEMICOLON
	{NULL, binary, PREC_FACTOR},	 // TOKEN_SLASH
	{NULL, binary, PREC_FACTOR},	 // TOKEN_STAR
	{NULL, binary, PREC_EQUALITY},	 // TOKEN_BANG_EQUAL
	{NULL, NULL, PREC_NONE},		 // TOKEN_EQUAL
	{NULL, binary, PREC_EQUALITY},	 // TOKEN_EQUAL_EQUAL
	{NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER
	{NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
	{NULL, binary, PREC_COMPARISON}, // TOKEN_LESS
	{NULL, binary, PREC_COMPARISON}, // TOKEN_LESS_EQUAL
	{variable, NULL, PREC_NONE},	 // TOKEN_IDENTIFIER
	{string, NULL, PREC_NONE},		 // TOKEN_STRING
	{number, NULL, PREC_NONE},		 // TOKEN_NUMBER
	{NULL, NULL, PREC_NONE},		 // TOKEN_AND
	{NULL, NULL, PREC_NONE},		 // TOKEN_ELSE
	{literal, NULL, PREC_NONE},		 // TOKEN_FALSE
	{NULL, NULL, PREC_NONE},		 // TOKEN_FOR
	{NULL, NULL, PREC_NONE},		 // TOKEN_FN
	{NULL, NULL, PREC_NONE},		 // TOKEN_IF
	{NULL, NULL, PREC_NONE},		 // TOKEN_LET
	{NULL, NULL, PREC_NONE},		 // TOKEN_OR
	{unary, NULL, PREC_NONE},		 // TOKEN_NOT
	{NULL, NULL, PREC_NONE},		 // TOKEN_PRINT
	{NULL, NULL, PREC_NONE},		 // TOKEN_RETURN
	{literal, NULL, PREC_NONE},		 // TOKEN_TRUE
	{NULL, NULL, PREC_NONE},		 // TOKEN_WHILE
	{NULL, NULL, PREC_NONE},		 // TOKEN_ERROR
	{NULL, NULL, PREC_NONE},		 // TOKEN_EOF
};

static void parse_precedence(Precedence precedence)
{
	advance();
	ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
	if (prefix_rule == NULL) {
		error("Expect expression.");
		return;
	}

	bool canAssign = precedence <= PREC_ASSIGNMENT;
	prefix_rule(canAssign);

	while (precedence <= get_rule(parser.current.type)->precedence) {
		advance();
		ParseFn infix_rule = get_rule(parser.previous.type)->infix;
		infix_rule(canAssign);
	}

	if (canAssign && match(TOKEN_EQUAL)) {
		error("Invalid assignment target.");
	}
}

static ParseRule *get_rule(TokenType type)
{
	return &rules[type];
}

static void expression()
{
	parse_precedence(PREC_ASSIGNMENT);
}

static void var_declaration()
{
	uint8_t global = parse_variable("Expect variable name.");

	if (match(TOKEN_EQUAL)) {
		expression();
	} else {
		emit_byte(OP_META);
	}
	consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

	define_variable(global);
}

static void expression_statement()
{
	expression();
	emit_byte(OP_POP);
	consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
}

static void print_statement()
{
	consume(TOKEN_LEFT_PAREN, "Expect '(' before expression.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emit_byte(OP_PRINT);
}

static void synchronize()
{
	parser.panicMode = false;

	while (parser.current.type != TOKEN_EOF) {
		if (parser.previous.type == TOKEN_SEMICOLON)
			return;

		switch (parser.current.type) {
		case TOKEN_FN:
		case TOKEN_LET:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_PRINT:
		case TOKEN_RETURN:
			return;

		default:
			// Do nothing.
			;
		}

		advance();
	}
}

static void declaration()
{
	if (match(TOKEN_LET)) {
		var_declaration();
	} else {
		statement();
	}

	if (parser.panicMode)
		synchronize();
}

static void statement()
{
	if (match(TOKEN_PRINT)) {
		print_statement();
	} else {
		expression_statement();
	}
}

bool compile(const char *source, Chunk *chunk)
{
	init_scanner(source);
	compilingChunk = chunk;
	parser.hadError = false;
	parser.panicMode = false;
	advance();
	while (!match(TOKEN_EOF)) {
		declaration();
	}
	end_compiler();
	return !parser.hadError;
}
