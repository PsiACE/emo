#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/common.h"
#include "core/compiler.h"
#include "core/memory.h"
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
	PREC_FACTOR,	 // * / %
	PREC_INDICES,	 // **
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

typedef struct {
	Token name;
	int depth;
	bool isCaptured;
} Local;

typedef struct {
	uint8_t index;
	bool isLocal;
} Upvalue;

typedef enum { TYPE_FUNCTION, TYPE_SCRIPT } FunctionType;

typedef struct Compiler {
	struct Compiler *enclosing;
	ObjFunction *function;
	FunctionType type;

	Local locals[UINT8_COUNT];
	int localCount;
	Upvalue upvalues[UINT8_COUNT];
	int scopeDepth;
} Compiler;

Parser parser;

Compiler *current = NULL;

static Chunk *current_chunk()
{
	return &current->function->chunk;
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

static void emit_loop(int loopStart)
{
	emit_byte(OP_LOOP);

	int offset = current_chunk()->count - loopStart + 2;
	if (offset > UINT16_MAX)
		error("Loop body too large.");

	emit_byte((offset >> 8) & 0xff);
	emit_byte(offset & 0xff);
}

static int emit_jump(uint8_t instruction)
{
	emit_byte(instruction);
	emit_byte(0xff);
	emit_byte(0xff);
	return current_chunk()->count - 2;
}

static void emit_return()
{
	emit_byte(OP_META);
	emit_byte(OP_RETURN);
}

static uint8_t make_constant(Value value)
{
	int constant = add_constant(current_chunk(), value);
	if (constant > UINT8_MAX) {
		error("Too many constants in one chunk.");
		return 0;
	}

	return (uint8_t)constant;
}

// static void emit_constant(Value value)
// {
// 	emit_bytes(OP_CONSTANT, make_constant(value));
// }

static void emit_constant(Value value)
{
	write_constant(current_chunk(), value, parser.previous.line);
}

static void patch_jump(int offset)
{
	// -2 to adjust for the bytecode for the jump offset itself.
	int jump = current_chunk()->count - offset - 2;

	if (jump > UINT16_MAX) {
		error("Too much code to jump over.");
	}

	current_chunk()->code[offset] = (jump >> 8) & 0xff;
	current_chunk()->code[offset + 1] = jump & 0xff;
}

static void init_compiler(Compiler *compiler, FunctionType type)
{
	compiler->enclosing = current;
	compiler->function = NULL;
	compiler->type = type;
	compiler->localCount = 0;
	compiler->scopeDepth = 0;
	compiler->function = new_function();
	current = compiler;

	if (type != TYPE_SCRIPT) {
		current->function->name = copy_string(parser.previous.start, parser.previous.length);
	}

	Local *local = &current->locals[current->localCount++];
	local->depth = 0;
	local->isCaptured = false;
	local->name.start = "";
	local->name.length = 0;
}

static ObjFunction *end_compiler()
{
	emit_return();
	ObjFunction *current_function = current->function;
#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError) {
		disassemble_chunk(current_chunk(), current_function->name != NULL ? current_function->name->chars : "<script>");
	}
#endif
	current = current->enclosing;
	return current_function;
}

static void begin_scope()
{
	current->scopeDepth++;
}

static void end_scope()
{
	current->scopeDepth--;

	while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
		if (current->locals[current->localCount - 1].isCaptured) {
			emit_byte(OP_CLOSE_UPVALUE);
		} else {
			emit_byte(OP_POP);
		}
		current->localCount--;
	}
}

static void expression();
static void statement();
static void declaration();
static ParseRule *get_rule(TokenType type);
static void parse_precedence(Precedence precedence);

static uint8_t identifier_constant(Token *name)
{
	return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}

static bool identifiers_equal(Token *a, Token *b)
{
	if (a->length != b->length)
		return false;
	return memcmp(a->start, b->start, a->length) == 0;
}

static int resolve_local(Compiler *compiler, Token *name)
{
	for (int i = compiler->localCount - 1; i >= 0; i--) {
		Local *local = &compiler->locals[i];
		if (identifiers_equal(name, &local->name)) {
			if (local->depth == -1) {
				error("Cannot read local variable in its own initializer.");
			}
			return i;
		}
	}

	return -1;
}

static int add_upvalue(Compiler *compiler, uint8_t index, bool isLocal)
{
	int upvalueCount = compiler->function->upvalueCount;

	for (int i = 0; i < upvalueCount; ++i) {
		Upvalue *upvalue = &compiler->upvalues[i];
		if (upvalue->index == index && upvalue->isLocal == isLocal) {
			return i;
		}
	}

	if (upvalueCount == UINT8_COUNT) {
		error("Too many closure variables in function.");
		return 0;
	}

	compiler->upvalues[upvalueCount].isLocal = isLocal;
	compiler->upvalues[upvalueCount].index = index;
	return compiler->function->upvalueCount++;
}

