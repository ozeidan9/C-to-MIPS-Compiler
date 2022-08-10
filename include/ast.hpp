#ifndef ast_hpp
#define ast_hpp

#include <iostream>
#include <string>
#include <fstream>

/* ---------------------NODE TYPES-------------------------- */

typedef enum Node_Type {
	BASIC_NODE,   //  0 - no special usage (for roots only)
	// declarations
	DECLARATIONS, //  1 - declarations
	DECL_NODE,    //  2 - declaration
	CONST_NODE,   //  3 - constant
	// statements
	STATEMENTS,   //  4 - statements
	IF_NODE,      //  5 - if statement
	ELSIF_NODE,   //  6 - else if branch
	FOR_NODE,     //  7 - for statement
	WHILE_NODE,   //  8 - while statement
	ASSIGN_NODE,  //  9 - assigment
	SIMPLE_NODE,  // 10 - continue or break statement
	INCR_NODE,    // 11 - increment statement (non-expression one)
	FUNC_CALL,    // 12 - function call
	CALL_PARAMS,  // 13 - function call parameters
	// expressions
	ARITHM_NODE,  // 14 - arithmetic expression
	BOOL_NODE,    // 15 - boolean expression
	BITWISE_NODE,
	REL_NODE,     // 16 - relational expression
	EQU_NODE,     // 17 - equality expression
	REF_NODE,	  // 18 - identifier in expression
	// functions
	FUNC_DECLS,   // 19 - function declarations
	FUNC_DECL,    // 20 - function declaration
	RET_TYPE,     // 21 - function return type
	DECL_PARAMS,  // 22 - function parameters
	RETURN_NODE,  // 23 - return statement of functions
}Node_Type;


/* --------------------OPERATOR TYPES----------------------- */

typedef enum Arithm_op{
	ADD,  // + operator
	SUB,  // - operator
	MUL,  // * operator
	DIV, // / operator
	INC, // ++ operator
	DEC, // -- operator
	MOD // % operator
}Arithm_op;

typedef enum Bool_op{
	OR,  // || operator
	AND, // && operator
	NOT  // ! operator
}Bool_op;

typedef enum Bitwise_op{
	BITOR,  // | operator
	BITAND, // & operator
	BITXOR, // ^ operator
	BITNOT,  // ~ operator
	LSHIFT,  // <<  operator
	RSHIFT  // >> operator
}Bitwise_op;

typedef enum Rel_op{
	GREATER,        // > operator
	LESS,           // < operator
	GREATER_EQUAL,  // >= operator
	LESS_EQUAL     // <= operator
}Rel_op;

typedef enum Equ_op{
	EQUAL,    // == operator
	NOT_EQUAL // != operator
}Equ_op;

/* -----------------------AST NODES------------------------- */

/* The basic node */
typedef struct AST_Node{
	Node_Type type; // node type
	AST_Node *left;  // left child
	AST_Node *right; // right child
}AST_Node;

/* Static AST Structures */
inline AST_Node* main_decl_tree; /* main function's declarations AST Tree */
inline AST_Node* main_func_tree; /* main function's statements AST Tree */
inline AST_Node* opt_func_tree; /* optional functions AST Tree */

/* Queue of identifiers to revisit */ // ADDED
typedef struct revisit_queue{
	// symbol table entry
	list_t *entry;
	
	// name of identifier
	char *st_name;
	
	// type of revisit
	int revisit_type;
	
	// parameters of function calls
	int **par_types;
	int *num_of_pars;
	int num_of_calls;
	
	// assignment expression nodes
	AST_Node **nodes;   //chnaged to astnode
	int num_of_assigns;

	
	struct revisit_queue *next;
}revisit_queue;


/* Declarations */
typedef struct AST_Node_Decl{
	Node_Type type; // node type
	
	// data type
	int data_type;
	
	// symbol table entries of the variables
	list_t **names;
	int names_count;
}AST_Node_Decl;

typedef struct AST_Node_Const{
	Node_Type type; // node type
	
	// data type
	int const_type;
	
	// constant value
	Value val;
}AST_Node_Const;

typedef struct AST_Node_Equ{
	Node_Type type; // node type
	
	AST_Node *left;  // left child
	AST_Node *right; // right child
	
	// data type of result
	int data_type;
	
	// operator
	Equ_op op;
	
	/* register assignment stuff */
	int g_index;
}AST_Node_Equ;

typedef struct AST_Node_If{
	Node_Type type; // node type
	
	// condition
	AST_Node *condition;
	
	// if branch
	AST_Node *if_branch;
	
	// else if branches
	AST_Node **elsif_branches;
	int elseif_count;
	
	// else branch
	AST_Node *else_branch;
}AST_Node_If;

typedef struct AST_Node_Elsif{
	Node_Type type; // node type
	
	// condition
	AST_Node *condition;
	
	// branch
	AST_Node *elsif_branch;
}AST_Node_Elsif;

