#ifndef code_gen_hpp
#define code_gen_hpp


#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <stdio.h>


#include <unordered_map>
#include <cstring>
#include <string>



/* Adjacency List */
typedef struct AdjList{
	int index;
	AdjList *next;
}AdjList;

/* Adjacency Graph */
typedef struct AdjGraph{
	AdjList **adj;
	int vertexCount;
}AdjGraph;

/* Graph Functions */
inline AdjGraph *g;

/* Variable Array */
inline char **var_name;
inline int var_count = 0;
inline int temp_count = 0;
inline int makeNameUnq=0;
int ret = 0;
int count = 0;
int while_count = 0;
int w = 0;
int literal_count = 0;
int subi_imm;

std::string str;
char * label;
int level = 0;

std::unordered_map<char*, int> decl_map; 
int sp;



inline int getVarIndex(char *name){
	int index = -1;
	
	int i;
	for(i = 0; i < var_count; i++){
		if(strcmp(var_name[i], name) == 0){
			index = i;
			break;
		}
	}
	
	return index;
}


inline void printVarArray(std::unordered_map<char*, int> decl_map){
	int i;
	printf("VarArray:\n");
	for(i = 0 ; i < var_count; i++){
		printf("%d: %s\n", i, var_name[i]);
		decl_map.insert(std::make_pair(var_name[i], i));
	}
	printf("\n");
}



/* data declaration assembly code */
inline void generate_data_declarations(FILE *fp){

	int i, j;
	for (i = 0; i < SIZE; i++){ 
		/* if hashtable list not empty */
		if (hash_table[i] != NULL){ 
			list_t *l = hash_table[i];
			/* loop through list */
			while (l != NULL){ 
				/* Simple Variables */
				if (l->st_type == INT_TYPE){
					// fprintf(fp, "%s: .word %d\n", l->st_name, l->val.ival);
				}
				else if (l->st_type == REAL_TYPE){
					// fprintf(fp, "%s: .double %f\n", l->st_name, l->val.fval);
				}
				else if (l->st_type == CHAR_TYPE){
					// fprintf(fp, "%s: .byte \'%c\'\n", l->st_name, l->val.cval);
				}				
				
				else if (l->st_type == FUNCTION_TYPE){
					
				}
			
				l = l->next;
			}
		}
	}
}

inline char * GetRegisterName(int color, int isFloat){
    char* regName;
    regName = (char*) malloc(5 * sizeof(char));

    if(isFloat == 0){
        switch(color){
        /* callee saved values */
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            sprintf(regName, "$s%d", color);
            break;
        /* caller saved temporaries */
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            sprintf(regName, "$t%d", color - 8);
            break;
        default:
            fprintf(stderr, "Too many GP registers!\n");
            exit(1);
        }
    }
    else{
        switch(color){
        /* callee saved values */
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            sprintf(regName, "$f%d", color*2);
            break;
        default:
            fprintf(stderr, "Too many FP registers!\n");
            exit(1);
        }
    }
	
    return regName;
}



inline void generate_func_call_res(FILE *fp, AST_Node_Func_Call *node){
	if(node->entry->inf_type == REAL_TYPE){
		fprintf(fp, "		li.d		%s, 0.0\n", GetRegisterName(node->g_index, 1));
	}
	else{
		fprintf(fp, "		li  		%s, 0\n", GetRegisterName(node->g_index, 0));
	}	
}




/* Graph Functions */
inline void initGraph(){
	g = (AdjGraph*) malloc(sizeof(AdjGraph));
	g->adj = NULL;
	g->vertexCount = 0;
}

inline void insertEdge(int i, int j){
	
	AdjList *l;
	
	if(i < 0 || j < 0){
		return;
	}
	
	/* find max */
	int max;
	if(i > j){
		max = i;
	}
	else{
		max = j;
	}

	/* variable count is max + 1 */
	max = max + 1;

	/* manage space */
	if (g->vertexCount == 0){ /* first allocation */
		g->adj = (AdjList**) malloc(max * sizeof(AdjList*));
		g->vertexCount = max;
	}
	else if(g->vertexCount < max){ 
		g->adj = (AdjList**) realloc(g->adj, max * sizeof(AdjList*));
		g->vertexCount = max;
	}
	
	/* insert edge i-j */
	l = g->adj[i];
	
	while ((l != NULL) && (l->index != j)) l = l->next;
	
	if (l == NULL){
		l = (AdjList*) malloc(sizeof(AdjList));
		l->index = j;
		l->next = g->adj[i];
		g->adj[i] = l;
	}
	
	/* insert edge j-i */
	l = g->adj[j];
	
	while ((l != NULL) && (l->index != i)) l = l->next;
	
	if (l == NULL){
		l = (AdjList*) malloc(sizeof(AdjList));
		l->index = i;
		l->next = g->adj[j];
		g->adj[j] = l;
	}
}

inline void printGraph(){	
	int i;
	AdjList *l;
	
	printf("AdjGraph:\n");
	for(i = 0 ; i < g->vertexCount; i++){
		printf("%d: ", i);
		l = g->adj[i];
		while(l != NULL){
			printf("%d ", l->index);
			l = l->next;
		}
		printf("\n");
	}
	printf("\n");
}

inline int *greedyColoring(){
	
	AdjList *l;
	int i;
	
	int V = g->vertexCount;
	
	// color array
	int *colors;
	colors = (int*) malloc(V * sizeof(int));

	for(i = 0; i < V; i++){
		colors[i] = -1;
	}

	colors[0] = 0;

	int *available;
	available = (int*) malloc(V * sizeof(int));

	for(i = 0; i < V; i++){
		available[i] = 1;
	}

	int u;
	for (u = 1; u < V; u++) {
		l = g->adj[u];
		while (l != NULL) {
			i = l->index;
			if (colors[i] != -1) {
				available[colors[i]] = 0;
			}
			l = l->next;
		}

		int cr;
		for (cr = 0; cr < V; cr++) {
			if (available[cr] == 1){
				break;
			}
		}

		// assign the first avaiable color
		colors[u] = cr;

		// reset values back to true for the next iteration
		for(i = 0; i < V; i++){
			available[i] = 1;
		}
	}
	
	return colors;
}

/* Variable Array Functions */

inline void insertVar(char *name){
	/* first insertion */
	if(var_count == 0){
		var_name = (char**) malloc(1 * sizeof(char*));
		var_name[0] = (char*) malloc((strlen(name) + 1) * sizeof(char));		
		strcpy(var_name[0], name);
		
		var_count++;
	}
	else{
		/* check if variable already exists */
		int flag = 0;
		int i;
		for(i = 0; i < var_count; i++){
			if(strcmp(var_name[i], name) == 0){
				flag = 1;
				break;
			}
		}
		
		/* not inserted yet */
		if(flag == 0){
			var_name = (char**) realloc(var_name, (var_count + 1) * sizeof(char*));
			var_name[var_count] = (char*) malloc((strlen(name) + 1) * sizeof(char));		
			strcpy(var_name[var_count], name);
			
			var_count++;
		}
	}
}

inline void generate_load(FILE *fp, AST_Node_Ref *node){			
    if(node->entry->st_type == REAL_TYPE){
    	fprintf(fp, "		l.d			%s, %s\n", GetRegisterName(node->entry->g_index, 1), node->entry->st_name);			
    }
    else{
        if(node->ref == 1){
            fprintf(fp, "		la			%s, %s($0)\n", GetRegisterName(node->entry->g_index, 0), node->entry->st_name); 
        }
        else{
            fprintf(fp, "		lw			%s, %s($0)\n", GetRegisterName(node->entry->g_index, 0), node->entry->st_name);		
        }
    }
}

inline void generate_simple(FILE *fp, char* Label){
	fprintf(fp, "		j			%s\n", Label);
	fprintf(fp, "		nop", Label);						

}