static int resolve_upvalue(Compiler *compiler, Token *name)
{
	if (compiler->enclosing == NULL)
		return -1;

	int local = resolve_local(compiler->enclosing, name);
	if (local != -1) {
		compiler->enclosing->locals[local].isCaptured = true;
		return add_upvalue(compiler, (uint8_t)local, true);
	}

	int upvalue = resolve_upvalue(compiler->enclosing, name);
	if (upvalue != -1) {
		return add_upvalue(compiler, (uint8_t)upvalue, false);
	}

	return -1;
}

static void add_local(Token name)
{
	if (current->localCount == UINT8_COUNT) {
		error("Too many local variables in function.");
		return;
	}

	Local *local = &current->locals[current->localCount++];
	local->name = name;
	local->depth = -1;
	local->isCaptured = false;
}

static void declare_variable()
{
	// Global variables are implicitly declared.
	if (current->scopeDepth == 0)
		return;

	Token *name = &parser.previous;

	for (int i = current->localCount - 1; i >= 0; i--) {
		Local *local = &current->locals[i];
		if (local->depth != -1 && local->depth < current->scopeDepth) {
			break;
		}

		if (identifiers_equal(name, &local->name)) {
			error("Variable with this name already declared in this scope.");
		}
	}
	add_local(*name);
}

static uint8_t parse_variable(const char *errorMessage)
{
	consume(TOKEN_IDENTIFIER, errorMessage);

	declare_variable();
	if (current->scopeDepth > 0)
		return 0;

	return identifier_constant(&parser.previous);
}

