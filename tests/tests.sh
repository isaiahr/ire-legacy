#!/bin/sh


echo "Running tests"

pass=0
fail=0

# runs a test on a specific backend.
# params: llvm, filename, exitstatus
runtest(){
    if [ $1 -ne 0 ]
    then
        ../irec -bllvm "$2.ire" > /dev/null
    else
        ../irec -basm "$2.ire" > /dev/null
    fi
    # check if compilation succeeded
    if [ "$?" -ne 0 ]
    then
        echo Could not compile $2
        echo Failed
        exit
    fi
    "./$2"
    if [ "$?" -eq "$3" ]
    then
        pass=$((pass+1))
        echo -n "."
    else
        fail=$((fail+1))
        echo -n "E"
    fi
}

# runs a test with both backends
runtestboth(){
    # run with asm backend
    runtest 0 $1 $2
    # run with llvm backend
    runtest 1 $1 $2
}

# same as runtest except with output as param 4
runtestoutput(){
    if [ $1 -ne 0 ]
    then
        ../irec -bllvm "$2.ire" > /dev/null
    else
        ../irec -basm "$2.ire" > /dev/null
    fi
    # check if compilation succeeded
    if [ "$?" -ne 0 ]
    then
        echo Could not compile $2
        echo Failed
        exit
    fi
    OUTPUT=$("./$2")
    if [ "$?" -eq "$3" ] && [ "$OUTPUT" = "$4" ]
    then
        pass=$((pass+1))
        echo -n "."
    else
        fail=$((fail+1))
        echo -n "E"
    fi
}

# see runtestboth
runtestoutputboth(){
    runtestoutput 0 $1 $2 "$3"
    runtestoutput 1 $1 $2 "$3"
}

runtestboth exit 33
runtestboth arrays 10
runtestboth arith 3
runtestoutputboth hello 0 "Hello, World!"
runtestoutputboth types 5 "North America"
runtestoutputboth primes 0 "The prime numbers between 7855 and 8000 are: 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919, 7927, 7933, 7937, 7949, 7951, 7963, 7993, "
runtestboth factorial 120
runtestboth fibonacci 201
runtestboth ifelse 124
runtestboth sideeffect 1

echo ""
# end part

if [ "$fail" -eq 0 ]
then
echo All tests passed.
else
echo $fail tests failed.
fi

exit