inline void generate_equ(FILE *fp, AST_Node_Equ *node, int invLogic, char*  Label){
	int const_op = 0;
	int float_op = 0;
	
	/* GP: 0, FP: 1, Constant: 2, FP Constant: 3 */
	int Operand1 = 0; 
	int Operand2 = 0;
	
	AST_Node_Const *temp_const = new AST_Node_Const;;

	/* check left operand */
	if (expression_data_type(node->left) == REAL_TYPE){
		float_op = 1;
		if(node->left->type == CONST_NODE){
			const_op = 1;
			Operand1 = 3;
		}
		else{
			Operand1 = 1;
		}
	}
	else{
		if(node->left->type == CONST_NODE){
			const_op = 1;
			Operand1 = 2;
		}
		else{
			Operand1 = 0;
		}
	}

	/* check right operand */
	if(expression_data_type(node->right) == REAL_TYPE){
		float_op = 1;
		if(node->right->type == CONST_NODE){
			const_op = 1;
			Operand2 = 3;
		}
		else{
			Operand2 = 1;
		}
	}
	else{
		if(node->right->type == CONST_NODE){
			const_op = 1;
			Operand2 = 2;
		
		}
		else{
			Operand2 = 0;
		}
	}
	
	/* inverted logic */
	int op;
	if(invLogic == 1){
		/* EQUAL <-> NOT_EQUAL */
		if(node->op == EQUAL){
			op = NOT_EQUAL;
		}
		else{
			op = EQUAL;
		}
	}
	else{
		op = node->op;
	}

	fprintf(fp, "		li			$t8, 0\n");
	/* operation */
	switch(op){
		
		case EQUAL:
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$t7, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$t7, %d.0\n", temp_const->val);
					}
					
				}
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d 		%s, $t9\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				
				fprintf(fp, "		c.eq.d		");
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$t9 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7 ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$t9 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7 ");
				}
				fprintf(fp, "\n");
				
				fprintf(fp, "		bc1t		%s\n", Label);
				
			}
			else if(const_op == 1){
				if(Operand1 != 0){

					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		beq 		%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val, Label);
				}
				else if(Operand2 != 0){

					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		beq 		%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), temp_const->val, Label);
				}
			}
			else{
				if(node->left->type == ARITHM_NODE){
					fprintf(fp, "		beq			$t8, %s, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
				else if (node->right->type == ARITHM_NODE){
					fprintf(fp, "		beq			%s, $t8, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), Label);

				}
				else{
					fprintf(fp, "		beq			%s, %s, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
			}
			fprintf(fp, "		nop\n");
			fprintf(fp, "		li			$t8, 1\n");

			break;
		case NOT_EQUAL:
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$t7, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$t7, %d.0\n", temp_const->val);
					}
					
				}
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w 	$t9, $t9\n");
				}
				
				fprintf(fp, "		c.eq.d		");
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$t9 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7 ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$t9 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7 ");
				}
				fprintf(fp, "\n");
				
				fprintf(fp, "		bc1f		%s\n", Label);
				
			}
			else if(const_op == 1){
				if(Operand1 != 0){
					temp_const = (AST_Node_Const *) node->left;
					
					
					fprintf(fp, "		bne			%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val, Label); 
				}
				else if(Operand2 != 0){ 
					temp_const = (AST_Node_Const *) node->right;

					fprintf(fp, "		li			$t%d, %d\n", literal_count,  temp_const->val);
					fprintf(fp, "		bne			%s, $t%d, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), literal_count, Label);
					literal_count = literal_count + 1;
				}
			}
			else{
				if(node->left->type == ARITHM_NODE){
					fprintf(fp, "		bne			$t8, %s, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
				else if (node->right->type == ARITHM_NODE){
					fprintf(fp, "		bne			%s, $t8, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), Label);

				}
				else{
					fprintf(fp, "		bne			%s, %s, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
							}
			fprintf(fp, "		nop\n");
			fprintf(fp, "		li			$t8, 1\n");

			break;			
		default:
			fprintf(stderr, "Error in OP selection!\n");
			exit(1);
	}
}

/* various generation functions */
inline void generate_arithm(FILE *fp, AST_Node_Arithm *node){
	
	printf("In generate arithmetic function!\n");
	int float_op = 0;
	int const_op = 0;
	
	/* GP: 0, FP: 1, Constant: 2, FP Constant: 3 */
	int Result = 0;
	int Operand1 = 0; 
	int Operand2 = 0;
	
	AST_Node_Const *temp_const = new AST_Node_Const;
	AST_Node_Const *temp_const1 = new AST_Node_Const;
	AST_Node_Const *temp_const2 = new AST_Node_Const;

	

	
	/* operation */
	switch(node->op){
		case ADD:
		
			/* check left operand */
			if (expression_data_type(node->left) == REAL_TYPE){
				float_op = 1;
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 3;
				}
				else{
					Operand1 = 1;
				}
			}
			else{
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 2;
				}
				else{
					Operand1 = 0;
				}
			}
			
			/* check right operand */
			if(expression_data_type(node->right) == REAL_TYPE){
				float_op = 1;
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 3;
				}
				else{
					Operand2 = 1;
				}
			}
			else{
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 2;
				}
				else{
					Operand2 = 0;
				}
			}
			
			/* check result */
			if(node->data_type == REAL_TYPE){
				float_op = 1;
				Result = 1;
			}


			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					
					/* floating-point constant */
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$t7, %.2f\n", temp_const->val); // $t7 -> $a2
					}
					else{
						fprintf(fp, "		li.d		$t7, %d.0\n", temp_const->val);
					}
				}
				
				/* operand needs conversion */
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->left) , 0)); // $t9 -> $a2
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $t7\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$t7, $t7\n");
				}
				
				fprintf(fp, "		add.s		");
				
				/* check if result needs conversion */
				if(Result == 0){
					fprintf(fp, "$t5, ");
				}
				else{
					fprintf(fp, "%s, ", GetRegisterName(node->g_index, 1));
				}
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				fprintf(fp, "\n");
				
				if(Result == 0){
					fprintf(fp, "		cvt.w.d		$t5, $t5\n");
					fprintf(fp, "		mtc1		%s, $t5\n", GetRegisterName(node->g_index, 0));
				}
			}
			else if(const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					
					fprintf(fp, "		li			$t5, %d\n", temp_const->val);
					fprintf(fp, "		addi		$t8, $t5, %d\n", temp_const1->val);

				}
				else if(Operand1 != 0){ 
					temp_const = (AST_Node_Const *) node->left;

					fprintf(fp, "		addi		$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val);

				}
				else if(Operand2 != 0){ 
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		addi		$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->left), 0), temp_const->val);

				}
			
			}
			else{ //if left and right addition operands are both registers (check)
				temp_const = (AST_Node_Const *) node->right;
				temp_const1 = (AST_Node_Const *) node->left;

				fprintf(fp, "		add			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0));

			}
			
			break;		
		case SUB:
		
			/* check left operand */
			if (expression_data_type(node->left) == REAL_TYPE){
				float_op = 1;
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 3;
				}
				else{
					Operand1 = 1;
				}
			}
			else{
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 2;
				}
				else{
					Operand1 = 0;
				}
			}
			
			/* check right operand */
			if(expression_data_type(node->right) == REAL_TYPE){
				float_op = 1;
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 3;
				}
				else{
					Operand2 = 1;
				}
			}
			else{
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 2;
				}
				else{
					Operand2 = 0;
				}
			}
			
			/* check result */
			if(node->data_type == REAL_TYPE){
				float_op = 1;
				Result = 1;
			}
			
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					
						fprintf(fp, "		li.d		$t7, %.2f\n", temp_const->val);
					}
				}
				else{
					fprintf(fp, "		li.d		$t7, %d.0\n", temp_const->val);
				}
					
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				
				fprintf(fp, "		sub.s		");
				
				if(Result == 0){
					fprintf(fp, "$t5, ");
				}
				else{
					fprintf(fp, "%s, ", GetRegisterName(node->g_index, 1));
				}
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				fprintf(fp, "\n");
				
				if(Result == 0){
					fprintf(fp, "		cvt.w.d		$t5, $t5\n");
					fprintf(fp, "		mtc1		%s, $t5\n", GetRegisterName(node->g_index, 0));
				}
			}

			else if(const_op == 1){
				printf("const_op=1\n");
				temp_const = (AST_Node_Const *) node->left;
				temp_const1 = (AST_Node_Const *) node->right;

				if(Operand1 != 0 && Operand2 != 0){
					fprintf(fp, "		li			$t6, %d\n", temp_const->val);
					fprintf(fp, "		li			$t7, %d\n", temp_const1->val);
					fprintf(fp, "		sub			$t8, $t6, $t7\n");
					
				}
				else if(Operand1 != 0){
					fprintf(fp, "		li			$t6, %d\n", temp_const->val);
					fprintf(fp, "		sub			$t8, $t6, %s\n", GetRegisterName(getGraphIndex(node->right), 0));

				}
				else if(Operand2 != 0){  
					fprintf(fp, "		li			$t7, %d\n", temp_const1->val);
					fprintf(fp, "		sub			$t8, %s, $t7\n", GetRegisterName(getGraphIndex(node->left), 0)); //converted subi into addi but used - (not valud for --)

				}
			}
			else{
				fprintf(fp, "		sub			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0));
			
			}
			break;
		case MUL:
		
			/* check left operand */
			if (expression_data_type(node->left) == REAL_TYPE){
				float_op = 1;
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 3;
				}
				else{
					Operand1 = 1;
				}
			}
			else{
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 2;
				}
				else{
					Operand1 = 0;
				}
			}
			
			/* check right operand */
			if(expression_data_type(node->right) == REAL_TYPE){
				float_op = 1;
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 3;
				}
				else{
					Operand2 = 1;
				}
			}
			else{
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 2;
				}
				else{
					Operand2 = 0;
				}
			}
			
			/* check result */
			if(node->data_type == REAL_TYPE){
				float_op = 1;
				Result = 1;
			}
			
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$t7, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$t7, %d.0\n", temp_const->val);
					}
					
				}
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				
				fprintf(fp, "		mul.s		");
				
				if(Result == 0){
					fprintf(fp, "$t5, ");
				}
				else{
					fprintf(fp, "%s, ", GetRegisterName(node->g_index, 1));
				}
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				fprintf(fp, "\n");
				
				if(Result == 0){
					fprintf(fp, "		cvt.w.d		$t5, $t5\n");
					fprintf(fp, "		mtc1		%s, $t5\n", GetRegisterName(node->g_index, 0));
				}
			}
			else if(const_op == 1){
				if (Operand1 != 0 && Operand2 != 0){ 

					temp_const1 = (AST_Node_Const *) node->left;
					temp_const2 = (AST_Node_Const *) node->right;
					fprintf(fp, "		li			$t6, %d\n", temp_const1->val);	
					fprintf(fp, "		li			$t7, %d\n", temp_const2->val);	
					fprintf(fp, "		mul			$t8, $t6, $t7\n");

				
				}
				else if(Operand1 != 0){ 	
					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		li			$t6, %d\n", temp_const->val);	
					fprintf(fp, "		mul			$t8, %s, $t6\n", GetRegisterName(getGraphIndex(node->right), 0));

				}
				else if(Operand2 != 0){ 
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		li			$t7, %d\n", temp_const->val);	
					fprintf(fp, "		mul			$t8, %s, $t7\n", GetRegisterName(getGraphIndex(node->left), 0));
				
				}
			}
			else{
				fprintf(fp, "		mul			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0));
			
			}
			break;
		case DIV:
			printf("IN DIV CASE\n");
			/* check left operand */
			if (expression_data_type(node->left) == REAL_TYPE){
				float_op = 1;
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 3;
				}
				else{
					Operand1 = 1;
				}
			}
			else{
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 2;
				}
				else{
					Operand1 = 0;
				}
			}
			
			/* check right operand */
			if(expression_data_type(node->right) == REAL_TYPE){
				float_op = 1;
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 3;
				}
				else{
					Operand2 = 1;
				}
			}
			else{
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 2;
				}
				else{
					Operand2 = 0;
				}
			}
			
			/* check result */
			if(node->data_type == REAL_TYPE){
				float_op = 1;
				Result = 1;
			}
			
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$t7, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$t7, %d.0\n", temp_const->val);
					}
					
				}
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $t9\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$t9, $t9\n");
				}
				
				fprintf(fp, "		div.s		");
				
				if(Result == 0){
					fprintf(fp, "$t5, ");
				}
				else{
					fprintf(fp, "%s, ", GetRegisterName(node->g_index, 1));
				}
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$t9, ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$t7, ");
				}
				fprintf(fp, "\n");
				
				if(Result == 0){
					fprintf(fp, "		cvt.w.d		$t5, $t5\n");
					fprintf(fp, "		mtc1		%s, $t5\n", GetRegisterName(node->g_index, 0));
				}
			}
			else if(const_op == 1){
				if (Operand1 != 0 && Operand2 != 0){ 

					temp_const1 = (AST_Node_Const *) node->left;
					temp_const2 = (AST_Node_Const *) node->right;
					fprintf(fp, "		li			$t6, %d\n", temp_const1->val);
					fprintf(fp, "		li			$t7, %d\n", temp_const2->val);
					fprintf(fp, "		div			$t8, $t6, $t7\n");

				}
				if(Operand1 != 0){
					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		li			$t6, %d\n", temp_const->val);	
					fprintf(fp, "		div 		$t8, $t6, %s\n", GetRegisterName(getGraphIndex(node->right), 0));
				
				}
				if(Operand2 != 0){
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		li			$t7, %d\n", temp_const->val);
					fprintf(fp, "		div 		$t8, %s, $t7\n",GetRegisterName(getGraphIndex(node->left), 0));

				}
			}
			else{
				fprintf(fp, "		div			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0));
			
			}
			break;
		case MOD:
			printf("IN MOD CASE\n");
			/* check left operand */
			if (expression_data_type(node->left) == REAL_TYPE){
				float_op = 1;
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 3;
				}
				else{
					Operand1 = 1;
				}
			}
			else{
				if(node->left->type == CONST_NODE){
					const_op = 1;
					Operand1 = 2;
				}
				else{
					Operand1 = 0;
				}
			}
		
			/* check right operand */
			if(expression_data_type(node->right) == REAL_TYPE){
				float_op = 1;
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 3;
				}
				else{
					Operand2 = 1;
				}
			}
			else{
				if(node->right->type == CONST_NODE){
					const_op = 1;
					Operand2 = 2;
				}
				else{
					Operand2 = 0;
				}
			}
			
			/* check result */
			if(node->data_type == REAL_TYPE){
				float_op = 1;
				Result = 1;
			}


			if(const_op == 1){
				if (Operand1 != 0 && Operand2 != 0){ //added for case of x = 5 * 2

					temp_const1 = (AST_Node_Const *) node->left;
					temp_const2 = (AST_Node_Const *) node->right;
					fprintf(fp, "		li			$t8, %d\n", temp_const1->val);	//added to replace divi with div
					fprintf(fp, "		li			$t7, %d\n", temp_const2->val);	//added to replace divi with div

				}
				else if(Operand1 != 0){
					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		li			$t8, %d\n", temp_const->val);	//added to replace divi with div
					fprintf(fp, "		move 		$t7, %s\n", GetRegisterName(getGraphIndex(node->right), 0));
				
				}
				else if(Operand2 != 0){
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		move 		$t8, %s\n",GetRegisterName(getGraphIndex(node->left), 0));
					fprintf(fp, "		li			$t7, %d\n", temp_const->val);	//added to replace divi with div
					

				}
			}
			else{
				fprintf(fp, "		move 		$t8, %s\n",GetRegisterName(getGraphIndex(node->left), 0));
				fprintf(fp, "		move 		$t7, %s\n",GetRegisterName(getGraphIndex(node->right), 0));
				
			}


			w++;
			//label
			fprintf(fp, "\n$W%d:\n", w);

			//while t6 > 0
			fprintf(fp, "		ble			$t8, $0, $L%d\n", count);
			fprintf(fp, "		nop	\n\n");
			
			//sub t6 t6 t7
			fprintf(fp, "		sub			$t8, $t8, $t7\n");

			fprintf(fp, "		b			$W%d\n", w);
			fprintf(fp, "		nop	\n\n");
			
			fprintf(fp, "\n$L%d:\n", count);
			count++;

			//add t6 t6 t7
			fprintf(fp, "		add			$t8, $t8, $t7\n");


			break;
		case INC:
			break;
		case DEC:
			break;
		default:
			fprintf(stderr, "Error in OP selection!\n");
			exit(1);
	}	
}


