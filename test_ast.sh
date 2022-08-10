#!/bin/bash

echo "running bison..."

bison -v -d src/parser.y -o compiler_parser.tab.cpp    > null.stderr 

echo "running flex..."

flex -o compiler_flexer.yy.cpp  src/lexer.flex       > null.stderr 

echo "Compiling the parser and lexer..."

g++ -std=c++17  -W -Wall -g -o fri_test compiler_parser.tab.cpp  compiler_flexer.yy.cpp    


echo "Running parser and lexer with input file..."

INPUT_FILE="return_test.c"

./fri_test ${INPUT_FILE} 

RESULT=$?

if [[ "${RESULT}" -ne 0 ]] ; then
    echo "${INPUT_FILE}, Fail"
else
    echo "${INPUT_FILE}, Pass"
fi

echo "Cleaning up..."

rm -f lex.yy.c parser.tab.c       