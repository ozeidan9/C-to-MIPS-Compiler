%{
    #include "../include/basic_types.hpp"
    #include "../include/semantics.hpp"
    
    #include "../include/symbol_table.hpp"
    #include "../include/ast.hpp"
    #include "../include/ast_functions.hpp"

    

    #include <string>
    #include <stdlib.h>
    #include <string.h>
    #include <fstream>
    #include <iostream>
    #include <stdio.h>



    extern AST_Node *g_root;
    extern FILE *yyin;
    extern FILE *yyout;
    extern int lineno;
    extern list_t *lookup(char *name); 
    extern void generate_code(); 

    void add_to_names(list_t *entry);
    list_t **names;
    int nc = 0;
    extern int yylex();


    void yyerror(char const *s);


    // for else ifs
    inline void add_elseif(AST_Node *elsif); 
    AST_Node **elsifs;
    int elseif_count = 0;


    // for functions
    AST_Node_Func_Decl *temp_function = new AST_Node_Func_Decl;

    AST_Node_Statements *temp1 = new AST_Node_Statements;
    AST_Node_Ref *temp2 = new AST_Node_Ref;
    AST_Node_Const *temp3 = new AST_Node_Const;
    AST_Node_Decl *temp4 = new AST_Node_Decl;
    AST_Node_Declarations *temp5 = new AST_Node_Declarations;
    AST_Node_Decl_Params *temp6 = new AST_Node_Decl_Params;
    AST_Node_Decl_Params *temp7 = new AST_Node_Decl_Params;
    AST_Node_Ret_Type *temp8 = new AST_Node_Ret_Type;
    AST_Node_Func_Declarations *temp9 = new AST_Node_Func_Declarations;
    AST_Node *temp11 = new AST_Node;
    AST_Node *incr_node = new AST_Node;
    
%}


%union{
	
	Value val;   
	list_t* symbol_table_item;
	AST_Node* node;
	int data_type;
	Param par;
}

// Tokens and type from lexer
%token <val> TYPE_INT FOR TYPE_FLOAT CONST_DOUBLE TYPE_DOUBLE
%token <val> IF ELSE RETURN WHILE 
%token <val> OP_INC OP_DEC OP_AND OP_OR OP_LTE OP_GTE OP_EQ OP_NEQ BITOP_OR BITOP_AND L_SHIFT R_SHIFT '(' ')' '!' '+' '-' '*' '/' '^' '~' 
%token <val> CONST_INT CONST_FLOAT
%token <symbol_table_item> ID




%left ','
%right '='
%right '?' ':'
%left OP_OR BITOP_OR
%left OP_AND BITOP_AND L_SHIFT R_SHIFT '~' '^'
%left OP_EQ OP_NEQ
%left '>' '<' OP_LTE OP_GTE
%left '+' '-'
%left '*' '/' '%'
%right OP_INC OP_DEC '!' '&' MINUS
%left '(' ')' '[' ']'

%type <node> ROOT
%type <node> FUNCTIONS FUNCTION FUNCTION_DECLARATION FUNC_BLOCK RETURN_TYPE
%type <node> PARAMETERS_OPTIONAL PARAMETERS 
%type <par> PARAMETER
%type <node> STATEMENT_OPTIONAL  VAR_REF STAT_ASSIGN

%type <node> STATEMENTS STATEMENT 

%type <node> DECLARATION   
%type <data_type> TYPE 
%type <symbol_table_item> VARIABLE

%type <symbol_table_item> INIT
%type <node> ASSIGNMENT
%type <node> FUNCTION_CALL VARIABLES 

%type <node> EXPRESSION
%type <node> CONSTANT  
%type <node> if_statement else_if optional_else TAIL while_statement for_statement 





%start ROOT

%%

ROOT: FUNCTIONS { g_root = $1; main_func_tree = $1; ast_traversal($1); }  
        ;


FUNCTIONS: FUNCTIONS FUNCTION { temp9 = (AST_Node_Func_Declarations*) $1;
		               $$ = new_func_declarations_node(temp9->func_declarations, temp9->func_declaration_count, $2);}
        | FUNCTION              {$$ = new_func_declarations_node(NULL, 0, $1);}
        ; 

