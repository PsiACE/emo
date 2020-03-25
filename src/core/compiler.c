#include <stdio.h>
#include <stdlib.h>

#include "core/common.h"
#include "core/compiler.h"
#include "core/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"     
#endif                 

typedef struct {
	Token current;
	Token previous;
	bool hadError;
	bool panicMode;
} Parser;

typedef enum {                  
  PREC_NONE,                    
  PREC_ASSIGNMENT,  // =        
  PREC_OR,          // or       
  PREC_AND,         // and      
  PREC_EQUALITY,    // == !=    
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -      
  PREC_FACTOR,      // * /      
  PREC_UNARY,       // ! -      
  PREC_CALL,        // . ()     
  PREC_PRIMARY                  
} Precedence; 

typedef void (*ParseFn)();

typedef struct {        
  ParseFn prefix;       
  ParseFn infix;        
  Precedence precedence;
} ParseRule; 

Parser parser;

Chunk* compilingChunk;

static Chunk* current_chunk() {                          
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

static void consume(TokenType type, const char* message) {
  if (parser.current.type == type) {                      
    advance();                                            
    return;                                               
  }

  error_at_current(message);                                
}                                                         

static void emit_byte(uint8_t byte) {                     
  write_chunk(current_chunk(), byte, parser.previous.line);
}   

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
  emit_byte(byte1);                                   
  emit_byte(byte2);                                   
} 

static void emit_return() {
  emit_byte(OP_RETURN);    
}    

static uint8_t make_constant(Value value) {          
  int constant = add_constant(current_chunk(), value);
  if (constant > UINT8_MAX) {                       
    error("Too many constants in one chunk.");      
    return 0;                                       
  }

  return (uint8_t)constant;                         
}     

static void emit_constant(Value value) {       
  emit_bytes(OP_CONSTANT, make_constant(value));
}    

static void end_compiler() {
  emit_return();     
#ifdef DEBUG_PRINT_CODE                      
  if (!parser.hadError) {                    
    disassemble_chunk(current_chunk(), "code");
  }                                          
#endif       
}   

static void expression();                          
static ParseRule* get_rule(TokenType type);         
static void parse_precedence(Precedence precedence);

static void binary() {                                     
  // Remember the operator.                                
  TokenType operatorType = parser.previous.type;

  // Compile the right operand.                            
  ParseRule* rule = get_rule(operatorType);                 
  parse_precedence((Precedence)(rule->precedence + 1));     

  // Emit the operator instruction.                        
  switch (operatorType) {                                  
    case TOKEN_PLUS:          emit_byte(OP_ADD); break;     
    case TOKEN_MINUS:         emit_byte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emit_byte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emit_byte(OP_DIVIDE); break;  
    default:                                               
      return; // Unreachable.                              
  }                                                        
} 

static void grouping() {                                     
  expression();                                              
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
} 

static void number() {                               
  double value = strtod(parser.previous.start, NULL);
  emit_constant(value);                               
}    

static void unary() {                            
  TokenType operatorType = parser.previous.type;

  // Compile the operand.                        
   parse_precedence(PREC_UNARY);                             

  // Emit the operator instruction.              
  switch (operatorType) {                        
    case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
    default:                                     
      return; // Unreachable.                    
  }                                              
}

ParseRule rules[] = {                                              
  { grouping, NULL,    PREC_NONE },       // TOKEN_LEFT_PAREN      
  { NULL,     NULL,    PREC_NONE },       // TOKEN_RIGHT_PAREN     
  { NULL,     NULL,    PREC_NONE },       // TOKEN_LEFT_BRACE
  { NULL,     NULL,    PREC_NONE },       // TOKEN_RIGHT_BRACE     
  { NULL,     NULL,    PREC_NONE },       // TOKEN_COMMA           
  { NULL,     NULL,    PREC_NONE },       // TOKEN_DOT             
  { unary,    binary,  PREC_TERM },       // TOKEN_MINUS           
  { NULL,     binary,  PREC_TERM },       // TOKEN_PLUS            
  { NULL,     NULL,    PREC_NONE },       // TOKEN_SEMICOLON       
  { NULL,     binary,  PREC_FACTOR },     // TOKEN_SLASH           
  { NULL,     binary,  PREC_FACTOR },     // TOKEN_STAR                       
  { NULL,     NULL,    PREC_NONE },       // TOKEN_BANG_EQUAL      
  { NULL,     NULL,    PREC_NONE },       // TOKEN_EQUAL           
  { NULL,     NULL,    PREC_NONE },       // TOKEN_EQUAL_EQUAL     
  { NULL,     NULL,    PREC_NONE },       // TOKEN_GREATER         
  { NULL,     NULL,    PREC_NONE },       // TOKEN_GREATER_EQUAL   
  { NULL,     NULL,    PREC_NONE },       // TOKEN_LESS            
  { NULL,     NULL,    PREC_NONE },       // TOKEN_LESS_EQUAL      
  { NULL,     NULL,    PREC_NONE },       // TOKEN_IDENTIFIER      
  { NULL,     NULL,    PREC_NONE },       // TOKEN_STRING          
  { number,   NULL,    PREC_NONE },       // TOKEN_NUMBER          
  { NULL,     NULL,    PREC_NONE },       // TOKEN_AND                      
  { NULL,     NULL,    PREC_NONE },       // TOKEN_ELSE            
  { NULL,     NULL,    PREC_NONE },       // TOKEN_FALSE           
  { NULL,     NULL,    PREC_NONE },       // TOKEN_FOR             
  { NULL,     NULL,    PREC_NONE },       // TOKEN_FN             
  { NULL,     NULL,    PREC_NONE },       // TOKEN_IF  
  { NULL,     NULL,    PREC_NONE },       // TOKEN_LET                         
  { NULL,     NULL,    PREC_NONE },       // TOKEN_OR
  { NULL,     NULL,    PREC_NONE },       // TOKEN_NOT              
  { NULL,     NULL,    PREC_NONE },       // TOKEN_PRINT           
  { NULL,     NULL,    PREC_NONE },       // TOKEN_RETURN                     
  { NULL,     NULL,    PREC_NONE },       // TOKEN_TRUE                      
  { NULL,     NULL,    PREC_NONE },       // TOKEN_WHILE           
  { NULL,     NULL,    PREC_NONE },       // TOKEN_ERROR           
  { NULL,     NULL,    PREC_NONE },       // TOKEN_EOF             
};                                                                 

static void parse_precedence(Precedence precedence) {
  advance();                                                 
  ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
  if (prefix_rule == NULL) {                                  
    error("Expect expression.");                             
    return;                                                  
  }

  prefix_rule();      

  
  while (precedence <= get_rule(parser.current.type)->precedence) {
    advance();                                                    
    ParseFn infix_rule = get_rule(parser.previous.type)->infix;     
    infix_rule();                                                  
  }                          
}                          

static ParseRule* get_rule(TokenType type) {
  return &rules[type];                     
}                                          

static void expression() {
  parse_precedence(PREC_ASSIGNMENT);    
}  

bool compile(const char *source, Chunk *chunk)
{
	init_scanner(source);
	parser.hadError = false;
	parser.panicMode = false;
	advance();
	expression();
	consume(TOKEN_EOF, "Expect end of expression.");
    end_compiler();
    return !parser.hadError; 
}
