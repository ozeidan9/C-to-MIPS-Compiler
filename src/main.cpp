#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cstring>


#include "../include/basic_types.hpp"
#include "semantics.hpp"  
#include "symbol_table.hpp"
#include "../include/ast_functions.hpp"
#include "../include/ast.hpp"
#include "../include/code_gen.hpp"



extern FILE *yyin;

extern void generate_code(); 

extern AST_Node *parseAST(FILE *file); 

std::ostream* fp1;
std::ofstream outfile;

int main(int argc, char *argv[])
{
    

    if(argc < 4)
    {
        std::cout << "Invalid Parameters Provided" << std::endl;
        std::exit(-1);
    }


    if(std::string(argv[3]) == "-o")
    {
        outfile.open(argv[4]);
        if (!outfile.is_open())
        {
          std::cout << "Output file not found." << std::endl;
          std::exit(2);
        }
        fp1 = &outfile;
    }

    else
    {
        fp1 = &(std::cout);
    }

    FILE *file = fopen(argv[2], "r");
    

	AST_Node *program = parseAST(file);

    if(program==NULL)
    {
        std::cout << "Tree not built\n";
        std::exit(3);
    }

   
    // Generate code:
    FILE *fp;
    fp = fopen(argv[4], "w");
    
    initGraph(); // initialize graph

    generate_statements(fp, decl_map);

    main_func_traversal(fp, main_func_tree, decl_map); 

    fclose(fp);                

    printf("Generation Completed!\n"); 


	if (outfile.is_open()){
    	outfile.close();
   	}

	

   	return 0;


}  