static void mark_initialized()
{
	if (current->scopeDepth == 0)
		return;
	current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void define_variable(uint8_t global)
{
	if (current->scopeDepth > 0) {
		mark_initialized();
		return;
	}

	emit_bytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argument_list()
{
	uint8_t argCount = 0;
	if (!check(TOKEN_RIGHT_PAREN)) {
		do {
			expression();

			if (argCount == 255) {
				error("Cannot have more than 255 arguments.");
			}

			argCount++;
		} while (match(TOKEN_COMMA));
	}

	consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
	return argCount;
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
	case TOKEN_PERCENT:
		emit_byte(OP_MODULO);
		break;
	case TOKEN_STAR_STAR:
		emit_byte(OP_POW);
		break;
	default:
		return; // Unreachable.
	}
}

static void call(bool canAssign)
{
	uint8_t argCount = argument_list();
	emit_bytes(OP_CALL, argCount);
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

static void or_(bool canAssign)
{
	int elseJump = emit_jump(OP_JUMP_IF_FALSE);
	int endJump = emit_jump(OP_JUMP);

	patch_jump(elseJump);
	emit_byte(OP_POP);

	parse_precedence(PREC_OR);
	patch_jump(endJump);
}

static void string(bool canAssign)
{
	emit_constant(OBJ_VAL(copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

static void named_variable(Token name, bool canAssign)
{
	uint8_t getOp, setOp;
	int arg = resolve_local(current, &name);
	if (arg != -1) {
		getOp = OP_GET_LOCAL;
		setOp = OP_SET_LOCAL;
	} else if ((arg = resolve_upvalue(current, &name)) != -1) {
		getOp = OP_GET_UPVALUE;
		setOp = OP_SET_UPVALUE;
	} else {
		arg = identifier_constant(&name);
		getOp = OP_GET_GLOBAL;
		setOp = OP_SET_GLOBAL;
	}

	if (canAssign && match(TOKEN_EQUAL)) {
		expression();
		emit_bytes(setOp, (uint8_t)arg);
	} else {
		emit_bytes(getOp, (uint8_t)arg);
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
	{grouping, call, PREC_CALL},	 // TOKEN_LEFT_PAREN
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
	{NULL, binary, PREC_INDICES},	 // TOKEN_STAR_STAR
	{NULL, binary, PREC_EQUALITY},	 // TOKEN_BANG_EQUAL
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
	{NULL, or_, PREC_OR},			 // TOKEN_OR
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

static void block()
{
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
		declaration();
	}

	consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type)
{
	Compiler compiler;
	init_compiler(&compiler, type);
	begin_scope();

	// Compile the parameter list.
	consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
	if (!check(TOKEN_RIGHT_PAREN)) {
		do {
			current->function->arity++;
			if (current->function->arity > 255) {
				error_at_current("Cannot have more than 255 parameters.");
			}

			uint8_t paramConstant = parse_variable("Expect parameter name.");
			define_variable(paramConstant);
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

	// The body.
	consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
	block();

	// Create the function object.
	ObjFunction *function = end_compiler();
	emit_bytes(OP_CLOSURE, make_constant(OBJ_VAL(function)));

	for (int i = 0; i < function->upvalueCount; ++i) {
		emit_byte(compiler.upvalues[i].isLocal ? 1 : 0);
		emit_byte(compiler.upvalues[i].index);
	}
}

static void fn_declaration()
{
	uint8_t global = parse_variable("Expect function name.");
	mark_initialized();
	function(TYPE_FUNCTION);
	define_variable(global);
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

static void for_statement()
{
	begin_scope();

	// 1: Grab the name and slot of the loop variable so we can refer to it later.
	int loopVariable = -1;
	Token loopVariableName;
	loopVariableName.start = NULL;
	// end.

	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
	if (match(TOKEN_LET)) {
		// 1: Grab the name of the loop variable.
		loopVariableName = parser.current;
		// end.
		var_declaration();
		// 1: And get its slot.
		loopVariable = current->localCount - 1;
		// end.
	} else if (match(TOKEN_SEMICOLON)) {
		// No initializer.
	} else {
		expression_statement();
	}

	int loopStart = current_chunk()->count;

	int exitJump = -1;
	if (!match(TOKEN_SEMICOLON)) {
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

		// Jump out of the loop if the condition is false.
		exitJump = emit_jump(OP_JUMP_IF_FALSE);
		emit_byte(OP_POP); // Condition.
	}

	if (!match(TOKEN_RIGHT_PAREN)) {
		int bodyJump = emit_jump(OP_JUMP);

		int incrementStart = current_chunk()->count;
		expression();
		emit_byte(OP_POP);
		consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

		emit_loop(loopStart);
		loopStart = incrementStart;
		patch_jump(bodyJump);
	}

	// 1: If the loop declares a variable...
	int innerVariable = -1;
	if (loopVariable != -1) {
		// 1: Create a scope for the copy...
		begin_scope();
		// 1: Define a new variable initialized with the current value of the loop
		//    variable.
		emit_bytes(OP_GET_LOCAL, (uint8_t)loopVariable);
		add_local(loopVariableName);
		mark_initialized();
		// 1: Keep track of its slot.
		innerVariable = current->localCount - 1;
	}
	// end.

	statement();

	// 3: If the loop declares a variable...
	if (loopVariable != -1) {
		// 3: Store the inner variable back in the loop variable.
		emit_bytes(OP_GET_LOCAL, (uint8_t)innerVariable);
		emit_bytes(OP_SET_LOCAL, (uint8_t)loopVariable);
		emit_byte(OP_POP);

		// 4: Close the temporary scope for the copy of the loop variable.
		end_scope();
	}

	emit_loop(loopStart);

	if (exitJump != -1) {
		patch_jump(exitJump);
		emit_byte(OP_POP); // Condition.
	}

	end_scope();
}

static void if_statement()
{
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	int thenJump = emit_jump(OP_JUMP_IF_FALSE);
	emit_byte(OP_POP);
	statement();

	int elseJump = emit_jump(OP_JUMP);
	patch_jump(thenJump);
	emit_byte(OP_POP);

	if (match(TOKEN_ELSE))
		statement();

	patch_jump(elseJump);
}

static void print_statement()
{
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'print'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emit_byte(OP_PRINT);
}

static void return_statement()
{
	if (current->type == TYPE_SCRIPT) {
		error("Cannot return from top-level code.");
	}

	if (match(TOKEN_SEMICOLON)) {
		emit_return();
	} else {
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
		emit_byte(OP_RETURN);
	}
}

static void while_statement()
{
	int loopStart = current_chunk()->count;

	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	int exitJump = emit_jump(OP_JUMP_IF_FALSE);

	emit_byte(OP_POP);
	statement();

	emit_loop(loopStart);

	patch_jump(exitJump);
	emit_byte(OP_POP);
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
	if (match(TOKEN_FN)) {
		fn_declaration();
	} else if (match(TOKEN_LET)) {
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
	} else if (match(TOKEN_FOR)) {
		for_statement();
	} else if (match(TOKEN_IF)) {
		if_statement();
	} else if (match(TOKEN_RETURN)) {
		return_statement();
	} else if (match(TOKEN_WHILE)) {
		while_statement();
	} else if (match(TOKEN_LEFT_BRACE)) {
		begin_scope();
		block();
		end_scope();
	} else {
		expression_statement();
	}
}

ObjFunction *compile(const char *source)
{
	init_scanner(source);
	Compiler compiler;
	init_compiler(&compiler, TYPE_SCRIPT);
	parser.hadError = false;
	parser.panicMode = false;

	advance();
	while (!match(TOKEN_EOF)) {
		declaration();
	}

	ObjFunction *current_function = end_compiler();
	return parser.hadError ? NULL : current_function;
}

void mark_compiler_roots()
{
	Compiler *compiler = current;
	while (compiler != NULL) {
		mark_object((Obj *)compiler->function);
		compiler = compiler->enclosing;
	}
}
