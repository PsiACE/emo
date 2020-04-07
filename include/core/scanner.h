#ifndef emo_core_scanner_h
#define emo_core_scanner_h

typedef enum {
	// Single-character tokens.
	TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
	TOKEN_LEFT_BRACE,
	TOKEN_RIGHT_BRACE,
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_MINUS,
	TOKEN_PLUS,
	TOKEN_SEMICOLON,
	TOKEN_SLASH,
	TOKEN_STAR,

	// One or two character tokens.
	TOKEN_BANG_EQUAL,
	TOKEN_EQUAL,
	TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER,
	TOKEN_GREATER_EQUAL,
	TOKEN_LESS,
	TOKEN_LESS_EQUAL,

	// Literals.
	TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_NUMBER,

	// Keywords.
	TOKEN_AND,
	TOKEN_ELSE,
	TOKEN_FALSE,
	TOKEN_FOR,
	TOKEN_FN,
	TOKEN_IF,
	TOKEN_LET,
	TOKEN_OR,
	TOKEN_NOT,
	TOKEN_PRINT,
	TOKEN_RETURN,
	TOKEN_TRUE,
	TOKEN_WHILE,

	TOKEN_ERROR,
	TOKEN_EOF
} TokenType;

typedef struct {
	TokenType type;
	const char *start;
	int length;
	int line;
} Token;

typedef struct {
	const char *start;
	const char *current;
	int line;
} Scanner;

void init_scanner(const char *source);

Token scan_token();

#endif