FUNCTION: { incr_scope(); } FUNCTION_DECLARATION FUNC_BLOCK     { revisit(temp_function->entry->st_name);
                                                                hide_scope();
                                                                $$ = (AST_Node *) temp_function; }
        | { incr_scope(); } FUNCTION_DECLARATION ';'              { revisit(temp_function->entry->st_name);
                                                                hide_scope();
                                                                $$ = (AST_Node *) temp_function; }
        ;

FUNCTION_DECLARATION: { function_decl = 1; } RETURN_TYPE ID '(' {function_decl = 0; AST_Node_Ret_Type *temp8 = (AST_Node_Ret_Type *) $2;
                                        temp_function = (AST_Node_Func_Decl *) new_ast_func_decl_node(temp8->ret_type, temp8->pointer, $3);
                                        temp_function->entry->st_type = FUNCTION_TYPE;
                                        temp_function->entry->inf_type = temp8->ret_type;}

        PARAMETERS_OPTIONAL ')' 	{ if($6 != NULL){
                                                temp7 = (AST_Node_Decl_Params *) $6;
                                                temp_function->parameters = $6;
                                                temp_function->entry->parameters = temp7->parameters;
                                                temp_function->entry->num_of_pars = temp7->num_of_pars;
                                        }
                                        else{
                                                temp_function->entry->parameters = NULL;
                                                temp_function->entry->num_of_pars = 0;
                                        }	
                                        }
  
        ;

PARAMETERS_OPTIONAL: PARAMETERS   {$$ = $1;}
                |                 {$$ = NULL;}
                ; 

PARAMETERS: PARAMETERS ',' PARAMETER    { temp6 = (AST_Node_Decl_Params *) $1;
		                        $$ = new_ast_decl_params_node(temp6->parameters, temp6->num_of_pars, $3); }
        | PARAMETER 	                { $$ = new_ast_decl_params_node(NULL, 0, $1); }
        ; 

PARAMETER: { declare = 1; } TYPE VARIABLE        { declare = 0;
                                                // set type of symbol table entry	
                                                if($3->st_type == UNDEF){ /* "simple" type */
                                                        set_type($3->st_name, $2, UNDEF); 
                                                }
                                                else if($3->st_type == POINTER_TYPE){ /* pointer */
                                                        set_type($3->st_name, POINTER_TYPE, $2);
                                                }
                                                else if($3->st_type == ARRAY_TYPE){ /* array  */
                                                        set_type($3->st_name, ARRAY_TYPE, $2);
                                                }
                                                /* define parameter */
                                                $$ = def_param($2, $3->st_name, 0); }
        ;

FUNC_BLOCK: '{'  STATEMENT_OPTIONAL '}';  


STATEMENT_OPTIONAL: STATEMENTS  {temp_function->statements = $1;}  
                |               {temp_function->statements = NULL; temp_function->return_node = NULL;} 
                ; 


/* statements */

STATEMENTS: STATEMENTS STATEMENT        {temp1 = reinterpret_cast<AST_Node_Statements*> ($1);
		                        $$ = new_statements_node(temp1->statements, temp1->statement_count, $2); }
        | STATEMENT                     {$$ = new_statements_node(NULL, 0, $1);}
        ; 

STATEMENT: for_statement             {$$ = $1; temp_function->return_node = NULL;}
        | while_statement             {$$ = $1; temp_function->return_node = NULL;}
        | if_statement                 {$$ = $1; temp_function->return_node = NULL;} 
        | DECLARATION ';'               {$$ = $1; temp_function->return_node = NULL;}  
        | RETURN EXPRESSION ';'         {  temp_function->return_node = new_ast_return_node(temp_function->ret_type, $2);
                                        $$ = temp_function->return_node;}

        | RETURN FUNCTION_CALL ';'      {  temp_function->return_node = new_ast_return_node(temp_function->ret_type, $2);
                                        $$ = temp_function->return_node;}

        | ASSIGNMENT ';'                {$$ = $1; temp_function->return_node = NULL; } 
        | ID OP_INC ';'                 {$$ = new_ast_incr_node($1, 0, 0);}                                   
        | ID OP_DEC ';'                 { $$ = new_ast_incr_node($1, 1, 0);}
        | OP_INC ID ';'                 {$$ = new_ast_incr_node($2, 0, 1);}                                   
        | OP_DEC ID ';'                 { $$ = new_ast_incr_node($2, 1, 1);}
        | FUNCTION_CALL ';'             {$$ = $1; temp_function->return_node = NULL;}
        ;