typedef struct AST_Node_For{
	Node_Type type; // node type
	
	// initialization
	AST_Node *initialize;
	
	// condition
	AST_Node *condition;
	
	// incrementation
	AST_Node *expressions;
	
	// branch
	AST_Node *for_branch;
	
	// loop counter
	list_t *counter;
}AST_Node_For;


typedef struct AST_Node_Incr{
	Node_Type type; // node type
	
	// identifier
	list_t *entry;
	
	// increment or decrement
	int incr_type; // 0: increment, 1: decrement
	
	// post- or prefix
	int pf_type; // 0: postfix, 1: prefix

}AST_Node_Incr;

typedef struct AST_Node_While{
	Node_Type type; // node type
	
	// condition
	AST_Node *condition;
	
	// branch
	AST_Node *while_branch;
}AST_Node_While;


typedef struct AST_Node_Arithm{
	Node_Type type; // node type
	
	AST_Node *left;  // left child
	AST_Node *right; // right child
	
	// data type of result
	int data_type;
	
	// operator
	Arithm_op op;
	
	/* register assignment stuff */
	int g_index;
}AST_Node_Arithm;


typedef struct AST_Node_Decl_Params{
	Node_Type type; // node type
	
	// parameters
	Param *parameters;
	int num_of_pars;
}AST_Node_Decl_Params;

typedef struct AST_Node_Assign{
	Node_Type type; // node type
	
	// symbol table entry
	list_t *entry;
	
	// reference or not
	int ref; // 0: not reference, 1: reference
	
	// assignment value
	AST_Node *assign_val;
}AST_Node_Assign;

typedef struct AST_Node_Func_Call{
	Node_Type type; // node type
	
	// function identifier
	list_t *entry;
	
	/* register assignment stuff */
	int g_index;
	
	// call parameters
	AST_Node **params;
	int num_of_pars;
}AST_Node_Func_Call;

typedef struct AST_Node_Call_Params{
	Node_Type type; // node type
	
	// call parameters
	AST_Node **params;
	int num_of_pars;
}AST_Node_Call_Params;

typedef struct AST_Node_Simple{
	Node_Type type; // node type
	
	int statement_type;
}AST_Node_Simple;

typedef struct AST_Node_Bool{
	Node_Type type; // node type
	
	AST_Node *left;  // left child
	AST_Node *right; // right child
	
	// data type of result
	int data_type;
	
	// operator
	Bool_op op;
	
	/* register assignment stuff */
	int g_index;
}AST_Node_Bool;

typedef struct AST_Node_Bitwise{
	Node_Type type; // node type
	
	AST_Node *left;  // left child
	AST_Node *right; // right child
	
	// data type of result
	int data_type;
	
	// operator
	Bitwise_op op;
	
	/* register assignment stuff */
	int g_index;
}AST_Node_Bitwise;

typedef struct AST_Node_Rel{
	Node_Type type; // node type
	
	AST_Node *left;  // left child
	AST_Node *right; // right child
	
	// data type of result
	int data_type;
	
	// operator
	Rel_op op;
	
	/* register assignment stuff */
	int g_index;
}AST_Node_Rel;

typedef struct AST_Node_Ret_Type{
	Node_Type type; // node type
	
	// return type
	int ret_type;
	
	// is pointer or not
	int pointer; // 0: not pointer, 1: pointer
}AST_Node_Ret_Type;


/* Functions */
typedef struct AST_Node_Declarations{
	Node_Type type; // node type
	
	// declarations
	struct AST_Node **declarations;
	int declaration_count;
}AST_Node_Declarations;

typedef struct AST_Node_Func_Declarations{
	Node_Type type; // node type
	
	// declarations
	AST_Node **func_declarations;
	int func_declaration_count;
}AST_Node_Func_Declarations;

typedef struct AST_Node_Func_Decl{
	Node_Type type; // node type
	
	// return type
	int ret_type;
	
	// is pointer or not
	int pointer; // 0: not pointer, 1: pointer
	
	// symbol table entry
	list_t *entry;
	
	// declarations, statements and return
	AST_Node *parameters;
	AST_Node *declarations;
	AST_Node *statements;
	AST_Node *return_node;
}AST_Node_Func_Decl;

typedef struct AST_Node_Return{
	Node_Type type; // node type
	
	// return type
	int ret_type;
	
	// return value
	AST_Node *ret_val;
}AST_Node_Return;

typedef struct AST_Node_Ref{
	Node_Type type; // node type
	
	// symbol table entry
	list_t *entry;
	
	// reference or not
	int ref; // 0: not reference, 1: reference
}AST_Node_Ref;

typedef struct AST_Node_Statements{
	Node_Type type; // node type
	
	// statements
	AST_Node **statements;
	int statement_count;
}AST_Node_Statements;


#endif