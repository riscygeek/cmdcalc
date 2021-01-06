#ifndef FILE_PARSER_H
#define FILE_PARSER_H
#include "token.h"

enum ExpressionType {
	EXPR_INTEGER,
	EXPR_FLOAT,
	EXPR_NAME,
	EXPR_PAREN,
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_FCALL,
	EXPR_ASSIGN,
	EXPR_STRING,
	EXPR_COMMA,
	EXPR_CONDITIONAL,
	EXPR_AT,
	EXPR_ARRAY,
};

typedef struct Expression {
	enum ExpressionType type;
	tokenpos_t pos;
	union {
		tkuint_t uVal;
		tkint_t  iVal;
		tkfloat_t fVal;
		const char* str;
		struct Expression* expr;
		struct Expression** comma;
		struct {
			Token op;
			struct Expression* expr;
		} unary;
		struct {
			struct Expression* left;
			Token op;
			struct Expression* right;
		} binary;
		struct {
			const char* name;
			struct Expression** args;
		} fcall;
		struct {
			const char* name;
			struct Expression* expr;
		} assign;
		struct {
			struct Expression* cond;
			struct Expression* true_case;
			struct Expression* false_case;
		} conditional;
		struct {
			struct Expression* base;
			struct Expression* index;
		} at;
	};
} Expression;

Expression* parse_expr(void);

void print_expr(const Expression*, FILE*);

void free_expr(Expression*);

#endif /* FILE_PARSER_H */