for_statement: FOR '(' ASSIGNMENT ';' EXPRESSION ';' STAT_ASSIGN ')' TAIL // was ID OP_INC
{
	
	$$ = new_ast_for_node($3, $5, $7, $9);
	set_loop_counter($$);
}
;

STAT_ASSIGN: ASSIGNMENT {$$=$1;}
        | EXPRESSION {$$=$1;}
        ;


while_statement: WHILE '(' EXPRESSION ')' TAIL {$$ = new_ast_while_node($3, $5);}
                ;


if_statement:
	IF '(' EXPRESSION ')' TAIL else_if optional_else
	{
		$$ = new_ast_if_node($3, $5, elsifs, elseif_count, $7);
		elseif_count = 0;
		elsifs = NULL;
	}
	| IF '(' EXPRESSION ')' TAIL optional_else
	{
		$$ = new_ast_if_node($3, $5, NULL, 0, $6);
	}
;

else_if:
	else_if ELSE IF '(' EXPRESSION ')' TAIL
	{
                temp11 = new_ast_elsif_node($5, $7);

                if(elseif_count == 0){
                        elseif_count = 1;
                        elsifs = (AST_Node **) malloc(1 * sizeof(AST_Node));
                        elsifs[0] = temp11;
                }
                // general case
                else{
                        elseif_count++;
                        elsifs = (AST_Node **) realloc(elsifs, elseif_count * sizeof(AST_Node));
                        elsifs[elseif_count - 1] = temp11;
                }
                
        }


	| ELSE IF '(' EXPRESSION ')' TAIL
	{
                printf("IN ELSIF \n");
		temp11 = new_ast_elsif_node($4, $6);

                if(elseif_count == 0){
                        elseif_count = 1;
                        elsifs = (AST_Node **) malloc(1 * sizeof(AST_Node));
                        elsifs[0] = temp11;
                }
                // general case
                else{
                        elseif_count++;
                        elsifs = (AST_Node **) realloc(elsifs, elseif_count * sizeof(AST_Node));
                        elsifs[elseif_count - 1] = temp11;
                }

	}
;

optional_else:
	ELSE TAIL {$$ = $2;}
	| {$$ = NULL;}
;

TAIL: '{' STATEMENTS  '}' { $$ = $2; }
        | '{'  '}'      { $$ = NULL; }
        ;

        
DECLARATION: TYPE { declare = 1; } NAMES { declare = 0; } 
                                                        {
                                                                int i;
                                                                $$ = new_ast_decl_node($1, names, nc);
                                                                nc = 0;
                                                                
                                                                temp4 = reinterpret_cast<AST_Node_Decl*> ($$);
                                                                
                                                                // declare types of the names
                                                                for(i=0; i < temp4->names_count; i++){
                                                                        // variable
                                                                        if(temp4->names[i]->st_type == UNDEF){
                                                                                set_type(temp4->names[i]->st_name, temp4->data_type, UNDEF);
                                                                        }
                                                                        // pointer
                                                                        else if(temp4->names[i]->st_type == POINTER_TYPE){
                                                                                set_type(temp4->names[i]->st_name, POINTER_TYPE, temp4->data_type);
                                                                        }
                                                                        // array
                                                                        else if(temp4->names[i]->st_type == ARRAY_TYPE){
                                                                                set_type(temp4->names[i]->st_name, ARRAY_TYPE, temp4->data_type);
                                                                        }
                                                                }
                                                        }
                                                ;
        
RETURN_TYPE: TYPE { $$ = new_ast_ret_type_node($1, 0); }
        ;

TYPE: TYPE_INT { $$ = INT_TYPE;  }
        | TYPE_FLOAT {$$ = REAL_TYPE;}
        | TYPE_DOUBLE {$$ = REAL_TYPE;}
        ;