inline void generate_bitwise(FILE *fp, AST_Node_Bitwise *node){
	printf("In generate_bitwise function\n");

	int const_op = 0;
	
	/* GP: 0, Constant: 1 */
	int Operand1 = 0; 
	int Operand2 = 0;
	
	AST_Node_Const *temp_const = new AST_Node_Const;
	AST_Node_Const *temp_const1 = new AST_Node_Const;

	
	
	if(node->op != BITNOT){
		if(node->left->type == CONST_NODE){
			const_op = 1;
			Operand1 = 1;
		}
		if(node->right->type == CONST_NODE){
			const_op = 1;
			Operand2 = 1;
		}
	}
	
	/* operation */
	switch(node->op){
		case BITOR:
			if (const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){  
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					printf("here5\n");
					printf("graph index in or for immediates is: %d\n", node->g_index);
					
					fprintf(fp, "		li			$t5, %d\n", temp_const->val);
					fprintf(fp, "		ori			$t8, $t5, %d\n", temp_const1->val);

				}

				else if(Operand1 == 1 && Operand2 == 0){ 
					printf("here1\n");

					temp_const = (AST_Node_Const*) node->left;
					fprintf(fp,"		ori			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val);
				}
				
				else{
					printf("here2\n");
					temp_const = (AST_Node_Const*) node->right;
					fprintf(fp,"		ori			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->left),0), temp_const->val);
				}
			}
			else{
				fprintf(fp,"		or			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left),0), GetRegisterName(getGraphIndex(node->right),0));
			}
			break;
		case BITAND:
			if (const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){  
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					printf("here50\n");
					printf("graph index in or for immediates is: %d\n", node->g_index);
					
					fprintf(fp, "		li			$t5, %d\n", temp_const->val);
					fprintf(fp, "		andi			$t8, $t5, %d\n", temp_const1->val);

				}

				else if(Operand1 == 1 && Operand2 == 0){ 
					printf("here10\n");

					temp_const = (AST_Node_Const*) node->left;
					fprintf(fp,"		andi			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val);
				}
				
				else{
					printf("here20\n");
					temp_const = (AST_Node_Const*) node->right;
					fprintf(fp,"		andi			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->left),0), temp_const->val);
				}
			}
			else{
				fprintf(fp,"		and			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left),0), GetRegisterName(getGraphIndex(node->right),0));
			}
			break;
		case BITNOT: 
			fprintf(fp, "		not 		$t8, %s", GetRegisterName(getGraphIndex(node->left),0));
			break;
		case BITXOR:  
	
			if (const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){  // x = 5 || 3
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					
					fprintf(fp, "		li			$t5, %d\n", temp_const->val);
					fprintf(fp, "		xori		$t8, $t5, %d\n", temp_const1->val);

				}

				else if(Operand1 == 1 && Operand2 == 0){ 
					printf("here10\n");

					temp_const = (AST_Node_Const*) node->left;
					fprintf(fp,"		xori		$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val);
				}
				
				else{
					printf("here20\n");
					temp_const = (AST_Node_Const*) node->right;
					fprintf(fp,"		xori		$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->left),0), temp_const->val);
				}
			}
			else{
				printf("here2020\n");
				fprintf(fp,"		xor			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left),0), GetRegisterName(getGraphIndex(node->right),0));
			}
			break;
		case LSHIFT:  
			if (const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){  // x = 5 || 3
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					printf("here500\n");
					printf("graph index in or for immediates is: %d\n", node->g_index);
					
					fprintf(fp, "		li			$t5, %d\n", temp_const->val);
					fprintf(fp, "		sll			$t8, $t5, %d\n", temp_const1->val);

				}

				else if(Operand1 == 1 && Operand2 == 0){ //going here for BOTH x=x||4 and x=5||7
					printf("here1000\n");

					temp_const = (AST_Node_Const*) node->left;
					fprintf(fp,"		sll			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val);
				}
				
				else{
					printf("here2000\n");
					temp_const = (AST_Node_Const*) node->right;
					fprintf(fp,"		sll			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->left),0), temp_const->val);
				}
			}
			else{
				fprintf(fp,"		sll			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left),0), GetRegisterName(getGraphIndex(node->right),0));
			}
			break;
		case RSHIFT: 
			if (const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){  // x = 5 || 3
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					printf("here50000\n");
					printf("graph index in or for immediates is: %d\n", node->g_index);
					
					fprintf(fp, "		li			$t5, %d\n", temp_const->val);
					fprintf(fp, "		sra			$t8, $t5, %d\n", temp_const1->val);

				}

				else if(Operand1 == 1 && Operand2 == 0){ //going here for BOTH x=x||4 and x=5||7
					printf("here10000\n");

					temp_const = (AST_Node_Const*) node->left;
					fprintf(fp,"		sra			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val);
				}
				
				else{
					printf("here200000\n");
					temp_const = (AST_Node_Const*) node->right;
					fprintf(fp,"		sra			$t8, %s, %d\n", GetRegisterName(getGraphIndex(node->left),0), temp_const->val);
				}
			}
			else{
				fprintf(fp,"		sra			$t8, %s, %s\n", GetRegisterName(getGraphIndex(node->left),0), GetRegisterName(getGraphIndex(node->right),0));
			}
			break;
		default:
			fprintf(stderr, "Error in BITWISE OP selection!\n");
			exit(1);
	}
}

inline void generate_rel(FILE *fp, AST_Node_Rel *node, int invLogic, char* Label){

	printf("generating rel instructions\n");
	
	int const_op = 0;
	int float_op = 0;
	
	int Operand1 = 0; 
	int Operand2 = 0;
	
	AST_Node_Const *temp_const = new AST_Node_Const;
	AST_Node_Const *temp_const1 = new AST_Node_Const;

	/* check left operand */
	if (expression_data_type(node->left) == REAL_TYPE){
		float_op = 1;
		if(node->left->type == CONST_NODE){
			const_op = 1;
			Operand1 = 3;
		}
		else{
			Operand1 = 1;
		}
	}
	else{
		if(node->left->type == CONST_NODE){
			const_op = 1;
			Operand1 = 2;
		}
		else{
			Operand1 = 0;
		}
	}

	/* check right operand */
	if(expression_data_type(node->right) == REAL_TYPE){
		float_op = 1;
		if(node->right->type == CONST_NODE){
			const_op = 1;
			Operand2 = 3;
		}
		else{
			Operand2 = 1;
		}
	}
	else{
		if(node->right->type == CONST_NODE){
			const_op = 1;
			Operand2 = 2;
		}
		else{
			Operand2 = 0;
		}
	}
	
	
	/* inverted logic */
	int op;
	if(invLogic == 1){
		switch(node->op){
			case GREATER:
				op = LESS_EQUAL;
				break;
			case LESS:
				op = GREATER_EQUAL;
				break;
			case GREATER_EQUAL:
				op = LESS;
				break;
			case LESS_EQUAL:
				op = GREATER;
				break;
			default:
				fprintf(stderr, "Error in RELATIVE OP selection!\n");
				exit(1);
		}
	}
	else{
		op = node->op;
	}
	printf("OP: %d\n", op);

	fprintf(fp, "		li			$t8, 0\n");

	/* operation */
	switch(op){
		case GREATER:
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$f28, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$f28, %d.0\n", temp_const->val);
					}
				}
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $f30\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$f30, $f30\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $f30\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$f30, $f30\n");
				}
				
				fprintf(fp, "		c.le.d		");
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				fprintf(fp, "\n");
				
				fprintf(fp, "		bc1f		%s\n", Label);
				
			}
			else if(const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					fprintf(fp, "		bgt			%d, %d, $%s\n", temp_const->val, temp_const1->val, Label);
				}
				else if(Operand1 != 0){
					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		ble			%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val, Label);
				}
				else if(Operand2 != 0){
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		bgt			%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), temp_const->val, Label);
				}
			}
			else{
				if(node->left->type == ARITHM_NODE){
					fprintf(fp, "		bgt			$t8, %s, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
				else if (node->right->type == ARITHM_NODE){
					fprintf(fp, "		bgt			%s, $t8, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), Label);

				}
				else{
					fprintf(fp, "		bgt			%s, %s, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0), Label);
				}

			}
			fprintf(fp, "		nop			\n");
			fprintf(fp, "		li			$t8, 1\n");

			break;
		case LESS:
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$f28, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$f28, %d.0\n", temp_const->val);
					}
					
				}
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $f30\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$f30, $f30\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $f30\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$f30, $f30\n");
				}
				
				fprintf(fp, "		c.lt.d		");
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				fprintf(fp, "\n");
				
				fprintf(fp, "		bc1t		%s\n", Label);
				
			}
			else if(const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					fprintf(fp, "		blt			%d, %d, $%s\n", temp_const->val, temp_const1->val, Label);
				}
				else if(Operand1 != 0){
					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		bge			%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val, Label);
				}
				else if(Operand2 != 0){
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		blt			%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), temp_const->val, Label);
				}
			}
			else{
				if(node->left->type == ARITHM_NODE){
					fprintf(fp, "		blt			$t8, %s, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
				else if (node->right->type == ARITHM_NODE){
					fprintf(fp, "		blt			%s, $t8, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), Label);

				}
				else{
					fprintf(fp, "		blt			%s, %s, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0), Label);
				}

			}
			fprintf(fp, "		nop			\n");
			fprintf(fp, "		li			$t8, 1\n");

			break;
		case GREATER_EQUAL:
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$f28, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$f28, %d.0\n", temp_const->val);
					}
					
				}
				
				if(Operand1 == 0){
					fprintf(fp, "MTC1.D %s, $f30\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w		$f30, $f30\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "MTC1.D %s, $f30\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$f30, $f30\n");
				}
				
				fprintf(fp, "		c.lt.d ");
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				fprintf(fp, "\n");
				
				fprintf(fp, "		bc1f		%s\n", Label);
				
			}
			else if(const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					fprintf(fp, "		bge			%d, %d, $%s\n", temp_const->val, temp_const1->val, Label);
				}
				else if(Operand1 != 0){
					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		blt			%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val, Label);
				}
				else if(Operand2 != 0){
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		bge 		%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), temp_const->val, Label);
				}
			}
			else{
				if(node->left->type == ARITHM_NODE){
					fprintf(fp, "		bge			$t8, %s, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
				else if (node->right->type == ARITHM_NODE){
					fprintf(fp, "		bge			%s, $t8, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), Label);

				}
				else{
					fprintf(fp, "		bge			%s, %s, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0), Label);
				}

			}
			fprintf(fp, "		nop			\n");
			fprintf(fp, "		li			$t8, 1\n");
			break;
		case LESS_EQUAL:
			if(float_op == 1){
				if(const_op == 1){
					if(Operand1 == 2 || Operand1 == 3){
						temp_const = (AST_Node_Const *) node->left;
					}
					else{
						temp_const = (AST_Node_Const *) node->right;
					}
					if(temp_const->const_type == REAL_TYPE){
						fprintf(fp, "		li.d		$f28, %.2f\n", temp_const->val);
					}
					else{
						fprintf(fp, "		li.d		$f28, %d.0\n", temp_const->val);
					}
					
				}
				
				if(Operand1 == 0){
					fprintf(fp, "		mtc1.d		%s, $f30\n", GetRegisterName(getGraphIndex(node->left) , 0));
					fprintf(fp, "		cvt.d.w 	$f30, $f30\n");
				}
				else if(Operand2 == 0){
					fprintf(fp, "		mtc1.d		%s, $f30\n", GetRegisterName(getGraphIndex(node->right) , 0));
					fprintf(fp, "		cvt.d.w		$f30, $f30\n");
				}
				
				fprintf(fp, "		c.le.d ");
				
				switch(Operand1){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s, ", GetRegisterName(getGraphIndex(node->left) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				
				switch(Operand2){
					case 0:
						fprintf(fp, "$f30 ");
						break;
					case 1:
						fprintf(fp, "%s ", GetRegisterName(getGraphIndex(node->right) , 1));
						break;
					case 2:
					case 3:
						fprintf(fp, "$f28 ");
				}
				fprintf(fp, "\n");
				
				fprintf(fp, "		bc1t		%s\n", Label);
				
			}
			else if(const_op == 1){
				if(Operand1 != 0 && Operand2 != 0){
					temp_const = (AST_Node_Const *) node->left;
					temp_const1 = (AST_Node_Const *) node->right;
					fprintf(fp, "		ble			%d, %d, $%s\n", temp_const->val, temp_const1->val, Label);
				}
				else if(Operand1 != 0){
					temp_const = (AST_Node_Const *) node->left;
					fprintf(fp, "		ble 		%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), temp_const->val, Label);
				}
				else if(Operand2 != 0){
					temp_const = (AST_Node_Const *) node->right;
					fprintf(fp, "		ble 		%s, %d, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), temp_const->val, Label);
				}
			}
			else{
				if(node->left->type == ARITHM_NODE){
					fprintf(fp, "		ble			$t8, %s, $%s\n", GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
				else if (node->right->type == ARITHM_NODE){
					fprintf(fp, "		ble			%s, $t8, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), Label);

				}
				else{
					fprintf(fp, "		ble			%s, %s, $%s\n", GetRegisterName(getGraphIndex(node->left), 0), GetRegisterName(getGraphIndex(node->right), 0), Label);
				}
			}
			fprintf(fp, "		nop			\n");
			fprintf(fp, "		li			$t8, 1\n");

			break;		
		default:
			fprintf(stderr, "Error in OP selection - 1929!\n");
			exit(1);
	}
}


/* main function register allocation */
inline void main_reg_allocation(AST_Node *node){
	
	AST_Node_Declarations *temp_declarations = new AST_Node_Declarations;
	AST_Node_Decl *temp_decl = new AST_Node_Decl;
	AST_Node_Decl_Params *temp_param_decl = new AST_Node_Decl_Params;
	AST_Node_Arithm *temp_arithm = new AST_Node_Arithm;
	AST_Node_Statements *temp_statements = new AST_Node_Statements;
	AST_Node_If *temp_if = new AST_Node_If;
	AST_Node_Elsif *temp_elsif = new AST_Node_Elsif;
	AST_Node_For *temp_for = new AST_Node_For;
	AST_Node_While *temp_while = new AST_Node_While;
	AST_Node_Incr *temp_incr = new AST_Node_Incr;
	AST_Node_Assign *temp_assign = new AST_Node_Assign;
	AST_Node_Func_Call *temp_func_call = new AST_Node_Func_Call;
	AST_Node_Call_Params *temp_call_params = new AST_Node_Call_Params;
	AST_Node_Func_Declarations *temp_func_declarations = new AST_Node_Func_Declarations;
	AST_Node_Func_Decl *temp_func_decl = new AST_Node_Func_Decl;

	
	/* temp variable name */
	char name[MAXTOKENLEN];
	
	int i;
	
	/* check if empty */
	if(node == NULL){
		return;
	}
	
	switch(node->type){
		/* declarations case */
		case DECLARATIONS:
			temp_declarations = (AST_Node_Declarations *) node;	
			for(i = 0; i < temp_declarations->declaration_count; i++){
				main_reg_allocation(temp_declarations->declarations[i]);
			}
			break;
		/* declaration case */
		case DECL_NODE:
			temp_decl = (AST_Node_Decl *) node;
			for(i = 0; i < temp_decl->names_count; i++){
				insertVar(temp_decl->names[i]->st_name);
				
				/* graph index */
				temp_decl->names[i]->g_index = getVarIndex(temp_decl->names[i]->st_name);
			}
			break;
		/* left and right child cases */
		case BASIC_NODE:
			main_reg_allocation(node->left);
			main_reg_allocation(node->right);
			break;
		case ARITHM_NODE:
			temp_arithm = (AST_Node_Arithm *) node;
			
			main_reg_allocation(node->left);
			main_reg_allocation(node->right);
			
			break;
		case BOOL_NODE:
			
			main_reg_allocation(node->left);
			main_reg_allocation(node->right);
			
			break;
		case BITWISE_NODE:
			main_reg_allocation(node->left);
			main_reg_allocation(node->right);
			break;
		case REL_NODE:			
			break;
		case EQU_NODE:		
			break;
		case REF_NODE:
			break;
		/* constant case */
		case CONST_NODE:
			break;
		/* statements case */
		case STATEMENTS:
			printf("In STATEMENTS case - reg alloc!\n");
			temp_statements = (AST_Node_Statements *) node;	
			printf("this is temp_statements->statement_count: %d", temp_statements->statement_count);
			
			for(i = 0; i < temp_statements->statement_count; i++){
				main_reg_allocation(temp_statements->statements[i]);
			}
			break;

		/* the if case */
		case IF_NODE:
			printf("In IF_NODE case!\n");
			temp_if = (AST_Node_If *) node;
		
			//main_reg_allocation(temp_if->condition);
		
			main_reg_allocation(temp_if->if_branch);
		
			if(temp_if->elseif_count > 0 ){
				for(i = 0; i < temp_if->elseif_count; i++){
					main_reg_allocation(temp_if->elsif_branches[i]);
				}	
			}
	
			if(temp_if->else_branch != NULL){
				main_reg_allocation(temp_if->else_branch);
			}
			break;

		/* the else if case */
		case ELSIF_NODE:
			printf("In ELSIF_NODE case!\n");
			temp_elsif = (AST_Node_Elsif *) node;
			
			
			main_reg_allocation(temp_elsif->elsif_branch);
			break;
		// /* for case */
		case FOR_NODE:
			temp_for = (AST_Node_For *) node;
	
			main_reg_allocation(temp_for->for_branch);
			
			break;
		/* while case */
		case WHILE_NODE:
			temp_while = (AST_Node_While *) node;
			
			main_reg_allocation(temp_while->while_branch);
			break;
		/* assign case */
		case ASSIGN_NODE:
			temp_assign = (AST_Node_Assign *) node;
			
			/* manage graph */
			insertEdge(temp_assign->entry->g_index, getGraphIndex(temp_assign->assign_val));
			
			main_reg_allocation(temp_assign->assign_val);
			break;
		/* simple case */
		case SIMPLE_NODE:
			break;
		/* increment statement */
		case INCR_NODE:
			temp_incr = (AST_Node_Incr*) node;
			
			break;
		/* function call case */
		case FUNC_CALL:
		printf("this is function call\n");
			temp_func_call = ( AST_Node_Func_Call *) node;
			if(temp_func_call->num_of_pars != 0){
				for(i = 0; i < temp_func_call->num_of_pars; i++){
					main_reg_allocation(temp_func_call->params[i]);
				}
			}
			
			/* insert temporary when function non-void */
			if(temp_func_call->entry->inf_type != VOID_TYPE){
				sprintf(name, "_temp%d", temp_count);
				insertVar(name);
				temp_count++;
				
				printf("Inserting %s for node:\n", name);
				ast_print_node(node);
				printf("\n");
				
				declare = 1;
				insert(name, strlen(name), temp_func_call->entry->inf_type, -1);
				declare = 0;
				
				/* graph index */
				temp_func_call->g_index = var_count - 1;
			}			
			
			;
			break;
		case CALL_PARAMS:
			temp_call_params = (AST_Node_Call_Params*) node;
			if(temp_call_params->num_of_pars > 0){
				for(i = 0; i < temp_call_params->num_of_pars; i++){
					main_reg_allocation(temp_call_params->params[i]);
				}
			}
			break;
		/* function declaration stuff */
		case FUNC_DECLS:
			printf("In FUNC_DECLS case!\n");
			temp_func_declarations = (AST_Node_Func_Declarations*) node;
			
			


			// For loop iterates over number of function declarations in propgram and points to func_decl case for each one to recurse in this function
			for(i = 0; i < temp_func_declarations->func_declaration_count; i++){  
				main_reg_allocation(temp_func_declarations->func_declarations[i]);  //Recursive call to jump to Func_Decl case below for each function declaration found
			}

			break;
		case FUNC_DECL:
			printf("In FUNC_DECL case!\n");
			temp_func_decl= (AST_Node_Func_Decl*) node;
			// Recurse through main_func_traversal for declarations, statements, return nodes
		

			for(i = 0; i < temp_func_decl->entry->num_of_pars; i++){  
				main_reg_allocation(temp_func_decl->parameters);  //Recursive call to jump to Func_Decl case below for each function declaration found
			}


			if(temp_func_decl->declarations != NULL){   //not working
				
				printf("Sending to declarations!\n");
				main_reg_allocation(temp_func_decl->declarations);

			}
			if(temp_func_decl->statements != NULL){   //not working
				
				printf("Sending to statements!\n");
				if (temp_func_decl->statements == NULL){
					printf("NULL!\n");
				}
				main_reg_allocation(temp_func_decl->statements);

			}
			if(temp_func_decl->return_node != NULL){
				main_reg_allocation(temp_func_decl->return_node);
			}

			break;

		case RET_TYPE:
		case DECL_PARAMS:
			printf("in decl_params case in reg alloc\n");
			temp_param_decl= (AST_Node_Decl_Params*) node;
			
			for(i = 0; i < temp_param_decl->num_of_pars; i++){
				insertVar(temp_param_decl->parameters[i].param_name);
				
			}

			break;
		case RETURN_NODE:
			printf("this is RETURN_NODE\n");
			break;
		default: /* wrong choice case */
			fprintf(stderr, "Error in node selection reg alloc!\n");
			exit(1);
	}
}


inline int generate_sw_sp(std::unordered_map<char*, int> decl_map, char* var_name){

	int sp;

	if (getVarIndex(var_name)==0){
		sp = 8;
		return 8;
	}

	else{
		return ((getVarIndex(var_name)*4)+8);   //need to use hashmap
	}

}


inline void generate_incr_decr(FILE *fp, AST_Node_Incr *node){
	printf("in generate_incr_decr\n");
	sp = generate_sw_sp(decl_map, node->entry->st_name);
	fprintf(fp, "		lw			%s, %d($fp)\n", GetRegisterName(node->entry->g_index, 0), sp);  

	switch(node->incr_type){
		case 0: // increment
			if(node->entry->st_type == REAL_TYPE){
				fprintf(fp, "		li.d			$28, 1.0\n");
				fprintf(fp, "		add.d			%s, %s, $28\n", GetRegisterName(node->entry->g_index, 1), GetRegisterName(node->entry->g_index, 1));
			}
			else{
				if(node->pf_type == 0){
					fprintf(fp, "		addi		$t5, %s, 1\n", GetRegisterName(node->entry->g_index, 0));
					fprintf(fp, "		sw			$t5, %d($fp)\n", sp);   
				}
				else{
					fprintf(fp, "		addi		%s, %s, 1\n", GetRegisterName(node->entry->g_index, 0), GetRegisterName(node->entry->g_index, 0));
					fprintf(fp, "		sw			%s, %d($fp)\n", GetRegisterName(node->entry->g_index, 0), sp);  
					
				}
			}
			break;
		case 1: // decrement
			fprintf(fp, "		li			$t6, 1\n");
			if(node->entry->st_type == REAL_TYPE){
				fprintf(fp, "		li.d		$28, 1.0\n");
				fprintf(fp, "		sub.d		%s, %s, $28\n", GetRegisterName(node->entry->g_index, 1), GetRegisterName(node->entry->g_index, 1));
			}
			else{
				if(node->pf_type == 0){
					fprintf(fp, "		add			$t5, %s, $t6\n", GetRegisterName(node->entry->g_index, 0));
					fprintf(fp, "		sw			$t5, %d($fp)\n", sp);   
				}
				else{
					fprintf(fp, "		add			%s, %s, 1\n", GetRegisterName(node->entry->g_index, 0), GetRegisterName(node->entry->g_index, 0));
					fprintf(fp, "		sw			%s, %d($fp)\n", GetRegisterName(node->entry->g_index, 0), sp);  
					
				}
			}
			break;
		default:
			fprintf(stderr, "Error in Incr_Type selection!\n");
			exit(1);
	}
}



inline void generate_assignment(FILE *fp, AST_Node_Assign *node, std::unordered_map<char*, int> decl_map){
	AST_Node_Const *temp_const = new AST_Node_Const;
	printf("in gen assignment\n");
	switch (node->entry->st_type){
		case INT_TYPE:
		case CHAR_TYPE:
			if(expression_data_type(node->assign_val) == INT_TYPE){
				/* constant */
				if(node->assign_val->type == CONST_NODE){
					temp_const = (AST_Node_Const *) node->assign_val;
					sp = generate_sw_sp(decl_map, node->entry->st_name);
					printf("here in const case in assignment\n");
					//printf("This is getVarIndex: %s",  node->entry->st_name);

					fprintf(fp, "		li			$t8, %d\n", temp_const->val);
					fprintf(fp, "		sw			$t8, %d($fp)\n", sp);  // Omar added for assignments

				}
				else if(node->assign_val->type == ARITHM_NODE){
					sp = generate_sw_sp(decl_map, node->entry->st_name);
					printf("here in arithm case in assignment\n");
					fprintf(fp, "		sw			$t8, %d($fp)\n", sp);
				}
				else if(node->assign_val->type == BOOL_NODE){
					sp = generate_sw_sp(decl_map, node->entry->st_name);
					printf("here in bool case in assignment");
					fprintf(fp, "		sw			$t8, %d($fp)\n", sp);
				}
				else if(node->assign_val->type == BITWISE_NODE){
					sp = generate_sw_sp(decl_map, node->entry->st_name);
					printf("here in bitwise case in assignment");
					fprintf(fp, "		sw			$t8, %d($fp)\n", sp);
				}
				else if(node->assign_val->type == INCR_NODE){
					sp = generate_sw_sp(decl_map, node->entry->st_name);
					printf("here in incr case in assignment");
					fprintf(fp, "		sw			%s, %d($fp)\n",  GetRegisterName(getGraphIndex(node->assign_val), 0), sp);
				}
				else if(node->assign_val->type == REL_NODE || node->assign_val->type == EQU_NODE){
					sp = generate_sw_sp(decl_map, node->entry->st_name);
					printf("here in relational case in assignment");
					fprintf(fp, "$L%d:\n", count);
					count++;
					fprintf(fp, "		sw			$t8, %d($fp)\n", sp);
				}
				/* variable */
				else{
					temp_const = (AST_Node_Const *) node->assign_val;
					sp = generate_sw_sp(decl_map, node->entry->st_name);
					printf("here in else case in assignment\n");
					fprintf(fp, "		sw			%s, %d($fp)\n",  GetRegisterName(getGraphIndex(node->assign_val), 0), sp);
				}
			}
			else{
				if(node->assign_val->type == CONST_NODE){
					temp_const = (AST_Node_Const *) node->assign_val;					
					fprintf(fp, "		li		%s, %d\n", GetRegisterName(node->entry->g_index, 0), temp_const->val);	
				}
				/* variable */
				else{
					fprintf(fp, "		cvt.w.d		%s, %s\n", GetRegisterName(getGraphIndex(node->assign_val), 1), GetRegisterName(getGraphIndex(node->assign_val), 1));
					fprintf(fp, "		mfc1		%s, %s\n", GetRegisterName(getGraphIndex(node->assign_val), 0), GetRegisterName(getGraphIndex(node->assign_val), 1));
					fprintf(fp, "		move		%s, %s\n", GetRegisterName(node->entry->g_index, 0), GetRegisterName(getGraphIndex(node->assign_val), 0));	
				}
			}
			break;
		case REAL_TYPE:
			if(expression_data_type(node->assign_val) == REAL_TYPE){
				/* constant */
				if(node->assign_val->type == CONST_NODE){
					temp_const = (AST_Node_Const *) node->assign_val;	
					printf("this is temp_const val: %d\n",temp_const->val);				
					fprintf(fp, "		li.d		%s, %.2f\n", GetRegisterName(node->entry->g_index, 1), temp_const->val);	
				}
				/* variable*/
				else{
					fprintf(fp, "		mov.d		%s, %s\n", GetRegisterName(node->entry->g_index, 1), GetRegisterName(getGraphIndex(node->assign_val), 1));
				}
			}
			else{
				/* constant */
				if(node->assign_val->type == CONST_NODE){
					temp_const = (AST_Node_Const *) node->assign_val;	
					printf("this is temp_const val: %d\n",temp_const->val);				
					fprintf(fp, "		li.d		%s, %d.0\n", GetRegisterName(node->entry->g_index, 1), temp_const->val);	
				}
				/* variable*/
				else{
					fprintf(fp, "		mtc1		%s, %s\n", GetRegisterName(getGraphIndex(node->assign_val), 0), GetRegisterName(getGraphIndex(node->assign_val), 1));
					fprintf(fp, "		cvt.d.w 	%s, %s\n", GetRegisterName(getGraphIndex(node->assign_val), 1), GetRegisterName(getGraphIndex(node->assign_val), 1));
					fprintf(fp, "		move		%s, %s\n", GetRegisterName(node->entry->g_index, 1), GetRegisterName(getGraphIndex(node->assign_val), 1));
				}
			}
			break;
		case POINTER_TYPE:
			if(node->assign_val->type == REF_NODE){
				AST_Node_Ref *temp_ref = (AST_Node_Ref *) node->assign_val;
				/* not a reference */
				if(temp_ref->ref == 0){
					/* integer */
					if(expression_data_type(node->assign_val) == INT_TYPE){
						fprintf(fp, "		sw			%s, 0(%s)\n", GetRegisterName(getGraphIndex(node->assign_val), 0),  GetRegisterName(node->entry->g_index, 0));
					}
					/* floating point */
					else{
						fprintf(fp, "		s.d			%s, 0(%s)\n", GetRegisterName(getGraphIndex(node->assign_val), 1), GetRegisterName(node->entry->g_index, 0));
					}
				}
				/* reference */
				else{
					fprintf(fp, "		sw			%s, %s($0)\n", GetRegisterName(getGraphIndex(node->assign_val), 0), node->entry->st_name);
					fprintf(fp, "		move		%s, %s\n", GetRegisterName(node->entry->g_index, 0), GetRegisterName(getGraphIndex(node->assign_val), 0));
				}
			}
			else{
				/* integer */
				if(expression_data_type(node->assign_val) == INT_TYPE){
					fprintf(fp, "		sw			%s, 0(%s)\n", GetRegisterName(getGraphIndex(node->assign_val), 0), GetRegisterName(node->entry->g_index, 0));
				}
				/* floating point */
				else{
					fprintf(fp, "		s.d			%s, 0(%s)\n", GetRegisterName(getGraphIndex(node->assign_val), 1), GetRegisterName(node->entry->g_index, 0));
				}
			}
			break;
		case ARRAY_TYPE:
			fprintf(fp, "Array Assignment\n");
			/* have to store the index somehow */
			break;
		default:
			fprintf(stderr, "Error in case selection - gen assignment!\n");
			exit(1);
	}
}	


/* main function tree traversal */
inline void main_func_traversal(FILE *fp, AST_Node *node, std::unordered_map<char*, int> decl_map){
	int i, j; //added for func_decl case
	int *colors = greedyColoring();  //added for func_decl case


	
	AST_Node_Declarations *temp_declarations = new AST_Node_Declarations;
	AST_Node_Decl *temp_decl = new AST_Node_Decl;
	AST_Node_Decl_Params *temp_param_decl = new AST_Node_Decl_Params;
	AST_Node_Arithm *temp_arithm = new AST_Node_Arithm;
	AST_Node_Bool *temp_bool = new AST_Node_Bool;
	AST_Node_Rel *temp_rel = new AST_Node_Rel;
	AST_Node_Equ *temp_equ = new AST_Node_Equ;
	AST_Node_Ref *temp_ref = new AST_Node_Ref;
	AST_Node_Statements *temp_statements = new AST_Node_Statements;
	AST_Node_If *temp_if = new AST_Node_If;
	AST_Node_Elsif *temp_elsif = new AST_Node_Elsif;
	AST_Node_For *temp_for = new AST_Node_For;
	AST_Node_While *temp_while = new AST_Node_While;
	AST_Node_Incr *temp_incr = new AST_Node_Incr;
	AST_Node_Assign *temp_assign = new AST_Node_Assign;
	AST_Node_Func_Call *temp_func_call = new AST_Node_Func_Call;
	AST_Node_Call_Params *temp_call_params = new AST_Node_Call_Params;
	AST_Node_Func_Declarations *temp_func_declarations = new AST_Node_Func_Declarations;
	AST_Node_Func_Decl *temp_func_decl = new AST_Node_Func_Decl;
	AST_Node_Return *temp_return = new AST_Node_Return;
	AST_Node_Const *temp_const = new AST_Node_Const;
	list_t* temp_entry;
	AST_Node_Const *temp_const1 = new AST_Node_Const;
	AST_Node_Const *temp_const2 = new AST_Node_Const;
	AST_Node_Ref *temp_ref2 = new AST_Node_Ref; 
	AST_Node_Bitwise *temp_bitwise = new AST_Node_Bitwise;

	
	/* temp variable ST entry */
	list_t *entry;
	
	printf("In main func traversal!\n");
	/* check if empty */
	if(node == NULL){
		return;
	}	
	
	switch(node->type){
		/* declarations case */
		case DECLARATIONS: // already implemnted using generate declarations function in func_declarations case
			printf("In DECARATIONS case!\n");
			temp_declarations = (AST_Node_Declarations *) node;	

			for(i = 0; i < temp_declarations->declaration_count; i++){
				main_func_traversal(fp, temp_declarations->declarations[i], decl_map);
			}
			break;
		/* declaration case */
		case DECL_NODE: // already implemnted using generate declarations function in func_declarations case
			printf("In DECL_NODE case!\n");

			temp_decl = (AST_Node_Decl *) node;
			temp_entry = *(temp_decl->names);

			for(i=0; i<temp_decl->names_count; i++){
				if (temp_entry->st_type == INT_TYPE){  // int x = 5 -> print code gen like assign

					sp = generate_sw_sp(decl_map, temp_decl->names[i]->st_name);

					fprintf(fp, "		li			%s, %d\n",  GetRegisterName(temp_decl->names[i]->g_index, 0), temp_decl->names[i]->val);				
					fprintf(fp, "		sw			%s, %d($fp)\n",  GetRegisterName(temp_decl->names[i]->g_index, 0), sp);  // Omar added for assignments
				}
				else if (temp_entry->st_type == REAL_TYPE){  // int x = 5 -> print code gen like assign

					sp = generate_sw_sp(decl_map, temp_decl->names[i]->st_name);

					fprintf(fp, "		li.d		%s, %d\n",  GetRegisterName(temp_decl->names[i]->g_index, 1), temp_decl->names[i]->val);				
					fprintf(fp, "		swc1		%s, %d($fp)\n",  GetRegisterName(temp_decl->names[i]->g_index, 1), sp);  // Omar added for assignments
				}
			}

			break;
		/* left and right child cases */
		case BASIC_NODE:
			main_func_traversal(fp, node->left, decl_map);
			main_func_traversal(fp, node->right, decl_map);
			break;
		case ARITHM_NODE:
			printf("In ARITHM_NODE case!\n");

			temp_arithm = (AST_Node_Arithm *) node;
			
			main_func_traversal(fp, node->left, decl_map);
			main_func_traversal(fp, node->right, decl_map);

			printf("generating arithmetic!\n");
			generate_arithm(fp, temp_arithm);
			
			break;
		case BOOL_NODE:
			printf("In BOOL_NODE case!\n");
			// printf("nope g_index: %d\n", node->g_index);
			temp_bool = (AST_Node_Bool *) node;
			
			main_func_traversal(fp, node->left, decl_map);

			if(temp_bool->op == OR){
				fprintf(fp, "		bne			%s, $0, $L%d\n", GetRegisterName(getGraphIndex(node->left), 0), count);
			}
			else if(temp_bool->op == AND){
				fprintf(fp, "		beq			%s, $0, $L%d\n", GetRegisterName(getGraphIndex(node->left), 0), count);
			}
			fprintf(fp, "		nop\n\n");
			
			main_func_traversal(fp, node->right, decl_map);

			if(temp_bool->op == OR){
				fprintf(fp, "		beq			%s, $0, $L%d\n", GetRegisterName(getGraphIndex(node->right), 0), count+1);
			}
			else if(temp_bool->op == AND){
				fprintf(fp, "		beq			%s, $0, $L%d\n", GetRegisterName(getGraphIndex(node->right), 0), count);
				fprintf(fp, "		nop\n\n");
				fprintf(fp, "		li			$t8, 1\n");
				fprintf(fp, "		b			$L%d\n", count+1);
				
			}
			fprintf(fp, "		nop\n\n");
			
			fprintf(fp, "$L%d:\n", count);
			count++;

			if(temp_bool->op == OR){
				fprintf(fp, "		li			$t8, 1\n");
				fprintf(fp, "		b			$L%d\n", count+1);

				fprintf(fp, "$L%d:\n", count);
				count++;
				fprintf(fp, "		li			$t8, 0\n");

				fprintf(fp, "$L%d:\n", count);
				count++;
			}
			else if(temp_bool->op == AND){
				fprintf(fp, "		li			$t8, 0\n");
				fprintf(fp, "$L%d:\n", count);
				count++;
			}			
			break;
		case BITWISE_NODE:
			printf("In BITWISE_NODE case!\n");
			temp_bitwise = (AST_Node_Bitwise *) node;

			main_func_traversal(fp, node->left, decl_map);
			main_func_traversal(fp, node->right, decl_map);

			generate_bitwise(fp, temp_bitwise);
			
			break;
		case REL_NODE:
			printf("In REL_NODE case!\n");
			temp_rel = (AST_Node_Rel *) node;
			
			main_func_traversal(fp, node->left, decl_map);
			main_func_traversal(fp, node->right, decl_map);
		
			str = "L" + std::to_string(count);
			label = &str[0];

			printf("before generate_rel\n");
			generate_rel(fp, temp_rel, 1, label);  //need to use generate unique label
			printf("after generate_rel\n");
			

			break;
		case EQU_NODE:
			printf("In EQU_NODE case!\n");
			temp_equ = (AST_Node_Equ *) node;
			
			main_func_traversal(fp, node->left, decl_map);
			main_func_traversal(fp, node->right, decl_map);

			str = "L" + std::to_string(count);
			label = &str[0];
			
			generate_equ(fp, temp_equ, 1, label);  //need to use generate unique label
			fprintf(fp, "		nop\n");

			break;
		/* reference case */
		case REF_NODE:
			temp_ref = (AST_Node_Ref *) node;
			
			/* load value from memory to register */
			printf("In REF_NODE case!\n");
			printf("This is current g_index: %d\n", temp_ref->entry->g_index); 
			sp = generate_sw_sp(decl_map, temp_ref->entry->st_name);
			printf("REF NODE TYPE: %d\n", temp_ref->type);
			
			if(expression_data_type(node) == INT_TYPE){
				fprintf(fp, "		lw			%s, %d($fp)\n",  GetRegisterName(temp_ref->entry->g_index, 0), sp);  // Omar added for assignments

			}
			else if(expression_data_type(node) == REAL_TYPE){
				printf("In REAL_TYPE condition!\n");
				fprintf(fp, "		lwc1		%s, %d($fp)\n",  GetRegisterName(temp_ref->entry->g_index, 1), sp);  // Omar added for assignments
			}
			
			break;
		/* constant case */
		case CONST_NODE:
			printf("In CONST_NODE case!\n");
			temp_const = (AST_Node_Const*) node;
	
			break;
		/* statements case */
		case STATEMENTS:
			printf("In STATEMENTS case!\n");
			temp_statements = (AST_Node_Statements *) node;	
			for(i = 0; i < temp_statements->statement_count; i++){
				main_func_traversal(fp, temp_statements->statements[i], decl_map);
			}
			break;
		/* the if case */
		case IF_NODE:
			ret++;
			count++;

			printf("In IF_NODE case!\n");


			temp_if = (AST_Node_If *) node;
			temp_const = (AST_Node_Const *) temp_if->condition;

			if(temp_if->condition->type == CONST_NODE){
				printf("If case const, const: %d\n", temp_const->val.ival);
				if(temp_const->val.ival == 0){
					fprintf(fp, "		b			$L%d\n", count);
				}
			}
			else{
				main_func_traversal(fp, temp_if->condition, decl_map);

			}

			fprintf(fp, "\n");

			main_func_traversal(fp, temp_if->if_branch, decl_map);

			
			fprintf(fp, "		b			$R%d\n", ret);
			fprintf(fp, "		nop\n");




			printf("else if count: %d\n", temp_if->elseif_count);



			if(temp_if->elseif_count > 0 ){
				printf("elseif count ios not zero!\n");
				for(i = 0; i < temp_if->elseif_count; i++){
					fprintf(fp, "\n$L%d:\n", count);
					count++;
					main_func_traversal(fp, temp_if->elsif_branches[i], decl_map);

				}
			}

			if(temp_if->else_branch != NULL){ // else branch -> need (b uniquelabel)
				
				printf("In else branch (not null)\n");
				fprintf(fp, "\n$L%d:\n", count);
				count++;
				main_func_traversal(fp, temp_if->else_branch, decl_map);
			}

			
			fprintf(fp, "\n$L%d:\n", count);
			count++;
			fprintf(fp, "\n$R%d:\n", ret);
			// level--;
			break;
		/* the else if case */
		case ELSIF_NODE:
			printf("In ELSIF_NODE case!\n");
			temp_elsif = (AST_Node_Elsif *) node;

			main_func_traversal(fp, temp_elsif->condition, decl_map);
			fprintf(fp, "\n");

			main_func_traversal(fp, temp_elsif->elsif_branch, decl_map);

			fprintf(fp, "		b			$R%d\n", ret);
			fprintf(fp, "		nop\n");

			break;
		// /* for case */
		case FOR_NODE:
			temp_for = (AST_Node_For *) node;
			
			main_func_traversal(fp, temp_for->initialize, decl_map);

			w++;
			fprintf(fp, "\n$W%d:\n", w);
			

			//label
			main_func_traversal(fp, temp_for->condition, decl_map);
			
			main_func_traversal(fp, temp_for->for_branch, decl_map);
			
			main_func_traversal(fp, temp_for->expressions, decl_map);

			//jump back to first label
			fprintf(fp, "		b			$W%d\n", w);
			fprintf(fp, "		nop\n");

			fprintf(fp, "\n$L%d:\n", count);
			count++;

			break;
		/* while case */
		case WHILE_NODE:
			printf("WHILE_NODE\n");

			temp_while = (AST_Node_While *) node;
			w++;

			//create first label
			fprintf(fp, "\n$W%d:\n", w);
			
			if(temp_while->condition->type == REF_NODE){
				printf("WHILE CONDITION IS REFERENCE\n");
				temp_ref = (AST_Node_Ref *) temp_while->condition;
				main_func_traversal(fp, temp_while->condition, decl_map);
				fprintf(fp, "		li			$t5, 0\n");
				fprintf(fp, "		ble			%s, $t5, $L%d\n", GetRegisterName(temp_ref->entry->g_index, 0), count);
				fprintf(fp, "		nop\n");
	
			}
			else if(temp_while->condition->type == CONST_NODE){
				temp_const = (AST_Node_Const *) temp_while->condition;
				if(temp_const->val.ival == 0){
					fprintf(fp, "		b			$L%d\n", count);
					fprintf(fp, "		nop\n");
				}
			}
			else{
				printf("WHILE CONDITION NOT REFERENCE\n");
				main_func_traversal(fp, temp_while->condition, decl_map);
			}
			printf("TESTING WHILE\n");
			main_func_traversal(fp, temp_while->while_branch, decl_map);
			
			//jump back to first label
			fprintf(fp, "		b			$W%d\n", w);
			fprintf(fp, "		nop\n");

			//create final label: count
			fprintf(fp, "\n$L%d:\n", count);
			count++;

			break;
		/* assign case */
		case ASSIGN_NODE:
	
			printf("In ASSIGN_NODE case!\n");

			temp_assign = (AST_Node_Assign *) node;

			main_func_traversal(fp, temp_assign->assign_val, decl_map);			
		
			generate_assignment(fp, temp_assign, decl_map);
			
			break;
		/* simple case */
		case SIMPLE_NODE:
			generate_simple(fp, "Temp_Label");
			
			break;
		/* increment statement */
		case INCR_NODE:
			temp_incr = (AST_Node_Incr*) node;

			
			
			generate_incr_decr(fp, temp_incr);
			break;
		// /* function call case */
		case FUNC_CALL:
			printf("In FUNC_CALL case!\n");
			fprintf(fp, "		sw      	$31, %d($sp)\n", 32+((g->vertexCount * 4) - 4));

			temp_func_call = (AST_Node_Func_Call *) node;
			
			if(temp_func_call->num_of_pars != 0){
				for(i = 0; i < temp_func_call->num_of_pars; i++){
					if(temp_func_call->params[i]->type == CONST_NODE){
						temp_const = (AST_Node_Const *) temp_func_call->params[i];
						fprintf(fp, "		li    		$a%d, %d\n", i, temp_const->val);
					}
					else{
						main_func_traversal(fp, temp_func_call->params[i], decl_map);
						fprintf(fp, "		move    	$a%d, $t8\n", i);
					}
				
					
					main_func_traversal(fp, temp_func_call->params[i], decl_map);

				}
			}

			fprintf(fp, "		jal    		%s\n", temp_func_call->entry->st_name);
			fprintf(fp, "		nop\n\n");
			
			/* when function non-void */
			if(temp_func_call->entry->inf_type != VOID_TYPE){
				
			}			
			break;
		case CALL_PARAMS:
			temp_call_params = (AST_Node_Call_Params*) node;	
			
			
			break;

		case FUNC_DECLS:  //Omar adding to this:
			printf("In FUNC_DECLS case!\n");
			temp_func_declarations = (AST_Node_Func_Declarations*) node;
			generate_data_declarations(fp); //geenrates data declarations for each function decl  -  


			// For loop iterates over number of function declarations in propgram and points to func_decl case for each one to recurse in this function
			for(i = 0; i < temp_func_declarations->func_declaration_count; i++){  
				main_func_traversal(fp, temp_func_declarations->func_declarations[i], decl_map);  //Recursive call to jump to Func_Decl case below for each function declaration found
			}

			break;

		case FUNC_DECL:  //Omar adding to this:
			// label++; //keeps count of current func decl
			printf("In FUNC_DECL case!\n");
			
			
			temp_func_decl = (AST_Node_Func_Decl*) node;

			fprintf(fp, "\n.globl %s\n", temp_func_decl->entry);

			// Look up symbol table for name of function and print to assembly output file
			fprintf(fp, "\n");
			fprintf(fp, "%s", temp_func_decl->entry); //change to name of fucntion from symtab
			fprintf(fp, ":\n");


			// Decrement stack frame for variable storing in functions:
  			// if there are variables that need storing - > decrement stack pointer according to number of variables needed to store
				
				
			fprintf(fp, "		addiu		$sp, $sp, -%d\n", 32+(g->vertexCount * 4));	// decrement stack frame by (4 * no_of_vbls)
			fprintf(fp, "		sw      	$fp, %d($sp)\n", 32+((g->vertexCount * 4) - 4));	
			fprintf(fp, "		move    	$fp, $sp\n");
			
			

			if(temp_func_decl->parameters != NULL){   
				
				printf("Sending to statements!\n");
				main_func_traversal(fp, temp_func_decl->parameters, decl_map);

			}

			if(temp_func_decl->statements != NULL){   
				
				printf("Sending to statements!\n");
				main_func_traversal(fp, temp_func_decl->statements, decl_map);

			}

			break;

		case RET_TYPE:
			printf("In RET_TYPE case!\n");
			break;
		case DECL_PARAMS:
			printf("In DECL_PARAMS case!\n");
			temp_param_decl = (AST_Node_Decl_Params*) node;
			
			for(i = 0; i < temp_param_decl->num_of_pars; i++){
				
				// printf("Parameter %s of type %d\n", temp_param_decl->parameters[i].param_name, temp_param_decl->parameters[i].par_type);


				if ( temp_param_decl->parameters[i].par_type == INT_TYPE){
					sp = generate_sw_sp(decl_map, temp_param_decl->parameters[i].param_name); //i fcan fix
					fprintf(fp, "		sw			$a%d, %d($fp)\n", i, sp);
				}
				
				else if(temp_param_decl->parameters[i].par_type == REAL_TYPE){
					sp = generate_sw_sp(decl_map, temp_param_decl->parameters[i].param_name); //i fcan fix
					fprintf(fp, "		swc1		$f%d, %d($fp)\n", 2*(i+6), sp);
				}
			
			
			
			}	

			break;

		case RETURN_NODE:
			printf("In RETURN_NODE case!\n");
			
			temp_return = (AST_Node_Return*) node;
			
			if (temp_return->ret_type == INT_TYPE){
			
				if ((temp_return->ret_val)->type == ARITHM_NODE){

					main_func_traversal(fp, temp_return->ret_val, decl_map);

					fprintf(fp,"		move		$2, $t8\n");
				}

				else if((temp_return->ret_val)->type == REL_NODE || (temp_return->ret_val)->type == EQU_NODE){
					main_func_traversal(fp, temp_return->ret_val, decl_map);
					
					fprintf(fp, "$L%d:\n", count);
					count++;

					fprintf(fp,"		move		$2, $t8\n");
				}
			
				else if ((temp_return->ret_val)->type == BOOL_NODE){
					main_func_traversal(fp, temp_return->ret_val, decl_map);

					fprintf(fp,"		move		$2, $t8\n");
				}

				else if ((temp_return->ret_val)->type == FUNC_CALL){
					main_func_traversal(fp, temp_return->ret_val, decl_map);

				}

				else if ((temp_return->ret_val)->type == BITWISE_NODE){
					main_func_traversal(fp, temp_return->ret_val, decl_map);

					fprintf(fp,"		move		$2, $t8\n");
				}
				else if ((temp_return->ret_val)->type == CONST_NODE){
					temp_const = (AST_Node_Const*) (temp_return->ret_val);
					fprintf(fp, "		li			$2, %d\n", temp_const->val);	
				}

				//else if ...  other nodes
				else{
					temp_ref2 = (AST_Node_Ref*) temp_return->ret_val; //added
					main_func_traversal(fp, temp_return->ret_val, decl_map);
					fprintf(fp,"		move		$2, %s\n", GetRegisterName(getGraphIndex(temp_return->ret_val), 0));

				}
			}
			else if (temp_return->ret_type == REAL_TYPE){
				if ((temp_return->ret_val)->type == ARITHM_NODE){

					main_func_traversal(fp, temp_return->ret_val, decl_map);

					// fprintf(fp,"		move		$f0, $t8\n");
				}

				else if((temp_return->ret_val)->type == REL_NODE || (temp_return->ret_val)->type == EQU_NODE){
					main_func_traversal(fp, temp_return->ret_val, decl_map);
					
					fprintf(fp, "$L%d:\n", count);
					count++;

					// fprintf(fp,"		move		$f0, $t8\n");
				}
			
				else if ((temp_return->ret_val)->type == BOOL_NODE){
					main_func_traversal(fp, temp_return->ret_val, decl_map);

					// fprintf(fp,"		move		$f0, $t8\n");
				}

				else if ((temp_return->ret_val)->type == FUNC_CALL){
					main_func_traversal(fp, temp_return->ret_val, decl_map);

				}

				else if ((temp_return->ret_val)->type == BITWISE_NODE){
					main_func_traversal(fp, temp_return->ret_val, decl_map);

					// fprintf(fp,"		move		$f0, $t8\n");
				}
				else if ((temp_return->ret_val)->type == CONST_NODE){
					temp_const = (AST_Node_Const*) (temp_return->ret_val);
					fprintf(fp, "		li.d		$f0, %d\n", temp_const->val);	
				}

				//else if ...  other nodes
				else{
					temp_ref2 = (AST_Node_Ref*) temp_return->ret_val; //added
					main_func_traversal(fp, temp_return->ret_val, decl_map);
					fprintf(fp,"		mov.d		$f0, %s\n", GetRegisterName(getGraphIndex(temp_return->ret_val), 1));

				}
			}


			if((temp_return->ret_val)->type == FUNC_CALL){
										// if there are variables that need storing - > decrement stack pointer according to number of variables needed to store
					
					fprintf(fp, "		move		$sp, $fp\n");
					fprintf(fp, "		lw    		$31, %d($sp)\n", 32+((g->vertexCount * 4) - 4));
					fprintf(fp, "		lw      	$fp, %d($sp)\n",32+((g->vertexCount * 4) - 8));  	
					fprintf(fp, "		addiu   	$sp, $sp, %d\n", 32+(g->vertexCount * 4));	// increment stack frame by (4 * no_of_vbls)
					fprintf(fp, "		j      		$31\n");
					fprintf(fp, "		nop\n");			

				
			}

			else{
				
					fprintf(fp, "		move		$sp, $fp\n");
					fprintf(fp, "		lw      	$fp, %d($sp)\n",32+((g->vertexCount * 4) - 4));  	
					fprintf(fp, "		addiu   	$sp, $sp, %d\n", 32+(g->vertexCount * 4));	// increment stack frame by (4 * no_of_vbls)
					fprintf(fp, "		j      		$31\n");
					fprintf(fp, "		nop\n");			

				
			}
			
			
			break;
		default: /* wrong choice case */
			fprintf(stderr, "Error in node selection - func traversal!\n");
			exit(1);
	}
	
}


/* statements assembly code */
inline void generate_statements(FILE *fp, std::unordered_map<char*, int> decl_map){
	int i, j;
	
	// main_reg_allocation(main_decl_tree);
	main_reg_allocation(main_func_tree);
	
	/* add edges from all the non-temporary variables */
	for(i = 0; i < var_count - temp_count; i++){
		for(j = 1; j < var_count; j++){
			if(i < j){
				insertEdge(i, j);
				insertEdge(j, i);
			}
		}
	}
	
	printVarArray(decl_map);
	
	printGraph();
	
	/* Main Function Register Assignment */
	int *colors = greedyColoring();
	
	printf("Colors:\n");
	for(i = 0; i < var_count; i++){
		printf("%s: %d\n", var_name[i], colors[i]);
		// entry->var_name[i]->g_index = colors[i];

	}
	printf("\n");
	
	printf("Registers:\n");
	for(i = 0; i < var_count; i++){
		printf("%s: %s or %s\n", var_name[i], GetRegisterName(colors[i], 0), GetRegisterName(colors[i], 1));
	}
	
	// assign register-color value as reg_name
	list_t *l;
	for(i = 0; i < var_count; i++){
		l = lookup(var_name[i]);
		l->reg_name = colors[i];
		l->g_index = i; //added for debigging g_index alloc
	}
	
	/* reset temporary counter */
	temp_count = 0;
	

}

#endif