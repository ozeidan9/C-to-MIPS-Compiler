#!/bin/bash

if which dos2unix ; then

    DOS2UNIX="dos2unix"

elif which fromdos ; then

    DOS2UNIX="fromdos"

else

    >&2 echo "warning: dos2unix is not installed."

    DOS2UNIX="tr -d \r"

fi


echo "=================================================="

echo "clean"

make clean

echo "Building c_compiler"

make bin/c_compiler > null.stderr 



if [[ "$?" -ne 0 ]]; then

    echo "Build failed.";

fi

echo""

echo "Enter filepath: "

read filepath

echo $filepath

file=$(basename -s .c $filepath)

echo ${file}

driverfile="${filepath}_driver"

echo "${driverfile}"

echo "Running bin/compiler"

bin/c_compiler -S test/$filepath.c -o out/$file.s > null.stderr

echo "Running  out/$file.o"
mips-linux-gnu-gcc -mfp32 -o out/$file.o -c out/$file.s > null.stderr

echo "Running  static"
mips-linux-gnu-gcc -mfp32 -static -o out/$file out/$file.o test/$driverfile.c  > null.stderr                 

echo "Running  qemu-mips"
qemu-mips out/$file > null.stderr

RESULT=$?

if [[ "${RESULT}" -ne 0 ]] ; then
    echo "${filepath}, Fail" > result.stdout
else
    echo "${filepath}, Pass" > result.stdout
fi