NAMES: NAMES ',' VARIABLE       {add_to_names($3);}
        | NAMES ',' INIT        {add_to_names($3);}
        | VARIABLE              {add_to_names($1);}
        | INIT                  {add_to_names($1);} 
        ;

 INIT: ID '=' CONSTANT          { temp3 = reinterpret_cast<AST_Node_Const*> ($3);
                                $1->val = temp3->val;
                                $1->st_type = temp3->const_type;
                                $$ = $1; }
        ;



ASSIGNMENT: VAR_REF '=' EXPRESSION 
        
                                {       AST_Node_Ref *temp = (AST_Node_Ref*) $1;
                                        $$ = new_ast_assign_node(temp->entry, temp->ref, $3);
                                        
                                        /* find datatypes */
                                        int type1 = get_type(temp->entry->st_name);
                                        int type2 = expression_data_type($3);
                                        
                                        /* the last function will give us information about revisits */
                                        
                                        /* contains revisit => add assignment-check to revisit queue */
                                        if(cont_revisit == 1){	
                                                /* search if entry exists */
                                                revisit_queue *q = search_queue(temp->entry->st_name);
                                                if(q == NULL){
                                                        add_to_queue(temp->entry, temp->entry->st_name, ASSIGN_CHECK);
                                                        q = search_queue(temp->entry->st_name);	
                                                }
                                                
                                                /* setup structures */
                                                if(q->num_of_assigns == 0){ /* first node */
                                                        q->nodes = (AST_Node **) malloc(sizeof(AST_Node **)); //changed to type AST_Node **
                                                }
                                                else{ /* general case */
                                                        q->nodes = (AST_Node **) realloc(q->nodes, (q->num_of_assigns + 1) * sizeof(AST_Node **));
                                                }
                                                
                                                /* add info of assignment */
                                                q->nodes[q->num_of_assigns] = (AST_Node *) $3; //changed to type AST_Node *
                                                
                                                /* increment number of assignments */
                                                q->num_of_assigns++;
                                                
                                                /* reset revisit flag */
                                                cont_revisit = 0;
                                                
                                                printf("Assignment revisit for %s at line %d\n", temp->entry->st_name, lineno);
                                        }
                                        else{ /* no revisit */
                                                /* check assignment semantics */
                                                get_result_type(
                                                        type1,       /*  variable datatype  */
                                                        type2,       /* expression datatype */
                                                        NONE  /* checking compatibility only (no operator) */
                                                );
                                        }
                                }
        ;

 
FUNCTION_CALL: ID '(' VARIABLES ')' {AST_Node_Call_Params *temp = (AST_Node_Call_Params*) $3;
                                        $$ = new_ast_func_call_node($1, temp->params, temp->num_of_pars);	
                                        
                                        /* add information to revisit queue entry (if one exists) */
                                        revisit_queue *q = search_queue($1->st_name);
                                        if(q != NULL){
                                                /* setup structures */
                                                if(q->num_of_calls == 0){ /* first call */
                                                        q->par_types = (int**) malloc(sizeof(int*));
                                                        q->num_of_pars = (int*) malloc(sizeof(int));
                                                }
                                                else{ /* general case */
                                                        q->par_types = (int**) realloc(q->par_types, (q->num_of_calls + 1) * sizeof(int*));
                                                        q->num_of_pars = (int*) realloc(q->num_of_pars, (q->num_of_calls + 1) * sizeof(int));
                                                }
                                                
                                                /* add info of function call */
                                                q->num_of_pars[q->num_of_calls] = temp->num_of_pars;
                                                q->par_types[q->num_of_calls] = (int*) malloc(temp->num_of_pars * sizeof(int));
                                                /* get the types of the parameters */
                                                int i;
                                                for(i = 0; i < temp->num_of_pars; i++){
                                                        /* get datatype of parameter-expression */
                                                        q->par_types[q->num_of_calls][i] = expression_data_type(temp->params[i]);
                                                }
                                                
                                                /* increment number of calls */
                                                q->num_of_calls++;
                                        }
                                        else{
                                                /* function declared before call */
                                                if($1->st_type == FUNCTION_TYPE){
                                                        /* check number of parameters */
                                                        if($1->num_of_pars != temp->num_of_pars){
                                                                fprintf(stderr, "Function call of %s has wrong num of parameters!\n", $1->st_name);
                                                                exit(1);
                                                        }
                                                        /* check if parameters are compatible */
                                                        int i;
                                                        for(i = 0; i < temp->num_of_pars; i++){
                                                                /* type of parameter in function declaration */
                                                                int type_1 = expression_data_type(temp->params[i]);
                                                                
                                                                /* type of parameter in function call*/
                                                                int type_2 = $1->parameters[i].par_type;
                                                                
                                                                /* check compatibility for function call */
                                                                get_result_type(type_1, type_2, NONE);
                                                                /* error occurs automatically in the function */
                                                        }
                                                }
                                        }           
                                }
        ;

