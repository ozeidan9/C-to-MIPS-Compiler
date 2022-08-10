#ifndef semantics_hpp
#define semantics_hpp
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* token types */
#define UNDEF 0
#define INT_TYPE 1
#define REAL_TYPE 2
#define CHAR_TYPE 3
#define STR_TYPE 4
#define ARRAY_TYPE 5
#define POINTER_TYPE 6
#define FUNCTION_TYPE 7
#define VOID_TYPE 8


/* operator types */
#define NONE 0		// to check types only - assignment, parameter
#define ARITHM_OP 1 // ADDOP, MULOP, DIVOP (+, -, *, /)
#define INCR_OP 2   // INCR (++, --)
#define BOOL_OP 3   // OROP, ANDOP (||, &&)
#define NOT_OP 4    // NOTOP (!)
#define REL_OP 5    // RELOP (>, <, >=, <=)
#define EQU_OP 6    // EQUOP (==, !=)
#define BIT_OP 7	// OROP, ANDOP (|, &)
#define BITNOT_OP 8 // BITNOT_OP (~)

extern int lineno;

inline void type_error(int type_1, int type_2, int op_type){ /* print type error */
	fprintf(stderr, "Type conflict between ");
	/* first type */
	if      (type_1 == INT_TYPE)           fprintf(stderr,"%s","int");
	else if (type_1 == REAL_TYPE)          fprintf(stderr,"%s","real");
	else if (type_1 == CHAR_TYPE)          fprintf(stderr,"%s","char");
	else                                   fprintf(stderr,"%s","other");
	
	fprintf(stderr, " and ");	
	
	/* second type */
	if      (type_2 == INT_TYPE)           fprintf(stderr,"%s","int");
	else if (type_2 == REAL_TYPE)          fprintf(stderr,"%s","real");
	else if (type_2 == CHAR_TYPE)          fprintf(stderr,"%s","char");
	else                                   fprintf(stderr,"%s","other");
	
	/* operator */
	fprintf(stderr," using op type ");
	switch(op_type){
		case NONE:
			fprintf(stderr,"%s","NONE");
			break;
		case ARITHM_OP:
			fprintf(stderr,"%s","ARITHM_OP");
			break;
		case INCR_OP:
			fprintf(stderr,"%s","INCR_OP");
			break;
		case BOOL_OP:
			fprintf(stderr,"%s","BOOL_OP");
			break;
		case NOT_OP:
			fprintf(stderr,"%s","NOT_OP");
			break;
		case REL_OP:
			fprintf(stderr,"%s","REL_OP");
			break;
		case EQU_OP:
			fprintf(stderr,"%s","EQU_OP");
			break;
		default: 
			fprintf(stderr, "Error in operator selection!\n");
			exit(1);	
	}
	
	/* line */
	fprintf(stderr, " in line %d\n", lineno);
	
	exit(1);

}

inline int get_result_type(int type_1, int type_2, int op_type){ /* type check and result type */
	switch(op_type){
		case NONE: /* type compatibility only, '1': compatible */
			// first type INT
			if(type_1 == INT_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return 1;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type REAL
			else if(type_1 == REAL_TYPE){
				// second type INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return 1;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type CHAR
			else if(type_1 == CHAR_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return 1;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			break;
		/* ---------------------------------------------------------- */
		case ARITHM_OP: /* arithmetic operator */
			// first type INT
			if(type_1 == INT_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				// second type REAL
				else if(type_2 == REAL_TYPE){
					return REAL_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type REAL
			else if(type_1 == REAL_TYPE){
				// second type INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return REAL_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type CHAR
			else if(type_1 == CHAR_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return CHAR_TYPE;
				}
				// second type REAL
				else if(type_2 == REAL_TYPE){
					return REAL_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case INCR_OP: /* special case of INCR */
			// type INT
			if(type_1 == INT_TYPE){
				return INT_TYPE;
			}
			// type REAL
			else if(type_1 == REAL_TYPE){
				return REAL_TYPE;
			}
			// type CHAR
			else if(type_1 == CHAR_TYPE){
				return CHAR_TYPE;
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case BOOL_OP: /* Boolean operator */
			// first type INT
			if(type_1 == INT_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type CHAR
			else if(type_1 == CHAR_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return CHAR_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case BIT_OP: /* Boolean operator */
			// first type INT
			if(type_1 == INT_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type CHAR
			else if(type_1 == CHAR_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return CHAR_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case NOT_OP: /* special case of NOTOP */
			// type INT
			if(type_1 == INT_TYPE){
				return INT_TYPE;
			}
			// type CHAR
			else if(type_1 == CHAR_TYPE){
				return INT_TYPE;
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case REL_OP: /* Relational operator */
			// first type INT
			if(type_1 == INT_TYPE){
				// second type INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else if(type_1 == REAL_TYPE){
				// second type INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type CHAR
			else if(type_1 == CHAR_TYPE){
				// second type INT, REAL or CHAR
				if(type_2 == INT_TYPE || type_2 == REAL_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		case EQU_OP: /* Equality operator */
			// first type INT
			if(type_1 == INT_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else if(type_1 == REAL_TYPE){
				// second type REAL
				if(type_2 == REAL_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			// first type CHAR
			else if(type_1 == CHAR_TYPE){
				// second type INT or CHAR
				if(type_2 == INT_TYPE || type_2 == CHAR_TYPE){
					return INT_TYPE;
				}
				else{
					type_error(type_1, type_2, op_type);
				}
			}
			else{
				type_error(type_1, type_2, op_type);
			}
			break;
		/* ---------------------------------------------------------- */
		default: /* wrong choice case */
			fprintf(stderr, "Error in operator selection!\n");
			exit(1);
	}
}



#endif