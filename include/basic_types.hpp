#ifndef basic_types_hpp
#define basic_types_hpp

/* maximum size of tokens-identifiers */
#define MAXTOKENLEN 40

typedef union Value{
	int ival;
	double fval;
	char cval;
	char *sval;
}Value;

/* parameter struct */
typedef struct Param{
	// parameter type and name
	int par_type;
	char param_name[MAXTOKENLEN];
	
	// to store the value
	Value val;
	int passing; // value or reference
}Param;

/* a linked list of references (lineno's) for each variable */
typedef struct RefList{ 
    int lineno;
    struct RefList *next;
}RefList;

// struct that represents a list node
typedef struct list_t{
	// name, size of name, scope and occurrences (lines)
	char st_name[MAXTOKENLEN];
    int st_size;
    int scope;
    RefList *lines;
    
	// to store value
	Value val;
	
    int st_type;
	
	int g_index;
	int reg_name;
    
 
	int inf_type;
	
	Value *vals;
	int array_size;
	
	Param *parameters;
	int num_of_pars;
	
	list_t *next;
}list_t;


#endif