VARIABLES: VARIABLES ',' EXPRESSION     {AST_Node_Call_Params *temp = (AST_Node_Call_Params*) $1;
                                        $$ = new_ast_call_params_node(temp->params, temp->num_of_pars, $3); }
        | EXPRESSION                    { $$ = new_ast_call_params_node(NULL, 0, $1); } 
        |                               {AST_Node_Call_Params *temp = (AST_Node_Call_Params*) malloc (sizeof (AST_Node_Call_Params));
                                        temp->type = CALL_PARAMS;
                                        temp->params = NULL;
                                        temp->num_of_pars = 0;
                                        $$ = (AST_Node*)temp; }
        ;
        
        
EXPRESSION: EXPRESSION '+' EXPRESSION           {$$ = new_ast_arithm_node(ADD, $1, $3);}               // need to implement SUB type in arithm node          
            | EXPRESSION '-' EXPRESSION         {$$ = new_ast_arithm_node(SUB, $1, $3);}
            | EXPRESSION '*' EXPRESSION         {$$ = new_ast_arithm_node(MUL, $1, $3);}
            | EXPRESSION '/' EXPRESSION         {$$ = new_ast_arithm_node(DIV, $1, $3);}
            | EXPRESSION '%' EXPRESSION         {$$ = new_ast_arithm_node(MOD, $1, $3);}
            | EXPRESSION BITOP_OR EXPRESSION    {$$ = new_ast_bitwise_node(BITOR, $1, $3);}
            | EXPRESSION BITOP_AND EXPRESSION   {$$ = new_ast_bitwise_node(BITAND, $1, $3);}
            | EXPRESSION '~' EXPRESSION         {$$ = new_ast_bitwise_node(BITNOT, $1, $3);}
            | EXPRESSION '^' EXPRESSION         {$$ = new_ast_bitwise_node(BITXOR, $1, $3);}
            | EXPRESSION L_SHIFT EXPRESSION     {$$ = new_ast_bitwise_node(LSHIFT, $1, $3);}
            | EXPRESSION R_SHIFT EXPRESSION     {$$ = new_ast_bitwise_node(RSHIFT, $1, $3);}
            | ID OP_INC                         {$$ = new_ast_incr_node($1, 0, 0);}                                   
            | ID OP_DEC                         {$$ = new_ast_incr_node($1, 1, 0);}
            | OP_INC ID                         {$$ = new_ast_incr_node($2, 0, 1);}                                   
            | OP_DEC ID                         {$$ = new_ast_incr_node($2, 1, 1);}
            | '!' EXPRESSION                    {$$ = new_ast_bool_node(NOT, $2, NULL);}
            | EXPRESSION OP_OR EXPRESSION       {$$ = new_ast_bool_node(OR, $1, $3);}
            | EXPRESSION OP_AND EXPRESSION      {$$ = new_ast_bool_node(AND, $1, $3);}
            | EXPRESSION OP_EQ EXPRESSION       {$$ = new_ast_equ_node(EQUAL, $1, $3);}
            | EXPRESSION OP_NEQ EXPRESSION      {$$ = new_ast_equ_node(NOT_EQUAL, $1, $3);}
            | EXPRESSION '<' EXPRESSION         {$$ = new_ast_rel_node(LESS, $1, $3);}
            | EXPRESSION '>' EXPRESSION         {$$ = new_ast_rel_node(GREATER, $1, $3);}
            | EXPRESSION OP_LTE EXPRESSION      {$$ = new_ast_rel_node(LESS_EQUAL, $1, $3);}
            | EXPRESSION OP_GTE EXPRESSION      {$$ = new_ast_rel_node(GREATER_EQUAL, $1, $3);}
            | '(' EXPRESSION ')'                {$$ = $2;}
            | '+' EXPRESSION %prec MINUS        {if($1.ival == ADD){
                                                        fprintf(stderr, "Error having plus as a sign!\n");
                                                        exit(1);
                                                }
                                                else{
                                                        AST_Node_Const *temp = (AST_Node_Const*) $2;
                                                
                                                        /* inverse value depending on the constant type */
                                                        switch(temp->const_type){
                                                                case INT_TYPE:
                                                                        temp->val.ival *= -1;
                                                                        break;
                                                                case REAL_TYPE:
                                                                        temp->val.fval *= -1;
                                                                        break;
                                                                case CHAR_TYPE:
                                                                        /* sign before char error */
                                                                        fprintf(stderr, "Error having sign before character constant!\n");
                                                                        exit(1);
                                                                        break;
                                                        }
                                                        
                                                        $$ = (AST_Node*) temp;
                                                }
	                                        }
             | '-' EXPRESSION %prec MINUS       {if($1.ival == ADD){
                                                        fprintf(stderr, "Error having plus as a sign!\n");
                                                        exit(1);
                                                }
                                                else{
                                                        AST_Node_Const *temp = (AST_Node_Const*) $2;
                                                
                                                        /* inverse value depending on the constant type */
                                                        switch(temp->const_type){
                                                                case INT_TYPE:
                                                                        temp->val.ival *= -1;
                                                                        break;
                                                                case REAL_TYPE:
                                                                        temp->val.fval *= -1;
                                                                        break;
                                                                case CHAR_TYPE:
                                                                        /* sign before char error */
                                                                        fprintf(stderr, "Error having sign before character constant!\n");
                                                                        exit(1);
                                                                        break;
                                                        }
                                                        
                                                        $$ = (AST_Node*) temp;
                                                }
                                        }
            | FUNCTION_CALL                     { $$ = $1; }
            | VAR_REF                           { $$ = $1; } 
            | CONSTANT                          { $$ = $1; }
            ;

VAR_REF: VARIABLE { $$ = new_ast_ref_node($1, 0); }                  
        ;

VARIABLE: ID    {$$ = $1;}
        ; 

CONSTANT: CONST_INT { $$ = new_ast_const_node(INT_TYPE, $1);}    
        | CONST_FLOAT {$$ = new_ast_const_node(REAL_TYPE, $1);}
        | CONST_DOUBLE {$$ = new_ast_const_node(REAL_TYPE, $1);}
        ;

%%


void add_to_names(list_t *entry){
	// first entry
	if(nc == 0){
		nc = 1;
		names = (list_t **) malloc( 1 * sizeof(list_t *));
		names[0] = entry;
	}
	// general case
	else{
		nc++;
		names = (list_t **) realloc(names, nc * sizeof(list_t *));
		names[nc - 1] = entry;		
	}
}

void add_elseif(AST_Node *elsif){
	// first 
	if(elseif_count == 0){
		elseif_count = 1;
		elsifs = (AST_Node **) malloc(1 * sizeof(AST_Node));
		elsifs[0] = elsif;
	}
	// general 
	else{
		elseif_count++;
		elsifs = (AST_Node **) realloc(elsifs, elseif_count * sizeof(AST_Node));
		elsifs[elseif_count - 1] = elsif;
	}
}


void yyerror (char const *s)
{
  fprintf (stderr, "Parse error : %s\n", s);
  exit(1);
}



AST_Node *g_root;

extern AST_Node *parseAST(FILE *file){

        init_symbol_table();

        yyin = file;
        if(yyin == NULL){
                std::cerr << "Couldn't open input file" << std::endl;
                exit(1);
        }
        g_root = NULL;
	yyparse();
        printf("Parsing finished!\n");

        // symbol table dump
	yyout = fopen("symtable_dump.out", "w");
	symtab_dump(yyout);
	fclose(yyout);

        return g_root;
}